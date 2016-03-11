// yxsdk.cpp : 定义控制台应用程序的入口点。

#include "stdafx.h"
#include "..\dll\PlayerSdkAPI.h"
#include "stdlib.h"
#include "windows.h"

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include "core\utility.hpp"
#include "cudastereo.hpp"
#include "highgui.hpp"
#include "imgproc.hpp"

using namespace cv;
using namespace std;

#define USE_GUDA
#define CAPTURE
#define RECTIFY

int bShowOnce = 0;
Mat imgRightGB ,picGray;

string windowL_name = "videoL ";
string windowR_name = "videoR ";
int g_ch;

Mat map11, map12, map21, map22;
#ifdef USE_GUDA
    cuda::GpuMat g_imgLeft, g_imgRight, g_map11, g_map12, g_map21, g_map22;
    Ptr<cuda::StereoBM> bm;
    Ptr<cuda::StereoBeliefPropagation> bp;
    Ptr<cuda::StereoConstantSpaceBP> csbp;
#endif 
std::string windowDisparity = "Disp_bm";
std::string windowDisparityCSBP = "Disp_csbp";

VideoWriter output_disparity,output_orignal;

int firstRun = 1;

int g_para = 1;
void onChangeTrackBar (int pos,void* usrdata)
{
	g_para = pos;
}

int g_para2 = 4;
void onChangeTrackBar2 (int pos,void* usrdata)
{
	g_para2 = pos;
}

int g_para3 = 4;
void onChangeTrackBar3(int pos,void* usrdata)
{
	g_para3 = pos;
}


int g_para4 = 1;
void onChangeTrackBar4 (int pos,void* usrdata)
{
	g_para4 = pos;
}

int g_para5 = 4;
void onChangeTrackBar5 (int pos,void* usrdata)
{
	g_para5 = pos;
}

int g_para6 = 4;
void onChangeTrackBar6(int pos,void* usrdata)
{
	g_para6 = pos;
}

