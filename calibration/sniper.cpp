#include <cv.h>
#include <highgui.h>
#include <stdio.h>
using namespace cv;
using namespace std;
int main()
{
    int numBoards = 0; int numCornersHor; int numCornersVer;
    char filename[200];
    char filename2[200];
    int n=0;
    printf("Enter number of corners along width: ");
    scanf("%d", &numCornersHor);
 
    printf("Enter number of corners along height: ");
    scanf("%d", &numCornersVer);
 
     cout<<"sniper 0.1";
    int numSquares = numCornersHor * numCornersVer;
    Size board_sz = Size(numCornersHor, numCornersVer);

    VideoCapture capture = VideoCapture(1);
    int randomKey = waitKey(300);
    VideoCapture capture2 = VideoCapture(2);

    vector<Point2f> corners;
    int successes=0;

    int frameCount = 0;
    Mat image;
    Mat image2;
    Mat gray_image; Mat gray_image2;
    capture >> image;
    randomKey = waitKey(300);
    capture >> image2; 
    while(1)
    {    
        
        cvtColor(image, gray_image, CV_BGR2GRAY);
        cvtColor(image2, gray_image2, CV_BGR2GRAY);
      
        bool found = findChessboardCorners(image, board_sz, corners, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);

        if(found)
        {
           
            cornerSubPix(gray_image, corners, Size(11, 11), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1));
            drawChessboardCorners(gray_image, board_sz, corners, found);
        }
        
        /*finding the chessboard on the second camera*/
        bool found2 = findChessboardCorners(image2, board_sz, corners, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);
 
        if(found2)
        {
            
            cornerSubPix(gray_image2, corners, Size(11, 11), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1));
            drawChessboardCorners(gray_image2, board_sz, corners, found2);
        }
        /*if the chessboard has been detected on both of the images - save it*/
     
        imshow("win2", gray_image);
        imshow("win1", gray_image2);
 
        capture >> image;
        capture2 >> image2;
 
        int key = waitKey(30);

 	frameCount++;
        if(frameCount>20&&found&&found2)
        {
	    frameCount =0;	
            int p[3];
            Mat imageG; Mat image2G;
            cvtColor(image, imageG, CV_BGR2GRAY);
	    cvtColor(image2, image2G, CV_BGR2GRAY);

	    IplImage img1 = imageG;
            IplImage img2 = image2G;

            p[0] = CV_IMWRITE_JPEG_QUALITY;
            p[1] = 80;
            p[2] = 0;
           
            sprintf(filename,"images/left%.3d.jpg",n);
     
            sprintf(filename2,"images/right%.3d.jpg",n++);
          
	    cvSaveImage(filename, &img1, p); 
	    cvSaveImage(filename2, &img2, p); 
            
        }

        if(key==27)
            return 0;
 
       
    }


    while(1)
    {
        capture >> image;
       
 
        imshow("win1", image);
      
 
        waitKey(1);
    }

    capture.release();
 
    return 0;
}
