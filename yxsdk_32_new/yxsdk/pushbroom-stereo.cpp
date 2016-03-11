/**
 * Implements pushbroom stereo, a fast, single-disparity stereo algorithm.
 *
 * Copyright 2013-2015, Andrew Barry <abarry@csail.mit.edu>
 *
 */
#include "stdafx.h"
#include "pushbroom-stereo.hpp"
//#include <pthread.h>

// if USE_SAFTEY_CHECKS is 1, GetSAD will try to make sure
// that it will do the right thing even if you ask it for pixel
// values near the edges of images.  Set to 0 for a small speedup.
#define USE_SAFTEY_CHECKS 0

#define INVARIANCE_CHECK_VERT_OFFSET_MIN (-8)
#define INVARIANCE_CHECK_VERT_OFFSET_MAX 8
#define INVARIANCE_CHECK_VERT_OFFSET_INCREMENT 2

#define INVARIANCE_CHECK_HORZ_OFFSET_MIN (-3)
#define INVARIANCE_CHECK_HORZ_OFFSET_MAX 3

#define NUMERIC_CONST 333 // just a constant that we multiply the score by to make
                          // all the parameters in a nice integer range

//PushbroomStereoThreadStarter thread_starter[NUM_THREADS+1];


PushbroomStereo::PushbroomStereo() 
{
	m_iBlockSize	= 5;		//原为 5	
	m_iDisparity	= 160;		//原为 -105
	m_iSadThreshold	= 90;		//原为 54	
	m_iSobelLimit	= 0;	
	m_fHorizontalInvarianceMultiplier = 0.5;
	m_bCheck_horizontal_invariance = false;
	m_bShow_display = true;
}

void PushbroomStereo::ProcessImages2(InputArray _leftImage, InputArray _rightImage, std::vector<Point3f> *pointVector3d, std::vector<Point3i> *pointVector2d, std::vector<uchar> *pointColors) 
{

    Mat leftImage = _leftImage.getMat();
    Mat rightImage = _rightImage.getMat();

    // make sure that the inputs are of the right type
    CV_Assert(leftImage.type() == CV_8UC1 && rightImage.type() == CV_8UC1);

    //Mat remapped_left(state.mapxL.rows, state.mapxL.cols, leftImage.depth());
    //Mat remapped_right(state.mapxR.rows, state.mapxR.cols, rightImage.depth());


	//INTEREST_OP步骤，拉普拉斯变换
    Mat laplacian_left(leftImage.rows, leftImage.cols, leftImage.depth());
    Mat laplacian_right(rightImage.rows, rightImage.cols, rightImage.depth());

	Laplacian(leftImage,laplacian_left, -1, 3, 1, 0, BORDER_DEFAULT);
	Laplacian(rightImage,laplacian_right, -1, 3, 1, 0, BORDER_DEFAULT);

	//imshow("Laplac_L", laplacian_left);
	//imshow("Laplac_R", laplacian_right);

	//while( 1 )
	//{
	//	waitKey( 100 );
	//}

	RunStereoPushbroomStereo2(leftImage, rightImage,laplacian_left,laplacian_right,	pointVector3d,pointVector2d,pointColors);


}

void PushbroomStereo::RunStereoPushbroomStereo2(Mat leftImage, Mat rightImage,Mat laplacian_left,Mat laplacian_right,	std::vector<Point3f> *pointVector3d,std::vector<Point3i> *pointVector2d,std::vector<uchar> *pointColors)
{

    int row_start						= 0;//statet->row_start;
	int row_end							= leftImage.rows;//statet->row_end;

    //PushbroomStereoState state			= statet->state;

    // we will do this by looping through every block in the left image
    // (defined by blockSize) and checking for a matching value on
    // the right image

    std::vector<Point3f> localHitPoints;

	//待确认
    int startJ = 0;
    int stopJ = leftImage.cols - (m_iDisparity + m_iBlockSize);
    if (m_iDisparity < 0)
    {
        startJ = -m_iDisparity;
        stopJ = leftImage.cols - m_iBlockSize;
    }

    //printf("row_start: %d, row_end: %d, startJ: %d, stopJ: %d, rows: %d, cols: %d\n", row_start, row_end, startJ, stopJ, leftImage.rows, leftImage.cols);
    int hitCounter = 0;
    //if (state.random_results < 0) 
	//{
        for (int i=row_start; i < row_end;i+=m_iBlockSize)
        {
            for (int j=startJ; j < stopJ; j+=m_iBlockSize)
            {
                // get the sum of absolute differences for this location  on both images
                int sad = GetSAD(leftImage, rightImage, laplacian_left, laplacian_right, j, i);
                // check to see if the SAD is below the threshold,
                // indicating a hit
				
                if (sad < m_iSadThreshold && sad >= 0)
                {
                    // got a hit
                    // now check for horizontal invariance (ie check for parts of the image that look the same as this which would indicate that this might be a false-positive)
                    if (!m_bCheck_horizontal_invariance || (CheckHorizontalInvariance(leftImage, rightImage, laplacian_left, laplacian_right, j, i)== false)) 
					{

                        // add it to the vector of matches
                        // don't forget to offset it by the blockSize,so we match the center of the block instead of the top left corner
                        localHitPoints.push_back(Point3f(j+m_iBlockSize/2.0, i+m_iBlockSize/2.0, -m_iDisparity));
                        //localHitPoints.push_back(Point3f(state.debugJ, state.debugI, -disparity));


                        uchar pxL = leftImage.at<uchar>(i,j);
                        pointColors->push_back(pxL); // this is the corner of the box, not the center

                        hitCounter ++;

                        if (m_bShow_display)
                            pointVector2d->push_back(Point3i(j, i, sad));
                     } // check horizontal invariance
                }
            }
        }


    // now we have an array of hits -- transform them to 3d points
    if (hitCounter > 0) 
		perspectiveTransform(localHitPoints, *pointVector3d, m_matQ);

}