int ConnectCamera(const char* rtspURL,const char* usrName,const char* pwd,void* hwnd)
{
    int rtn=0;
    rtn=Player_OpenStream(rtspURL,usrName, pwd, PLAY_PROTOCOL_TCP, 0x01, NULL); //œ«Para×÷Îª²ÎÊýŽ«Èë
    if (rtn<0)  //³É¹ŠÊ±ŽóÓÚµÈÓÚ0
        return -1;
 
    //if (hwnd!=NULL)
    //{
        int rtn2=Player_PlayStream(rtn,(HWND)hwnd,1);  //Player_StopPlay (int playID)
        if (rtn2!=0)
            return -1; //²¥·ÅÊÓÆµÁ÷Ê§°Ü
    //}
 
    return rtn;  //·µ»ØÊÓÆµÁ÷ID
}
int capturePicIndex = 0;
int CALLBACK StreamPlayerCallback(PLAY_MODE _playMode, int _chid, int *_chPtr, int _mediatype, char *pbuf, PLAYERSDK_MEDIA_FRAME_INFO *frameinfo)
{
	

    if (NULL == pbuf || NULL == frameinfo)
        return 0;

    if (_playMode == PLAY_MODE_REALTIME)
    {
        unsigned int _format = (frameinfo->codec);
        if (_format == DISPLAY_FORMAT_RGB24_GDI)
        {	
			imgRightGB = Mat( 1080 , 3840 , CV_8UC3,pbuf);
			Rect rectL(0,0,1920,1080);  
			Rect rectR(1920,0,1920,1080); 


#ifdef CAPTURE
			Mat imgLeftCor;
			Mat imgRightCor;
			imgRightGB(rectL).copyTo(imgLeftCor);
			imgRightGB(rectR).copyTo(imgRightCor);

			cvtColor(imgRightGB, picGray, COLOR_RGB2GRAY);
#endif
			

			Mat imgLeft;
			Mat imgRight;

            picGray(rectL).copyTo(imgLeft);  
			picGray(rectR).copyTo(imgRight);

#ifdef RECTIFY
			cv::remap(imgLeft, imgLeft, map11, map12, INTER_LINEAR);
            cv::remap(imgRight, imgRight, map21, map22, INTER_LINEAR);

			Mat imgLeftRectify;
			Mat imgRightRectify;
			imgLeft.copyTo(imgLeftRectify);
			imgRight.copyTo(imgRightRectify);
			//imshow("src_gray_left",imgLeft);
			//imshow("src_gray_right",imgRight);
#endif

			Size img_size2;
			img_size2.width = 500;
			img_size2.height = 281;

			resize(imgLeft,imgLeft,img_size2);
			resize(imgRight,imgRight,img_size2);

#ifdef USE_GUDA
		//	g_map11.upload(map11);
		//	g_map12.upload(map12);
		//	g_map21.upload(map11);
		//	g_map22.upload(map22);
		//	imgLeft.resize(0.5);
		//	imgRight.resize(0.5);
			g_imgLeft.upload(imgLeft);
			g_imgRight.upload(imgRight);   
#endif   
			
		
		Mat imgDisparity16S = Mat( imgLeft.rows, imgLeft.cols, CV_16S );
		cuda::GpuMat GPUimgDisparity16S = cuda::GpuMat( imgLeft.rows, imgLeft.cols, CV_16S );
        Mat imgDisparity8U = Mat( imgLeft.rows, imgLeft.cols, CV_8UC1 );

        if( imgLeft.empty() || imgRight.empty() ) {
            std::cout<< " --(!) Error reading images " << std::endl;
            return -1;
        }

		int posTrackBar1=11;
		int ndisparities = 16*(posTrackBar1+1); 
        //-- 2. Call the constructor for StereoBM
		if( firstRun )
		{
			firstRun = 0;

			namedWindow(windowL_name, WINDOW_NORMAL); 
			namedWindow(windowR_name, WINDOW_NORMAL); 
			namedWindow( windowDisparity, WINDOW_NORMAL );
			//namedWindow( windowDisparityCSBP, WINDOW_NORMAL );
			
			int maxValue1=15;    //NUMdisparity
			createTrackbar ("(ND+1)*16:",windowL_name,&posTrackBar1,maxValue1, onChangeTrackBar );

			int posTrackBar2=1;   //BLOCKSIZE
			int maxValue2=20;
			createTrackbar ("BS*2+3:",windowL_name,&posTrackBar2,maxValue2, onChangeTrackBar2 );

			int posTrackBar3=1;  //TEXTURE
			int maxValue3=20;
			createTrackbar ("Textu|Iters",windowL_name,&posTrackBar3,maxValue3, onChangeTrackBar3 );

			g_para = posTrackBar1;
			g_para2 = posTrackBar2;
			g_para3 = posTrackBar3;		


			int posTrackBar4=0; 
			int maxValue4=15;    //min Disparity, 负数，0
			createTrackbar ("mD-10:",windowR_name,&posTrackBar4,maxValue4, onChangeTrackBar4 );

			int posTrackBar5=1;   //SpeckleWindowSize
			int maxValue5=50;
			createTrackbar ("SWS*3|NumLev:",windowR_name,&posTrackBar5,maxValue5, onChangeTrackBar5 );

			int posTrackBar6=1;  //
			int maxValue6=30;
			createTrackbar ("SR*2|NrPlane",windowR_name,&posTrackBar6,maxValue6, onChangeTrackBar6 );

			g_para4 = posTrackBar4;
			g_para5 = posTrackBar5;
			g_para6 = posTrackBar6;			
		}

#ifdef USE_GUDA

	#if 0
			double minVal;
			double maxVal;
			int method=1;
			int64 t = getTickCount();
			cuda::GpuMat g_imgDisparity8U (imgDisparity8U.size(), CV_8U);
			bm = cuda::createStereoBM(ndisparities);


			csbp = cuda::createStereoConstantSpaceBP(32, 8,  4,  4,  CV_16SC1 );

			//bm->setBlockSize(50);
			//bm->setSmallerBlockSize(5);
			//bm->setUniquenessRatio(15);
			//bm->setDisp12MaxDiff(1);
		
			bm->setNumDisparities (  (g_para+1) * 16 );
			bm->setBlockSize ( g_para2*2 + 3 );
			bm->setTextureThreshold ( g_para3 );
			bm->setMinDisparity( g_para4 - 10 );   //一般取0或负值
			bm->setSpeckleWindowSize(g_para5*3);
			bm->setSpeckleRange(g_para6*2);

			bm->setPreFilterType(CV_STEREO_BM_XSOBEL);
			bm->setPreFilterSize(7);
			bm->setPreFilterCap(32);
			bm->setBlockSize(11);
			bm->setMinDisparity(0);
			bm->setNumDisparities(96);
			bm->setTextureThreshold(8);
			bm->setUniquenessRatio(30);
			bm->setSpeckleWindowSize(200);
			bm->setSpeckleRange(10);
			bm->setDisp12MaxDiff(1);

			bm->compute(g_imgRight,g_imgLeft,g_imgDisparity8U);
			//bm->compute(g_imgLeft,g_imgRight,g_imgDisparity8U);
			g_imgDisparity8U.download(imgDisparity8U);

			Mat norm_imgDisparity8U;
			minMaxLoc( imgDisparity8U, &minVal, &maxVal );
			imgDisparity8U.convertTo( norm_imgDisparity8U, CV_8UC1, 255/(maxVal - minVal));
			t = getTickCount() - t;
			printf("bm Time elapsed: %fms\n", t*1000/getTickFrequency());
			imshow( windowDisparity, norm_imgDisparity8U );


			//csbp->setNumDisparities (  (g_para+1) * 16 );
			//csbp->setBlockSize ( g_para2*2 + 3 );
			//csbp->setNumIters ( g_para3 + 2 );

			//csbp->setMinDisparity( g_para4 - 10 );   //一般取0或负值
			//csbp->setNumLevels(g_para5*2 + 2);
			//csbp->setNrPlane(g_para6*2 + 2);

			// t = getTickCount();
			//cuda::GpuMat g_imgDisparity16S(imgLeft.size(), CV_16SC1); 
	

			//csbp->compute(g_imgLeft,g_imgRight,g_imgDisparity16S);
			//g_imgDisparity16S.download(imgDisparity16S);
			//double minVal;
			//      double maxVal;

			//      minMaxLoc( imgDisparity16S, &minVal, &maxVal );
			//      //-- 4. Display it as a CV_8UC1 image
			//      imgDisparity16S.convertTo( imgDisparity8U, CV_8UC1, 255/(maxVal - minVal));
			//t = getTickCount() - t;
			//printf("csbp Time elapsed: %fms\n", t*1000/getTickFrequency());
			//imshow(windowDisparityCSBP,imgDisparity8U);
	#else
		//CvStereoBMState *pState = cvCreateStereoBMState(); 
		//pState->preFilterType=CV_STEREO_BM_XSOBEL; 
		//pState->preFilterSize=21; //// ~5x5..21x21
		//pState->preFilterCap = 31; // up to ~31
		//pState->SADWindowSize = 11; // Could be 5x5,7x7...21x21
		//pState->minDisparity = 0; // minimum disparity
		//pState->numberOfDisparities = 96; // maximum disparity - minimum disparity, numbers of pixels to search
		//pState->textureThreshold=20; // areas with no texture are ignored
		//pState->uniquenessRatio = 10; // filter out pixels if there are other close matches with different disparity
		//pState->speckleWindowSize =20; //the maximum area of speckles to remove (set to 0 to disable speckle filtering)
		//pState->speckleRange = 10;  // acceptable range of disparity variation in each connected component
		//pState->disp12MaxDiff = 1;

		//IplImage* disp=cvCreateImage(cvSize(imgLeft.cols,imgLeft.rows),IPL_DEPTH_32F,1);
		//IplImage* norm_disp=cvCreateImage(cvSize(imgLeft.cols,imgLeft.rows),IPL_DEPTH_8U,1);
		//IplImage* img1=&IplImage(imgLeft);
		//IplImage* img2=&IplImage(imgRight);

		//cvFindStereoCorrespondenceBM( img2, img1, disp,pState);  
		//cvNormalize( disp, norm_disp, 0, 256, CV_MINMAX );  
		//cvShowImage("bm",norm_disp);

		Ptr<StereoSGBM> sgbm = StereoSGBM::create(0,96,11);
		sgbm->setBlockSize(7);
		int SADWindowSize = 7;
		int cn = imgLeft.channels();
		sgbm->setPreFilterCap(31);
		sgbm->setP1(4*cn*SADWindowSize*SADWindowSize);
		sgbm->setP2(16*cn*SADWindowSize*SADWindowSize);
		sgbm->setMinDisparity(0);
		sgbm->setNumDisparities(96);
		sgbm->setUniquenessRatio(1);
		sgbm->setSpeckleWindowSize(0);
		sgbm->setSpeckleRange(40);
		sgbm->setDisp12MaxDiff(1);
		Mat disp, disp8;
		int64 t = getTickCount();
		sgbm->compute( imgRight,imgLeft, disp);
		t = getTickCount() - t;
		double minVal;
		double maxVal;
		minMaxLoc(disp, &minVal, &maxVal);
		disp.convertTo(disp8, CV_8U, 255/(maxVal - minVal));

		Mat resizeDisp;
		resize(disp8,resizeDisp,cvSize(960,540));
		imshow("sgbm",resizeDisp);
	#endif
        
#else 
 
        Ptr<StereoBM> sbm = StereoBM::create( ndisparities, SADWindowSize );
        //Ptr<StereoSGBM> sbm = StereoSGBM::create(8, ndisparities, SADWindowSize );
        int64 t = getTickCount();		

        //-- 3. Calculate the disparity image
        sbm->compute( imgLeft, imgRight, imgDisparity16S );
        t = getTickCount() - t;
		printf("Time elapsed: %fms\n", t*1000/getTickFrequency());

		//-- Check its extreme values
        double minVal;
        double maxVal;

        minMaxLoc( imgDisparity16S, &minVal, &maxVal );

        printf("Min disp: %f Max value: %f \n", minVal, maxVal);

        //-- 4. Display it as a CV_8UC1 image
        imgDisparity16S.convertTo( imgDisparity8U, CV_8UC1, 255/(maxVal - minVal));
#endif        
		Size img_size;
		img_size.width = 1920;
		img_size.height = 1080;
		// VideoWriter output_disparity("D:\\data\\yxsdk_out\\v_disparity.avi",CV_FOURCC('M','J','P','G'),10,img_size,1);//输出灰度视频
		// VideoWriter output_orignal("D:\\data\\yxsdk_out\\v_orignal_left.avi",CV_FOURCC('M','J','P','G'),10,img_size,1);//输出灰度视频

		 
		//output_disparity << imgDisparity8U;
		//output_orignal << imgLeft;
		
		imshow(windowL_name, imgLeft);
		imshow(windowR_name, imgRight);

		//	imwrite("D:\\data\\yxsdk_out\\imgLeft.jpg",imgLeft);
		//	imwrite("D:\\data\\yxsdk_out\\imgRight.jpg",imgRight);
		g_ch = waitKey(30);

#ifdef CAPTURE

		if(g_ch == 's' )
		{
			string fileName;
			std::stringstream ss,ssr;
			ss << "d:\\data\\capture\\left" << capturePicIndex << ".jpg";
			ss >> fileName;

			imwrite(fileName , imgLeftCor);

			ssr << "d:\\data\\capture\\right" << capturePicIndex++ << ".jpg";
			ssr >> fileName;

			imwrite(fileName , imgRightCor);

			std::stringstream ssREc,ssREC2;
			ssREc << "d:\\data\\capture\\leftRec" << capturePicIndex << ".jpg";
			ssREc >> fileName;

			imwrite(fileName , imgLeftRectify);

			ssREC2 << "d:\\data\\capture\\rightRec" << capturePicIndex++ << ".jpg";
			ssREC2 >> fileName;

			imwrite(fileName , imgRightRectify);
		}
#endif
		printf("Callback...\n");
		bShowOnce = 1;
		}
	}
}

