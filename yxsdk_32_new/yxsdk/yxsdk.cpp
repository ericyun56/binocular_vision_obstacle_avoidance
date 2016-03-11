// yxsdk.cpp : 定义控制台应用程序的入口点。

#include "stdafx.h"
#include "PlayerSdkAPI.h"
#include "stdlib.h"
#include "windows.h"

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include "core\utility.hpp"
//#include "cudastereo.hpp"
#include "highgui\highgui.hpp"
#include "imgproc\imgproc.hpp"
#include "calib3d\calib3d.hpp"

#include "pushbroom-stereo.hpp"

using namespace cv;
using namespace std;

#define CAPTURE
#define RECTIFY

PushbroomStereo pushbroomMatcher;

int bShowOnce = 0;
Mat imgRightGB ,picGray;

string windowL_name = "videoL ";
string windowR_name = "videoR ";
int g_ch;

Mat map11, map12, map21, map22;

std::string windowDisparity = "Disp_bm";
std::string windowDisparityCSBP = "Disp_csbp";

VideoWriter output_disparity,output_orignal;

int firstRun = 1;


int ConnectCamera(const char* rtspURL,const char* usrName,const char* pwd,void* hwnd)
{
	int rtn=0;
	rtn=Player_OpenStream(rtspURL,usrName, pwd, PLAY_PROTOCOL_TCP, 0x01, NULL); 
	if (rtn<0)  
		return -1;

	//if (hwnd!=NULL)
	//{
	int rtn2=Player_PlayStream(rtn,(HWND)hwnd,1);  //Player_StopPlay (int playID)
	if (rtn2!=0)
		return -1; 
	//}

	return rtn;  
}

int iCount = 0;
int CALLBACK StreamPlayerCallback(PLAY_MODE _playMode, int _chid, int *_chPtr, int _mediatype, char *pbuf, PLAYERSDK_MEDIA_FRAME_INFO *frameinfo)
{
	if (NULL == pbuf || NULL == frameinfo)
		return 0;

	if (_playMode == PLAY_MODE_REALTIME)
	{
		unsigned int _format = (frameinfo->codec);
		if (_format == DISPLAY_FORMAT_RGB24_GDI)
		{	
			imgRightGB = Mat( 1080 , 3840 , CV_8UC3,pbuf); // 按照 RGB24 R-8 G-8 B-8 格式，将图片数据存储为 Mat 对象格式
			Rect rectL(0,0,1920,1080);  
			Rect rectR(1920,0,1920,1080); 


#ifdef CAPTURE
			Mat imgLeftCor;
			Mat imgRightCor;
			imgRightGB(rectL).copyTo(imgLeftCor);  // 把彩色图片对半拆开保存
			imgRightGB(rectR).copyTo(imgRightCor);

			cvtColor(imgRightGB, picGray, COLOR_RGB2GRAY); // 将完整图片变成灰度图
#endif


			Mat imgLeft;
			Mat imgRight;

			picGray(rectL).copyTo(imgLeft);  // 把灰度图对半拆开保存
			picGray(rectR).copyTo(imgRight);

#ifdef RECTIFY
			//cv::remap(imgLeft, imgLeft, map11, map12, INTER_LINEAR);
			//cv::remap(imgRight, imgRight, map21, map22, INTER_LINEAR);

			Mat imgLeftRectify;
			Mat imgRightRectify;
			imgLeft.copyTo(imgLeftRectify);      // 另外保存两张对半拆开的灰度图
			imgRight.copyTo(imgRightRectify);
#endif

			Size img_size2;
			img_size2.width = 1000;
			img_size2.height = 562;
			resize(imgLeft,imgLeft,img_size2);   // 裁剪两张灰度图到（1000，562 ）范围，这个操作无所谓，只是程序员想截取这一部分图片做立体算法处理而已，不影响算法实现
			resize(imgRight,imgRight,img_size2);

			if( imgLeft.empty() || imgRight.empty() ) {
				std::cout<< " --(!) Error reading images " << std::endl;
				return -1;
			}

			//新的pushbroom方法
			iCount++;
			pushbroomMatcher.m_iDisparity -= 10;
			if(iCount>14)
			{
				iCount = 0;
				pushbroomMatcher.m_iDisparity = 160;
			}
			std::vector<Point3f> pointVector3d; // 定义一个 float			类型的3维向量
			std::vector<uchar> pointColors;     // 定义一个 unsigned char	类型的变量
			std::vector<Point3i> pointVector2d; // 定义一个 int				类型的3维向量
			pushbroomMatcher.ProcessImages2(imgLeft,imgRight,&pointVector3d,&pointVector2d,&pointColors);

			int iLen = pointVector2d.size();
			for(int i=0; i<iLen; i++)
			{
				Point3i pt = pointVector2d[i];
				//imgDisparity8U.data[pt.x+pt.y*imgDisparity8U.cols] = 255;
				uchar* Mi = imgLeft.ptr <uchar>(pt.y) ;
				Mi[pt.x] = 255;
				Mi = imgRight.ptr <uchar>(pt.y) ;
				Mi[pt.x+pushbroomMatcher.m_iDisparity] = 255;
				//Mi[pt.x*3+1] = 0;
				//Mi[pt.x*3+2] = 0;			
			}
			iLen= pointVector3d.size();
			for(int i=0; i<iLen; i++)
			{
				Point3f pt = pointVector3d[i];
				int w = i+1;
			}

			imshow(windowL_name, imgLeft);
			imshow(windowR_name, imgRight);
			//imshow(windowDisparity, imgLeft);

			g_ch = waitKey(100);

			printf("Callback...%d\n",pushbroomMatcher.m_iDisparity);
			bShowOnce = 1;
		}
	}
	return 1;
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

	int i=0;

	intrinsic_filename="data\\G954intrinsics.yml";
	extrinsic_filename ="data\\G954extrinsics.yml"; 

	Size img_size;
	img_size.width = 1920;
	img_size.height = 1080;

	//output_disparity.open("D:\\data\\yxsdk_out\\v_disparity.avi",CV_FOURCC('M','J','P','G'),20,img_size,1);

#ifdef RECTIFY	
	if (!intrinsic_filename.empty()&&!extrinsic_filename.empty()) 
	{
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

		pushbroomMatcher.m_matQ = Q;
	}
#endif

	InitStitchingSDK();
	int chnid=ConnectCamera("rtsp://192.168.200.168:8557/PSIA/Streaming/channels/2?videoCodecType=H.264","admin","9999",NULL);

	while( bShowOnce == 0 )
		Sleep(200);

	while (1)
	{
		//waitKey(100); //delay N millis, usually long enough to display and capture input
		if( g_ch == 27)
			break;
	}

	Player_StopPlay(chnid);
	Player_Shutdown(); 
	//	Sleep(20000);
	return 0;
}

