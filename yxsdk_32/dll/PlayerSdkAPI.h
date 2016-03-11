#ifndef __PLAYER_SDK_API_H__
#define __PLAYER_SDK_API_H__


#include <winsock2.h>

#define PLAYSDKAPI __declspec(dllexport)


#define PLAY_PROTOCOL_TCP 0
#define PLAY_PROTOCOL_UDP 1

//媒体类型
#ifndef MEDIA_VIDEO
#define MEDIA_VIDEO		0x00000001
#endif
#ifndef MEDIA_AUDIO
#define MEDIA_AUDIO		0x00000002
#endif
#ifndef MEDIA_EVENT
#define MEDIA_EVENT		0x00000004
#endif

#ifndef     MAX_USN_SIZE
#define		MAX_USN_SIZE		(16)
#endif

typedef enum __RENDER_FORMAT
{
	DISPLAY_FORMAT_YV12		=		842094169,
	DISPLAY_FORMAT_YUY2		=		844715353,
	DISPLAY_FORMAT_UYVY		=		1498831189,
	DISPLAY_FORMAT_A8R8G8B8	=		21,
	DISPLAY_FORMAT_X8R8G8B8	=		22,
	DISPLAY_FORMAT_RGB565	=		23,
	DISPLAY_FORMAT_RGB555	=		25,

	DISPLAY_FORMAT_RGB24_GDI=		26
}RENDER_FORMAT;
//显卡
#ifndef GRAPHICS_ADAPTER_DEFAULT
#define GRAPHICS_ADAPTER_DEFAULT	0
#endif

#define	GRAPHICS_ADAPTER_NUM		3		//最多3张显卡
#define	GRAPHICS_FORMAT_NUM			6		//最多6种格式
typedef struct __GRAPHICS_ADAPTER_T
{
	int		num;
	RENDER_FORMAT	format[GRAPHICS_ADAPTER_NUM][GRAPHICS_FORMAT_NUM];
}GRAPHICS_ADAPTER_T;


//提示字符
typedef enum __PLAY_STATUS_TIPS
{
	PLAY_CONNECTING		=	0x01,			//视频连接中
	PLAY_LOSSPACKET,						//视频丢包
}PLAY_STATUS_TIPS;


typedef enum __PLAY_MODE
{
	PLAY_MODE_REALTIME	=	0x00,
	PLAY_MODE_PLAYBACK,
	PLAY_MODE_EXPORT,
	PLAY_MODE_FILE
}PLAY_MODE;

//录像模式
typedef enum __REC_MODE_ENUM
{
	REC_MODE_NO		=	0x00,	//不录像
	//REC_MODE_REALTIME,			//实时录像
	REC_MODE_SCHEDULE,			//排程录像
	//REC_MODE_ALARM,				//报警录像
	REC_MODE_MOTION_DETECT,		//移动侦测
	REC_MODE_DI,				//DI
	REC_MODE_MOTION_OR_DI,		//Motion Or DI
	REC_MODE_MOTION_AND_DI,		//Motion And DI
	REC_MODE_VCA
}REC_MODE_ENUM;

//录像状态
typedef enum __REC_STATUS_ENUM
{
	REC_STATUS_NO	=	0x00,	//没有录像
	REC_STATUS_RECORDING		//录像中
}REC_STATUS_ENUM;

//存储策略
typedef enum __REC_STRATEGY_ENUM
{
	REC_STRATEGY_OVERRIDE	=	0x00,	//覆盖历史录像
	REC_STRATEGY_STOP					//停止录像
}REC_STRATEGY_ENUM;

