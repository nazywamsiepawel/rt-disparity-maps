#include <cv.h>
#include <highgui.h>
#include <stdio.h>

using namespace cv;
using namespace std;

int getPointColor(IplImage* src, CvPoint point){
 uchar* ptr=(uchar*)(src->imageData+point.y*src->widthStep);
 int b=ptr[3*point.x];
 int g=ptr[3*point.x+1];
 int r=ptr[3*point.x+2]; 
 return b+g+r; //sum is enough
}

int getMaxSum(IplImage* disparity){
  int max = -1;
  
  int width = disparity->width; 
  for(int y=0;y<disparity->height;y++){
     for(int x=0;x<width;x++){
       uchar* ptr=(uchar*)(disparity->imageData+y*disparity->widthStep);
       int b=ptr[3*x];
       int g=ptr[3*x+1];
             int r=ptr[3*x+2];
       int sum = b+g+r;
  
       if(sum>max) {max=sum;}
     }  
    }
  printf("currentmAX[%d]", max);
  return max;
}
void getLayer(IplImage* disparity, IplImage* src, int value){
  /*iterate through all the points to find the closest layer*/
    for(int y=0;y<disparity->height-1;y++){
     for(int x=0;y<disparity->width-1;x++){
       uchar* ptr=(uchar*)(disparity->imageData+y*disparity->widthStep);
       int b=ptr[3*x];
       int g=ptr[3*x+1];
             int r=ptr[3*x+2];
      
       printf("[(%d, %d, %d) at %dx%d size:%dx%d ] ", r, g, b, x, y, disparity->width, disparity->height);
     }  
    }
}

/*
 function finds the closest point, split the disparity map to layers,
 relates it to the original image and returns the regions of interest
*/
void getClosest(IplImage* disparity){
 /*find the value of the closest point in the disparity map*/
 int colorValue=-1;
 colorValue = getMaxSum(disparity);
 
 //printf("[max c : %d]", colorValue);
 /*at this point we could just use the tresholded value of the disparity map
   unfortunately, the cameras are low quality so it has to be cropped out
   and put into further processing*/

  /*copy only the points that we are interested in into a new image*/
  
  int width = 640;  
  int height = 480;
  printf("Size(%d, %d)", width, height);
  for(int y=0;y<height-1;y++){
     for(int x=0;x<width-1;x++){
      
       uchar* ptr=(uchar*)(disparity->imageData+y*disparity->widthStep);
             int b=ptr[3*x];
       int g=ptr[3*x+1];
             int r=ptr[3*x+2];
       //if(b+g+r!=0) printf("[[%d, %d]]",x, y);
       if(b+g+r == colorValue){
    /*we're not interested in those pixels for now*/
    ptr[3*x]=0;
          ptr[3*x+1]=0;
                ptr[3*x+2]=0;
       }  
   }
  }
}

/*cvtColor was infinitely annoying so I wrote a function for getting grayscale*/
void grayscale(IplImage* input, IplImage* dst){
 IplImage* r = cvCreateImage(cvGetSize(input), IPL_DEPTH_8U, 1);
 IplImage* g = cvCreateImage(cvGetSize(input), IPL_DEPTH_8U, 1);
 IplImage* b = cvCreateImage(cvGetSize(input), IPL_DEPTH_8U, 1);

 cvSplit(input, r, g, b, NULL); //split the channels
 //IplImage* temp = cvCreateImage(cvGetSize(input), IPL_DEPTH_8U, 1);
 cvAddWeighted(r, 1./3., g, 1./3., 0.0, dst);
 cvAddWeighted(dst, 2./3., b, 1./3., 0.0, dst);

}

