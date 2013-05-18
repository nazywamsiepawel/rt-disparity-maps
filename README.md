<b>Calibration</a>

<p>Calibration is the process in which the geometrical relationships (distance shift, rotation) between two cameras located on a different points is calculated. The result of the calibration is the translation vector T and the rotation matrix R that can be later used for undistorting both single and stereo images. OpenCV implements functions useful for doing all the calculations required for stereo calibration in the function cvStereoCalibrate(). The function outputs the essential and fundamental matrices that can be saved as XML files and easily reused in other applications using the same set-up.</p>

<p>In practice, you need around 15-20 stereo images from cameras aligned at one axis. They all have to feature chessboard in various positions. It is important that the chessboard has been recognised on both of them. I wrote a small program sniper.cpp that comes in very handy when preparing those.</b>
<p>For calibration you can use the code from Learning OpenCV book. Binary attached in calibration folder.</p>

<b>Stereo vision</b>

<p>When having both cameras calibrated and the images rectified we can begin attempting to relate and match certain points in on the the images to the other. This process is called finding stereo correspondence. However, it is only possible to find the matching points between the parts of the image that overlap. To maximise the overlap both of the cameras need to be placed as close to each other as possible.
</p>

<p>
OpenCV has a number of stereo matching algorithms. The two tested in this project were implemented in the functions cvFindStereoCorrespondenceGC() and cvFindStereoCorrespondenceBM(). Running the first one results with very detailed disparity maps. Unfortunately, it is very computationally expensive and therefore not suitable for application in real-time. The second one, cvFindStereoCorrespondenceBM(), is based on Konolige's Block Matching Algorithm. It is less accurate but fast enough to be applied in computing the disparity maps in real-time.</p>
<p>
The algorithm works by using small SAD windows to find matching points between the images streamed from two views. The first step in finding stereo correspondence is pre-processing the images for easier calculations. The features that need to be optimised are image brightness (especially when the video devices are set in auto-brightness mode) and visibility of a texture. The second step involves sums of absolute difference windows, which is a trivial and fast algorithm that is widely used for measuring the similarity between image blocks. It works by taking the absolute difference between each of the windows by following the epipolar lines from both of the images which are later summed into a simple metric of block similarity.
</p>
<p>Calculating disparity maps in real time is computationally expensive task, even when using the faster (but less accurate) block-matching algorithm.</p>