// 播放速度
typedef enum __PLAY_SPEED_ENUM
{	
	PLAY_SPEED_NORMAL	=	0x00,		// 正常播放
	PLAY_SPEED_PAUSED,					// 暂停
	PLAY_SPEED_SLOW_X2,					// 1/2
	PLAY_SPEED_SLOW_X4,					// 1/4
	PLAY_SPEED_SLOW_X8,					// 1/8
	PLAY_SPEED_FAST_X2,					// x2
	PLAY_SPEED_FAST_X4,					// x4
	PLAY_SPEED_FAST_X8,					// x8
	PLAY_SPEED_ERROR,					// 错误??
}PLAY_SPEED_ENUM;
/*
typedef struct __REC_PARAM
{
	char		usn[MAX_USN_SIZE];
	REC_MODE_ENUM	mode;				//录像模式
	UINT64			schedule[7][24];	//排程(最小单位: 分钟)  7x24小时
	unsigned short	holiday_date[32];	//月日  高8位:月  低8位:日
	UINT64			sch_holiday[32][24];//假日排程(精确到分钟)
	unsigned int	prerec_times;		//预录时长
	unsigned int	duration_times;		//延录时长
}REC_PARAM;
*/
typedef struct __REC_PARAM
{
	char		usn[MAX_USN_SIZE];
	REC_MODE_ENUM	mode;				//录像模式
	unsigned int	rec_time[8][8];		//排程(最小单位: 分钟)  7+1(每周7天+节假日)天x8个时段
	unsigned char	rec_mode[8][8];		//排程对应的录像模式
	unsigned short	holiday_date[32];	//节假日日期  高8位:月  低8位:日
	unsigned int	prerec_times;		//预录时长
	unsigned int	duration_times;		//延录时长
}REC_PARAM;

typedef struct __REC_DATE_T
{
	unsigned short	year;
	unsigned char	month;
	unsigned char	day;
}REC_DATE_T;
typedef struct __REC_TIME_T
{
	REC_DATE_T		date;
	unsigned short	hour;
	unsigned char	minute;
	unsigned char	second;
}REC_TIME_T;


typedef struct __PLAYERSDK_MEDIA_FRAME_INFO
{
	unsigned int	codec;			//编码格式
	unsigned char	type;			//帧类型
	unsigned char	fps;			//帧率
	unsigned char	reserved1;
	unsigned char	reserved2;

	unsigned short	width;			//宽
	unsigned short  height;			//高
	unsigned int	sample_rate;	//采样率
	unsigned int	channels;		//声道
	unsigned int	length;			//帧大小
	unsigned int    rtptimestamp;	//rtp timestamp
	unsigned int	timestamp_sec;	//秒
	//unsigned int	timestamp_usec;	//微秒
	
	float			bitrate;
	float			losspacket;
}PLAYERSDK_MEDIA_FRAME_INFO;
//typedef int (CALLBACK *PLAYER_SOURCE_CallBack)(PLAY_MODE _playMode, int _chid, int _mediatype, char *pbuf, PLAYERSDK_MEDIA_FRAME_INFO *frameinfo);
typedef int (CALLBACK *PLAYER_SOURCE_CallBack)(PLAY_MODE _playMode, int _chid, int *_chPtr, int _mediatype, char *pbuf, PLAYERSDK_MEDIA_FRAME_INFO *frameinfo);