/*creates the disparity map using BM algorithm*/
void createDisparityMap(IplImage* frame, IplImage* frame2, IplImage* dst, CvStereoBMState* BMState){

/*create the pointer for the output*/
  IplImage *dist_img;
/*measure the size of the image, assuming that both of the images are the same size we can use just one*/
  CvSize imageSize = {0,0};
  imageSize = cvGetSize(frame);
/*load up the matrices created by the calibration process*/
  CvMat *Q   = (CvMat *)cvLoad("../calibration/xml/Q.xml",NULL,NULL,NULL);
  CvMat *mx1 = (CvMat *)cvLoad("../calibration/xml/mx1.xml",NULL,NULL,NULL);
  CvMat *my1 = (CvMat *)cvLoad("../calibration/xml/my1.xml",NULL,NULL,NULL);
  CvMat *mx2 = (CvMat *)cvLoad("../calibration/xml/mx2.xml",NULL,NULL,NULL);
  CvMat *my2 = (CvMat *)cvLoad("../calibration/xml/my2.xml",NULL,NULL,NULL);

  /*allocate memory for the matrices we will be working with*/
  CvMat* img1r = cvCreateMat(imageSize.height,  imageSize.width, CV_8U );
  CvMat* img2r = cvCreateMat(imageSize.height,  imageSize.width, CV_8U );
  CvMat* disp  = cvCreateMat(imageSize.height,  imageSize.width, CV_16S );
  CvMat* vdisp = cvCreateMat(imageSize.height,  imageSize.width, CV_8U );
  CvMat* pair;

  double R1[3][3], R2[3][3], P1[3][4], P2[3][4];
  CvMat _R1 = cvMat(3, 3, CV_64F, R1);
  CvMat _R2 = cvMat(3, 3, CV_64F, R2);
  CvMat part;
  cvRemap(frame, img1r, mx1, my1 );
  cvRemap(frame, img2r, mx2, my2 );

/* 
   We use BM algorithm for finding disparities - passing in two
   images (left, right), pointed for disparity and algorithm 
   parameters class
*/
  cvFindStereoCorrespondenceBM( img1r, img2r, disp, BMState);
  cvNormalize( disp, vdisp, 0, 256, CV_MINMAX );
  
/*correspondence is found, convert to IplImage*/
  IplImage stub;
  dist_img = cvGetImage(vdisp, &stub);
  cvCopy(dist_img, dst);
}

int main(){
    int frameCount=0;
    IplImage* frame; 
    IplImage* frame2;
    IplImage* gray;
    IplImage* gray2;
    CvCapture* capture = cvCreateCameraCapture(1);
    CvCapture* capture2 = cvCreateCameraCapture(2);
    //cvNamedWindow("view1",1); 
    //cvNamedWindow("view2",1); 
    cvNamedWindow( "disparity" );
/*create CvStereoBMState - the class responsible for configuration of the BM algorithm*/
    CvStereoBMState *BMState = cvCreateStereoBMState();
    assert(BMState != 0);
    BMState->preFilterSize=41;
        BMState->preFilterCap=31;
        BMState->SADWindowSize=41;
        BMState->minDisparity=-64;
        BMState->numberOfDisparities=128;
        BMState->textureThreshold=10;
        BMState->uniquenessRatio=15;
   
    /*until the aplication is terminated, proceed with the calculation*/
    while(1){
  frameCount++;
        frame = cvQueryFrame(capture); 
  frame2 = cvQueryFrame(capture2); 
  gray  = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
  gray2 = cvCreateImage(cvGetSize(frame2), IPL_DEPTH_8U, 1);

  /*we need grayscale images for the BM algorithm*/ 
  grayscale(frame, gray);
  grayscale(frame2, gray2);
  /*get the disparity map*/
  CvPoint c; c.x = 200; c.y=200;
  IplImage* disparity = cvCreateImage(Size(640, 480), IPL_DEPTH_8U, 1);
        createDisparityMap(gray, gray2, disparity, BMState);
        IplImage* layer  = cvCloneImage(disparity);//cvCreateImage(Size(640, 480), IPL_DEPTH_8U, 1);
        
  /*get the closest layer that we are interested in, contains both hands*/
  if(frameCount>2) getClosest(layer);
  if(frameCount>200){
    /*save pics for diagrams*/
          char filename[200];
          char filename2[200];
      char filename3[200];
            int p[3];

             p[0] = CV_IMWRITE_JPEG_QUALITY;
             p[1] = 80;
             p[2] = 0;
           
            sprintf(filename,"left.jpg");
            sprintf(filename2,"right.jpg");
            sprintf(filename3,"disparity.jpg");
             
      cvSaveImage(filename, frame, p); 
      cvSaveImage(filename2, frame2, p);
      cvSaveImage(filename3, &disparity, p);
  }
  /*get the regions of interests - split the image and get both hands*/
   
        cvShowImage( "disparity", disparity);
        if(waitKey(30) >= 'q') break;
    }
    cvReleaseImage(&frame);
    cvReleaseImage(&frame2);
    cvReleaseCapture(&capture);
    return(0);
}