bool InitStitchingSDK()
{
    GRAPHICS_ADAPTER_T adapterinfo;
    int type;
    int rtn=-2;
    rtn=Player_GetD3dFormat(&adapterinfo,&type); 
     
    rtn=Player_Init(DISPLAY_FORMAT_RGB24_GDI); 
 
    rtn=Player_SetCallback(StreamPlayerCallback);
    if(rtn!=0)
        return false;
 
    return true;
}
std::string intrinsic_filename;
std::string extrinsic_filename;
int _tmain(int argc, _TCHAR* argv[])
{
	

	InitStitchingSDK();
	int chnid=ConnectCamera("rtsp://192.168.200.168:8557/PSIA/Streaming/channels/2?videoCodecType=H.264","admin","9999",NULL);

	int i=0;

	intrinsic_filename="E:\\新建文件夹\\G954intrinsics.yml";
    extrinsic_filename ="E:\\新建文件夹\\G954extrinsics.yml"; 
	
	Size img_size;
    img_size.width = 1920;
    img_size.height = 1080;

	//output_disparity.open("D:\\data\\yxsdk_out\\v_disparity.avi",CV_FOURCC('M','J','P','G'),20,img_size,1);

#ifdef RECTIFY	
	if (!intrinsic_filename.empty()&&!extrinsic_filename.empty()) {
        // reading intrinsic parameters
        FileStorage fs(intrinsic_filename, FileStorage::READ);
        if (!fs.isOpened()) {
            printf("Failed to open file %s\n", intrinsic_filename);
            return -1;
        }

        Mat M1, D1, M2, D2,Q;
        fs["M1"] >> M1;
        fs["D1"] >> D1;
        fs["M2"] >> M2;
        fs["D2"] >> D2;


        fs.open(extrinsic_filename, FileStorage::READ);
        if (!fs.isOpened()) {
            printf("Failed to open file %s\n", extrinsic_filename);
            return -1;
        }

        Mat R, T, R1, P1, R2, P2;
        fs["R"] >> R;
        fs["T"] >> T;

		cv::fisheye::stereoRectify(M1, D1, M2, D2, img_size, R, T, R1, R2, P1, P2, Q, CALIB_ZERO_DISPARITY);
        cv::fisheye::initUndistortRectifyMap(M1, D1, R1, P1, img_size, CV_16SC2, map11, map12);
        cv::fisheye::initUndistortRectifyMap(M2, D2, R2, P2, img_size, CV_16SC2, map21, map22);
    }
#endif
	
//	namedWindow(windowL_name, WINDOW_KEEPRATIO); //resizable window;  //callback函数中不能成功显示
//    namedWindow(windowR_name, WINDOW_KEEPRATIO); //resizable window;

	while( bShowOnce == 0 )
		Sleep(200);

	while (1)
	{
		waitKey(150); //delay N millis, usually long enough to display and capture input

		if( g_ch == 27)
		{
				break;
		}
	}

	Player_StopPlay(chnid);
    Player_Shutdown(); 
//	Sleep(20000);
	return 0;
}