/**
 * Get the sum of absolute differences for a specific pixel location and disparity
 *
 * @param leftImage left image
 * @param rightImage right image
 * @param laplacianL laplacian-fitlered left image
 * @param laplacianR laplacian-filtered right image
 * @param pxX row pixel location
 * @param pxY column pixel location
 * @param state state structure that includes a number of parameters
 * @param left_interest optional parameter that will be filled with the value for the left interest operation
 * @param right_interest same as above, for the right image
 *
 * @retval scaled sum of absolute differences for this block --
 *      the value is the sum/numberOfPixels
 */
int PushbroomStereo::GetSAD(Mat leftImage, Mat rightImage, Mat laplacianL, Mat laplacianR, int pxX, int pxY, int *left_interest, int *right_interest, int *raw_sad)
{
    // top left corner of the SAD box
    int startX = pxX;
    int startY = pxY;

    // bottom right corner of the SAD box
    #ifndef USE_NEON
        int endX = pxX + m_iBlockSize - 1;
    #endif

    int endY = pxY + m_iBlockSize - 1;

    #if USE_SAFTEY_CHECKS
        int flag = false;
        if (startX < 0)
        {
            printf("Warning: startX < 0\n");
            flag = true;
        }

        if (endX > rightImage.cols)
        {
            printf("Warning: endX > leftImage.cols\n");
            flag = true;
        }

        if (startX + disparity < 0)
        {
            printf("Warning: startX + disparity < 0\n");
            flag = true;
        }

        if (endX + disparity > rightImage.cols)
        {
            printf("Warning: endX + disparity > leftImage.cols\n");
            flag = true;
        }

        if (endX + disparity > rightImage.cols)
        {
            printf("Warning: endX + disparity > rightImage.cols\n");
            endX = rightImage.cols - disparity;
            flag = true;
        }

        if (startY < 0) {
            printf("Warning: startY < 0\n");
            flag = true;
        }

        if (endY > rightImage.rows) {
            printf("Warning: endY > rightImage.rows\n");
            flag = true;
        }

        // disparity might be negative as well
        if (disparity < 0 && startX + disparity < 0)
        {
            printf("Warning: disparity < 0 && startX + disparity < 0\n");
            startX = -disparity;
            flag = true;
        }

        if (flag == true)
        {
            printf("startX = %d, endX = %d, disparity = %d, startY = %d, endY = %d\n", startX, endX, disparity, startY, endY);
        }



        startX = max(0, startX);
        startY = max(0, startY);

        endX = min(leftImage.cols - disparity, endX);
        endY = min(leftImage.rows, endY);
    #endif

    int leftVal = 0, rightVal = 0;

    int sad = 0;

    #ifdef USE_NEON
        uint16x8_t interest_op_sum_8x_L, interest_op_sum_8x_R, sad_sum_8x;

        // load zeros into everything
        interest_op_sum_8x_L = vdupq_n_u16(0);
        interest_op_sum_8x_R = vdupq_n_u16(0);
        sad_sum_8x = vdupq_n_u16(0);

    #endif
    for (int i=startY;i<=endY;i++) 
	{
		if(i>=leftImage.rows-1)
			continue;
        //Get a pointer for this row
        uchar *this_rowL = leftImage.ptr<uchar>(i);
        uchar *this_rowR = rightImage.ptr<uchar>(i);

        uchar *this_row_laplacianL = laplacianL.ptr<uchar>(i);
        uchar *this_row_laplacianR = laplacianR.ptr<uchar>(i);

        #ifdef USE_NEON
            // load this row into memory
            uint8x8_t this_row_8x8_L = vld1_u8(this_rowL + startX);
            uint8x8_t this_row_8x8_R = vld1_u8(this_rowR + startX + disparity);

            uint8x8_t interest_op_8x8_L = vld1_u8(this_row_laplacianL + startX);
            uint8x8_t interest_op_8x8_R = vld1_u8(this_row_laplacianR + startX + disparity);

            // do absolute differencing for the entire row in one operation!
            uint8x8_t sad_8x = vabd_u8(this_row_8x8_L, this_row_8x8_R);

            // sum up
            sad_sum_8x = vaddw_u8(sad_sum_8x, sad_8x);

            // sum laplacian values
            interest_op_sum_8x_L = vaddw_u8(interest_op_sum_8x_L, interest_op_8x8_L);
            interest_op_sum_8x_R = vaddw_u8(interest_op_sum_8x_R, interest_op_8x8_R);

        #else // USE_NEON
            for (int j=startX;j<=endX;j++) {
                // we are now looking at a single pixel value
                /*uchar pxL = leftImage.at<uchar>(i,j);
                uchar pxR = rightImage.at<uchar>(i,j + disparity);

                uchar sL = laplacianL.at<uchar>(i,j);
                uchar sR = laplacianR.at<uchar>(i,j + disparity);
                */


                uchar sL = this_row_laplacianL[j];//laplacianL.at<uchar>(i,j);
                uchar sR = this_row_laplacianR[j + m_iDisparity]; //laplacianR.at<uchar>(i,j + disparity);

                leftVal += sL;
                rightVal += sR;

                uchar pxL = this_rowL[j];
                uchar pxR = this_rowR[j + m_iDisparity];

                sad += abs(pxL - pxR);
            }
        #endif // USE_NEON
    }

    #ifdef USE_NEON
        // sum up
        sad = vgetq_lane_u16(sad_sum_8x, 0) + vgetq_lane_u16(sad_sum_8x, 1)
           + vgetq_lane_u16(sad_sum_8x, 2) + vgetq_lane_u16(sad_sum_8x, 3)
           + vgetq_lane_u16(sad_sum_8x, 4);// + vgetq_lane_u16(sad_sum_8x, 5)
    //           + vgetq_lane_u16(sad_sum_8x, 6) + vgetq_lane_u16(sad_sum_8x, 7);

        leftVal = vgetq_lane_u16(interest_op_sum_8x_L, 0)
                + vgetq_lane_u16(interest_op_sum_8x_L, 1)
                + vgetq_lane_u16(interest_op_sum_8x_L, 2)
                + vgetq_lane_u16(interest_op_sum_8x_L, 3)
                + vgetq_lane_u16(interest_op_sum_8x_L, 4);


        rightVal = vgetq_lane_u16(interest_op_sum_8x_R, 0)
                 + vgetq_lane_u16(interest_op_sum_8x_R, 1)
                 + vgetq_lane_u16(interest_op_sum_8x_R, 2)
                 + vgetq_lane_u16(interest_op_sum_8x_R, 3)
                 + vgetq_lane_u16(interest_op_sum_8x_R, 4);
    #endif

    //cout << "(" << leftVal << ", " << rightVal << ") vs. (" << leftVal2 << ", " << rightVal2 << ")" << endl;

    int laplacian_value = leftVal + rightVal;

	int fThresh = 200;
	if((leftVal<fThresh)||(rightVal<fThresh))
		laplacian_value /= 10;
    //cout << "sad with neon: " << sad << " without neon: " << sad2 << endl;
    if (left_interest != NULL)         *left_interest = leftVal; 
    if (right_interest != NULL)        *right_interest = rightVal;

    // percentage of total interest value that is different
    //float diff_score = 100*(float)abs(leftVal - rightVal)/(float)laplacian_value;

    if (raw_sad != NULL)        *raw_sad = sad;
    if (leftVal < m_iSobelLimit || rightVal < m_iSobelLimit)// || diff_score > state.interest_diff_limit)
        return -1;

    // weight laplacian_value into the score

    //return sobel;
    return NUMERIC_CONST*(float)sad/(float)laplacian_value;
}