#ifdef __cplusplus
extern "C"
{
#endif

// 获取错误信息
PLAYSDKAPI int Player_GetLastError();	//(暂未实现)


//获取显卡的支持格式
PLAYSDKAPI int Player_GetD3dFormat(GRAPHICS_ADAPTER_T *adapterinfo, int *type=NULL);

// 初始化SDK 指定视频显示格式
PLAYSDKAPI int Player_Init(RENDER_FORMAT renderFormat);

// 关闭SDK
PLAYSDKAPI void Player_Shutdown();

//设置回调函数
PLAYSDKAPI int Player_SetCallback(PLAYER_SOURCE_CallBack _callback);


PLAYSDKAPI int Player_SetTips(PLAY_STATUS_TIPS _id, char *tip);

// 打开码流 返回playID( >= 0 可用)
PLAYSDKAPI int Player_OpenStream(const char *url, const char *username, const char *password, int protocol = PLAY_PROTOCOL_TCP, int reconnection=0x01, int *userPtr=NULL, PLAYER_SOURCE_CallBack _callback=NULL, double starttime=0.0f, double endtime=0.0f, float fScale=1.0f);

// 关闭码流
PLAYSDKAPI int Player_CloseStream(int playID);

// 播放码流
PLAYSDKAPI int Player_PlayStream(int playID, HWND hwnd, int displayVideo=0x01);


//按比例显示  ShownToScale: 0x01按比例显示,其它为铺满窗口
PLAYSDKAPI int Player_SetShownToScale(int playID, int ShownToScale, COLORREF bkColor);
//视频翻转
PLAYSDKAPI int Player_SetVideoFlip(int playID, int flip);
//设置解码类型(仅解码显示关键帧)
PLAYSDKAPI int Player_SetDecodeType(int playID, int decodeKeyframeOnly);
// 设置缓存帧数
PLAYSDKAPI int Player_SetPlayCache(int playID, int cache);
//显示OSD信息
PLAYSDKAPI int Player_ShowOSD(int playID, int show/*0x01:显示  0x00:不显示*/);

//视频上画移动侦测框(当_enable为0x01时, 自动切换为GDI显示,当_enable为0x00时,自动切换回来)
PLAYSDKAPI int Player_EnableMotionGraph(int playID, int _row, int _col, unsigned char *_md_bitmap, int _fill, int _enable);

// 播放文件 返回PlayID( >= 0 可用)
PLAYSDKAPI int Player_StartPlayFile(const char *filename, HWND hwnd, RENDER_FORMAT renderFormat, unsigned int *duration, int *userPtr, PLAYER_SOURCE_CallBack _callback);
PLAYSDKAPI int Player_StopPlayFile(int playID);

//录像回放
PLAYSDKAPI int Player_StartPlayback(const char *usn, REC_TIME_T *pRecTime, HWND hwnd, unsigned int *pcbPlayTime/*回调播放时间*/, int *userPtr);
//切换播放时间
PLAYSDKAPI int Player_ChangePlayTime(int playID, REC_TIME_T *pRecTime);
//同步回放	返回一个新的PlayID  调用Player_StopPlayback关闭该PlayID后,将恢复到异步播放
PLAYSDKAPI int Player_SyncPlayback(int id_num, int *playID, REC_TIME_T *pRecTime, unsigned int *pcbPlayTime/*实时回调播放时间,所以该指针要么为NULL,要么为全局变量*/);
//视频导出	返回一个新的PlayID	可调用Player_StopPlayback关闭该导出线程
PLAYSDKAPI int Player_ExportToFile(const char *pUSN, REC_TIME_T *pStartTime, REC_TIME_T *pEndTime, const char *filename, PLAYER_SOURCE_CallBack _callback);	//指定时间段导出视频

//停止回放
PLAYSDKAPI int Player_StopPlayback(int playID);

// 设置播放速率
PLAYSDKAPI int Player_SetPlaySpeed(int playID, PLAY_SPEED_ENUM speed);

// 获取播放速率
PLAYSDKAPI PLAY_SPEED_ENUM Player_GetPlaySpeed(int playID);

// 单帧前进		可调用Player_SetPlaySpeed再切换回流畅播放
PLAYSDKAPI int Player_NextFrame(int playID);

// 停止实时流播放
PLAYSDKAPI int Player_StopPlay(int playID);

// 播放声音
PLAYSDKAPI int Player_PlaySound(int playID);

// 停止声音
PLAYSDKAPI int Player_StopSound(int playID);

// 设置音量(0~100)
PLAYSDKAPI int Player_SetVolume(int playID, int volume);

// 获取音量
PLAYSDKAPI int Player_GetVolume(int playID);

// 是否播放声音
PLAYSDKAPI int Player_IsSoundPlaying(int playID);

// 截图到文件
PLAYSDKAPI int Player_Capture2File(int playID, const char *filename, int sync=0/*0:异步: 1:同步*/);

// 截图到内存
PLAYSDKAPI int Player_Capture2Memory(int playID, unsigned char *bmpData);

// 获取视频信息
PLAYSDKAPI int Player_GetVideoInfo(int playID, int *codec, int *width, int *height);

//仅用于播放多媒体文件
PLAYSDKAPI int Player_FileSeek(int playID, unsigned int _time_secs);

//====================================================
//======================录像==========================
// 设置录像参数(录像模式切换也调用该函数)
PLAYSDKAPI int Player_SetRecordingParams(int playID, const REC_PARAM *param);
// 获取录像参数
PLAYSDKAPI REC_PARAM *Player_GetRecordingParams(int playID);
// 获取录像状态
PLAYSDKAPI REC_STATUS_ENUM Player_GetRecordingStatus(int playID);

//本地录像(直接指定文件名,和集中存储无关)
PLAYSDKAPI int Player_RecordingToLocalFile(int playID, const char *filename);
PLAYSDKAPI int Player_StopRecordingToLocalFile(int playID);


//根据指定的年月,获取该月有录像数据的日期(int: 4x8=32,每月最大31天,所以一个unsigned int就行)
PLAYSDKAPI	int	Player_GetRecDate(unsigned int _year, unsigned int _month, unsigned int *_date);
//根据日期获取该日期内有录像数据的设备USN
PLAYSDKAPI	int	Player_GetDeviceListByDate(REC_DATE_T *pRecDate, unsigned int usn_num, char **pUSN);
//根据日期和设备USN,和录像类型, 获取一天24小时的所有录像时段(精确到分钟) _time[1440]:每个字节表示当前每分钟的录像类型
//PLAYSDKAPI	int	Player_GetRecTimeByUSN(REC_DATE_T *pRecDate, char *pUSN, unsigned short _recMode, UINT64 _time[48]);
PLAYSDKAPI	int	Player_GetRecTimeByUSN(REC_DATE_T *pRecDate, char *pUSN, unsigned char _time[1440]);


//初始化磁盘
PLAYSDKAPI int Player_SetStorageStrategy(REC_STRATEGY_ENUM	_strategy);		//设置存储策略(覆盖历史文件  or  停止录像)
PLAYSDKAPI int Player_SetRecordingStrategy(unsigned int _strategy);			//设置录像逻辑策略 (录像逻辑由外部控制:0x00 or 内部控制:0x01)
PLAYSDKAPI int Player_InitialDisk(char *pDisk, unsigned int reserveSize=1024);	//reserveSize:保留大小 默认1G
PLAYSDKAPI int Player_GetDiskList(char disk[16]);		//获取已设置的磁盘列表
PLAYSDKAPI int Player_GetDiskStatus(char disk);		//获取磁盘状态  -1:未初始化		0:已初始化
PLAYSDKAPI int Player_AddDisk(char disk, unsigned int reserveSize=2048);			//添加磁盘	reserveSize:保留大小	默认2G(写索引需占用)
PLAYSDKAPI int Player_DelDisk(char disk);			//删除磁盘




//拖拽画框
PLAYSDKAPI int Player_SetDragStartPoint(int playID, float fX, float fY);		//设置拖拽起点
PLAYSDKAPI int Player_SetDragEndPoint(int playID, float fX, float fY);		//设置拖拽终点
PLAYSDKAPI int Player_SetZoomIn(int playID, int zoomIn);			//是否放大显示
PLAYSDKAPI int Player_ResetDragPoint(int playID);					//清空拖拽起点和终点
//设置显示区域
PLAYSDKAPI int Player_SetRenderRect(int playID, LPRECT lpRectSrc);




//2015.09.30
PLAYSDKAPI int Player_PauseStream(int playID);		//暂停播放
PLAYSDKAPI int Player_ResumeStream(int playID);		//继续播放
PLAYSDKAPI int Player_FastPlay(int playID, float fScale);	//调整速率播放	1/16 1/8 1/4 1/2 1 2 4 8 16
PLAYSDKAPI int Player_Seek(int playID, double starttime, double endtime, float fScale);		//定点播放, 根据获取到的文件总时长,设置起始和结束播放时间 及 速率
PLAYSDKAPI int Player_GetStreamTime(int playID, double *currplaytime, double *totaltime);	//获取当前播放时间和总时长
PLAYSDKAPI int Player_StreamIsDisconnect(int playID);		//获取断线状态

#ifdef _DEBUG
PLAYSDKAPI int Player_Debug_Channel(int channelId);
#endif

#ifdef __cplusplus
}
#endif

#endif