/*

int main(int ac, char** av)
{

    if (ac < 3) {
        help(av);
        return 1;
    }
   // std::string arg1 = av[1];
   // std::string arg2 = av[2];

   std::string arg1 = "rtsp://192.168.200.159:8557/PSIA/Streaming/channels/2?videoCodecType=H.264";
 
    VideoCapture captureL(arg1); //try to open string, this will attempt to open it as a video file or image sequence

    if (!captureL.isOpened()) //if this fails, try to open as a video camera, through the use of an integer param
        captureL.open(atoi(arg1.c_str()));
    if (!captureL.isOpened()) {
        cerr << "Failed to open the left video device, video file or image sequence!\n" << endl;
       
        return 1;
    }
   

    long totalFrameNumber = captureL.get(CV_CAP_PROP_FRAME_COUNT);
    cout<<"totalFrameNumber is "<<totalFrameNumber<<endl;
    long frameToStart = 0;
    captureL.set( CV_CAP_PROP_POS_FRAMES,frameToStart);
    double rate = captureL.get(CV_CAP_PROP_FPS);
    cout<<"the frame rate is "<<rate<<endl;

    Size img_size;
    img_size.width=captureL.get(CV_CAP_PROP_FRAME_WIDTH);
    img_size.height=captureL.get(CV_CAP_PROP_FRAME_HEIGHT);

    Rect roi1, roi2;
    Mat Q;

    string windowL_name = "videoL | q or esc to quit";
    string windowR_name = "videoR | q or esc to quit";
    cout << "press space to save a picture. q or esc to quit" << endl;
    namedWindow(windowL_name, WINDOW_KEEPRATIO); //resizable window;
    namedWindow(windowR_name, WINDOW_KEEPRATIO); //resizable window;
    Mat frameL,frameR;

    
    for (;;) {
        captureL >> frameL;
      //  captureR >> frameR;
        if (frameL.empty())
            break;

        imshow(windowL_name, frameL);
       waitKey(300);

	}
    return 0;
}*/