/**
 * Checks for horizontal invariance by searching near the zero-disparity region
 * for good matches.  If we find a match, that indicates that this is likely not
 * a true obstacle since it matches in more places than just the single-disparity
 * check.
 *
 * @param leftImage left image
 * @param rightImage right image
 * @param pxX column pixel location
 * @param pxY row pixel location
 * @param state state structure that includes a number of parameters
 *
 * @retval true if there is another match (so NOT an obstacle)
 */
bool PushbroomStereo::CheckHorizontalInvariance(Mat leftImage, Mat rightImage, Mat sobelL, Mat sobelR, int pxX, int pxY) 
{
	// top left corner of the SAD box
    int startX = pxX;
    int startY = pxY;

    // bottom right corner of the SAD box
    int endX = pxX + m_iBlockSize - 1;
    int endY = pxY + m_iBlockSize - 1;


    // if we are off the edge of the image so we can't tell if this might be an issue -- give up and return true
    // (note: this used to be false and caused bad detections on real flight
    // data near the edge of the frame)
    if (   startX + m_iDisparity + INVARIANCE_CHECK_HORZ_OFFSET_MIN < 0 || endX + m_iDisparity + INVARIANCE_CHECK_HORZ_OFFSET_MAX > rightImage.cols)
        return true;
 
    if (startY + INVARIANCE_CHECK_VERT_OFFSET_MIN < 0        || endY + INVARIANCE_CHECK_VERT_OFFSET_MAX > rightImage.rows) 
        // we are limited in the vertical range we can check here be smarter here
        // give up and bail out, deleting potential hits
        return true;

    // here we check a few spots:
    //  1) the expected match at zero-disparity (10-infinity meters away)
    //  2) inf distance, moved up 1-2 pixels
    //  3) inf distance, moved down 1-2 pixels
    //  4) others?

    // 1) check zero-disparity
    int leftVal = 0;

    int right_val_array[MAX_IMAGE_WIDTH];
    int sad_array[MAX_IMAGE_WIDTH];
    int sobel_array[MAX_IMAGE_WIDTH];

    for (int i=0;i<MAX_IMAGE_WIDTH;i++) 
	{
        right_val_array[i] = 0;
        sad_array[i] = 0;
        sobel_array[i] = 0;
    }

    int counter = 0;
    for (int i=startY;i<=endY;i++)
    {
        for (int j=startX;j<=endX;j++)
        {
            // we are now looking at a single pixel value
            uchar pxL = leftImage.at<uchar>(i,j);

            uchar pxR_array[MAX_IMAGE_WIDTH], sR_array[MAX_IMAGE_WIDTH];

            // for each pixel in the left image, we are going to search a bunch
            // of pixels in the right image.  We do it this way to save the computation
            // of dealing with the same left-image pixel over and over again.

            // counter indexes which location we're looking at for this run, so for each
            // pixel in the left image, we examine a bunch of pixels in the right image
            // and add up their results into different slots in sad_array over the loop
            counter = 0;
            for (int vert_offset = INVARIANCE_CHECK_VERT_OFFSET_MIN;vert_offset <= INVARIANCE_CHECK_VERT_OFFSET_MAX; vert_offset+= INVARIANCE_CHECK_VERT_OFFSET_INCREMENT) 
			{
                for (int horz_offset = INVARIANCE_CHECK_HORZ_OFFSET_MIN; horz_offset <= INVARIANCE_CHECK_HORZ_OFFSET_MAX;  horz_offset++) 
				{
                    pxR_array[counter] = rightImage.at<uchar>(i + vert_offset, j + m_iDisparity + horz_offset);
                    sR_array[counter]  = sobelR.at<uchar>    (i + vert_offset, j + m_iDisparity + horz_offset);
                    right_val_array[counter] += sR_array[counter];

                    sad_array[counter] += abs(pxL - pxR_array[counter]);
                    counter ++;
                }
            }

            uchar sL = sobelL.at<uchar>(i,j);

            leftVal += sL;

        }
    }

    for (int i = 0; i < counter; i++)
    {
        sobel_array[i] = leftVal + right_val_array[i];
        // we don't check for leftVal >= sobelLimit because we have already checked that in the main search loop (in GetSAD).        
        //if (right_val_array[i] >= sobelLimit && 100*(float)sad_array[i]/(float)((float)sobel_array[i]*state.interestOperatorMultiplierHorizontalInvariance) < state.sadThreshold) {
        if (right_val_array[i] >= m_iSobelLimit && NUMERIC_CONST*m_fHorizontalInvarianceMultiplier*(float)sad_array[i]/((float)sobel_array[i]) < m_iSadThreshold) {
            return true;
        }
    }
    return false;


}

