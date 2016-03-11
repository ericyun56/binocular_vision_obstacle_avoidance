/**
* Implements pushbroom stereo, a fast, single-disparity stereo algorithm.
*
* Copyright 2013-2015, Andrew Barry <abarry@csail.mit.edu>
*
*/

#ifndef PUSHBROOM_STEREO_HPP
#define PUSHBROOM_STEREO_HPP

#include "opencv2/opencv.hpp"
#include <cv.h>
#include <iostream>

#include <math.h>
#include <stdio.h>
#include <random> // for debug random generator

//#include <mutex>
//#include <condition_variable>

#ifdef USE_NEON
#include <arm_neon.h>
#endif // USE_NEON

#define NUM_THREADS 8
#define MAX_IMAGE_WIDTH 4000
//#define NUM_REMAP_THREADS 8

using namespace cv;
using namespace std;



class PushbroomStereo {
public:
	void ProcessImages2(InputArray _leftImage, InputArray _rightImage, std::vector<Point3f> *pointVector3d, std::vector<Point3i> *pointVector2d, std::vector<uchar> *pointColors);
private:
	void RunStereoPushbroomStereo2(Mat leftImage,Mat rightImage,Mat laplacian_left,Mat laplacian_right,std::vector<Point3f> *pointVector3d,std::vector<Point3i> *pointVector2d,std::vector<uchar> *pointColors);

	bool CheckHorizontalInvariance(Mat leftImage, Mat rightImage, Mat sobelL, Mat sobelR, int pxX, int pxY);
	int GetSAD(Mat leftImage, Mat rightImage, Mat laplacianL, Mat laplacianR, int pxX, int pxY, int *left_interest = NULL, int *right_interest = NULL, int *raw_sad = NULL);



public:
	PushbroomStereo();
	int m_iBlockSize;		//Search block size.
	int m_iDisparity;		//Infinite distance disparity.  This is what is used to filter out false-positives.  Can be positive or negative
	int m_iSadThreshold;	// SAD (sum of absolute differences) threshold. Smaller means pixels much match better. Must be positive
	int m_iSobelLimit;		// state.sobelLimit;

	float m_fHorizontalInvarianceMultiplier;
	bool  m_bCheck_horizontal_invariance;
	bool  m_bShow_display;
	Mat	  m_matQ;
};


#endif
