#ifndef __PLAYER_SDK_API_H__
#define __PLAYER_SDK_API_H__


#include <winsock2.h>

#define PLAYSDKAPI __declspec(dllexport)


#define PLAY_PROTOCOL_TCP 0
#define PLAY_PROTOCOL_UDP 1

//ý������
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
//�Կ�
#ifndef GRAPHICS_ADAPTER_DEFAULT
#define GRAPHICS_ADAPTER_DEFAULT	0
#endif

#define	GRAPHICS_ADAPTER_NUM		3		//���3���Կ�
#define	GRAPHICS_FORMAT_NUM			6		//���6�ָ�ʽ
typedef struct __GRAPHICS_ADAPTER_T
{
	int		num;
	RENDER_FORMAT	format[GRAPHICS_ADAPTER_NUM][GRAPHICS_FORMAT_NUM];
}GRAPHICS_ADAPTER_T;


//��ʾ�ַ�
typedef enum __PLAY_STATUS_TIPS
{
	PLAY_CONNECTING		=	0x01,			//��Ƶ������
	PLAY_LOSSPACKET,						//��Ƶ����
}PLAY_STATUS_TIPS;


typedef enum __PLAY_MODE
{
	PLAY_MODE_REALTIME	=	0x00,
	PLAY_MODE_PLAYBACK,
	PLAY_MODE_EXPORT,
	PLAY_MODE_FILE
}PLAY_MODE;

//¼��ģʽ
typedef enum __REC_MODE_ENUM
{
	REC_MODE_NO		=	0x00,	//��¼��
	//REC_MODE_REALTIME,			//ʵʱ¼��
	REC_MODE_SCHEDULE,			//�ų�¼��
	//REC_MODE_ALARM,				//����¼��
	REC_MODE_MOTION_DETECT,		//�ƶ����
	REC_MODE_DI,				//DI
	REC_MODE_MOTION_OR_DI,		//Motion Or DI
	REC_MODE_MOTION_AND_DI,		//Motion And DI
	REC_MODE_VCA
}REC_MODE_ENUM;

//¼��״̬
typedef enum __REC_STATUS_ENUM
{
	REC_STATUS_NO	=	0x00,	//û��¼��
	REC_STATUS_RECORDING		//¼����
}REC_STATUS_ENUM;

//�洢����
typedef enum __REC_STRATEGY_ENUM
{
	REC_STRATEGY_OVERRIDE	=	0x00,	//������ʷ¼��
	REC_STRATEGY_STOP					//ֹͣ¼��
}REC_STRATEGY_ENUM;

// �����ٶ�
typedef enum __PLAY_SPEED_ENUM
{	
	PLAY_SPEED_NORMAL	=	0x00,		// ��������
	PLAY_SPEED_PAUSED,					// ��ͣ
	PLAY_SPEED_SLOW_X2,					// 1/2
	PLAY_SPEED_SLOW_X4,					// 1/4
	PLAY_SPEED_SLOW_X8,					// 1/8
	PLAY_SPEED_FAST_X2,					// x2
	PLAY_SPEED_FAST_X4,					// x4
	PLAY_SPEED_FAST_X8,					// x8
	PLAY_SPEED_ERROR,					// ����??
}PLAY_SPEED_ENUM;
/*
typedef struct __REC_PARAM
{
	char		usn[MAX_USN_SIZE];
	REC_MODE_ENUM	mode;				//¼��ģʽ
	UINT64			schedule[7][24];	//�ų�(��С��λ: ����)  7x24Сʱ
	unsigned short	holiday_date[32];	//����  ��8λ:��  ��8λ:��
	UINT64			sch_holiday[32][24];//�����ų�(��ȷ������)
	unsigned int	prerec_times;		//Ԥ¼ʱ��
	unsigned int	duration_times;		//��¼ʱ��
}REC_PARAM;
*/
typedef struct __REC_PARAM
{
	char		usn[MAX_USN_SIZE];
	REC_MODE_ENUM	mode;				//¼��ģʽ
	unsigned int	rec_time[8][8];		//�ų�(��С��λ: ����)  7+1(ÿ��7��+�ڼ���)��x8��ʱ��
	unsigned char	rec_mode[8][8];		//�ų̶�Ӧ��¼��ģʽ
	unsigned short	holiday_date[32];	//�ڼ�������  ��8λ:��  ��8λ:��
	unsigned int	prerec_times;		//Ԥ¼ʱ��
	unsigned int	duration_times;		//��¼ʱ��
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
	unsigned int	codec;			//�����ʽ
	unsigned char	type;			//֡����
	unsigned char	fps;			//֡��
	unsigned char	reserved1;
	unsigned char	reserved2;

	unsigned short	width;			//��
	unsigned short  height;			//��
	unsigned int	sample_rate;	//������
	unsigned int	channels;		//����
	unsigned int	length;			//֡��С
	unsigned int    rtptimestamp;	//rtp timestamp
	unsigned int	timestamp_sec;	//��
	//unsigned int	timestamp_usec;	//΢��
	
	float			bitrate;
	float			losspacket;
}PLAYERSDK_MEDIA_FRAME_INFO;
//typedef int (CALLBACK *PLAYER_SOURCE_CallBack)(PLAY_MODE _playMode, int _chid, int _mediatype, char *pbuf, PLAYERSDK_MEDIA_FRAME_INFO *frameinfo);
typedef int (CALLBACK *PLAYER_SOURCE_CallBack)(PLAY_MODE _playMode, int _chid, int *_chPtr, int _mediatype, char *pbuf, PLAYERSDK_MEDIA_FRAME_INFO *frameinfo);



#ifdef __cplusplus
extern "C"
{
#endif

// ��ȡ������Ϣ
PLAYSDKAPI int Player_GetLastError();	//(��δʵ��)


//��ȡ�Կ���֧�ָ�ʽ
PLAYSDKAPI int Player_GetD3dFormat(GRAPHICS_ADAPTER_T *adapterinfo, int *type=NULL);

// ��ʼ��SDK ָ����Ƶ��ʾ��ʽ
PLAYSDKAPI int Player_Init(RENDER_FORMAT renderFormat);

// �ر�SDK
PLAYSDKAPI void Player_Shutdown();

//���ûص�����
PLAYSDKAPI int Player_SetCallback(PLAYER_SOURCE_CallBack _callback);


PLAYSDKAPI int Player_SetTips(PLAY_STATUS_TIPS _id, char *tip);

// ������ ����playID( >= 0 ����)
PLAYSDKAPI int Player_OpenStream(const char *url, const char *username, const char *password, int protocol = PLAY_PROTOCOL_TCP, int reconnection=0x01, int *userPtr=NULL, PLAYER_SOURCE_CallBack _callback=NULL, double starttime=0.0f, double endtime=0.0f, float fScale=1.0f);

// �ر�����
PLAYSDKAPI int Player_CloseStream(int playID);

// ��������
PLAYSDKAPI int Player_PlayStream(int playID, HWND hwnd, int displayVideo=0x01);


//��������ʾ  ShownToScale: 0x01��������ʾ,����Ϊ��������
PLAYSDKAPI int Player_SetShownToScale(int playID, int ShownToScale, COLORREF bkColor);
//��Ƶ��ת
PLAYSDKAPI int Player_SetVideoFlip(int playID, int flip);
//���ý�������(��������ʾ�ؼ�֡)
PLAYSDKAPI int Player_SetDecodeType(int playID, int decodeKeyframeOnly);
// ���û���֡��
PLAYSDKAPI int Player_SetPlayCache(int playID, int cache);
//��ʾOSD��Ϣ
PLAYSDKAPI int Player_ShowOSD(int playID, int show/*0x01:��ʾ  0x00:����ʾ*/);

//��Ƶ�ϻ��ƶ�����(��_enableΪ0x01ʱ, �Զ��л�ΪGDI��ʾ,��_enableΪ0x00ʱ,�Զ��л�����)
PLAYSDKAPI int Player_EnableMotionGraph(int playID, int _row, int _col, unsigned char *_md_bitmap, int _fill, int _enable);

// �����ļ� ����PlayID( >= 0 ����)
PLAYSDKAPI int Player_StartPlayFile(const char *filename, HWND hwnd, RENDER_FORMAT renderFormat, unsigned int *duration, int *userPtr, PLAYER_SOURCE_CallBack _callback);
PLAYSDKAPI int Player_StopPlayFile(int playID);

//¼��ط�
PLAYSDKAPI int Player_StartPlayback(const char *usn, REC_TIME_T *pRecTime, HWND hwnd, unsigned int *pcbPlayTime/*�ص�����ʱ��*/, int *userPtr);
//�л�����ʱ��
PLAYSDKAPI int Player_ChangePlayTime(int playID, REC_TIME_T *pRecTime);
//ͬ���ط�	����һ���µ�PlayID  ����Player_StopPlayback�رո�PlayID��,���ָ����첽����
PLAYSDKAPI int Player_SyncPlayback(int id_num, int *playID, REC_TIME_T *pRecTime, unsigned int *pcbPlayTime/*ʵʱ�ص�����ʱ��,���Ը�ָ��ҪôΪNULL,ҪôΪȫ�ֱ���*/);
//��Ƶ����	����һ���µ�PlayID	�ɵ���Player_StopPlayback�رոõ����߳�
PLAYSDKAPI int Player_ExportToFile(const char *pUSN, REC_TIME_T *pStartTime, REC_TIME_T *pEndTime, const char *filename, PLAYER_SOURCE_CallBack _callback);	//ָ��ʱ��ε�����Ƶ

//ֹͣ�ط�
PLAYSDKAPI int Player_StopPlayback(int playID);

// ���ò�������
PLAYSDKAPI int Player_SetPlaySpeed(int playID, PLAY_SPEED_ENUM speed);

// ��ȡ��������
PLAYSDKAPI PLAY_SPEED_ENUM Player_GetPlaySpeed(int playID);

// ��֡ǰ��		�ɵ���Player_SetPlaySpeed���л�����������
PLAYSDKAPI int Player_NextFrame(int playID);

// ֹͣʵʱ������
PLAYSDKAPI int Player_StopPlay(int playID);

// ��������
PLAYSDKAPI int Player_PlaySound(int playID);

// ֹͣ����
PLAYSDKAPI int Player_StopSound(int playID);

// ��������(0~100)
PLAYSDKAPI int Player_SetVolume(int playID, int volume);

// ��ȡ����
PLAYSDKAPI int Player_GetVolume(int playID);

// �Ƿ񲥷�����
PLAYSDKAPI int Player_IsSoundPlaying(int playID);

// ��ͼ���ļ�
PLAYSDKAPI int Player_Capture2File(int playID, const char *filename, int sync=0/*0:�첽: 1:ͬ��*/);

// ��ͼ���ڴ�
PLAYSDKAPI int Player_Capture2Memory(int playID, unsigned char *bmpData);

// ��ȡ��Ƶ��Ϣ
PLAYSDKAPI int Player_GetVideoInfo(int playID, int *codec, int *width, int *height);

//�����ڲ��Ŷ�ý���ļ�
PLAYSDKAPI int Player_FileSeek(int playID, unsigned int _time_secs);

//====================================================
//======================¼��==========================
// ����¼�����(¼��ģʽ�л�Ҳ���øú���)
PLAYSDKAPI int Player_SetRecordingParams(int playID, const REC_PARAM *param);
// ��ȡ¼�����
PLAYSDKAPI REC_PARAM *Player_GetRecordingParams(int playID);
// ��ȡ¼��״̬
PLAYSDKAPI REC_STATUS_ENUM Player_GetRecordingStatus(int playID);

//����¼��(ֱ��ָ���ļ���,�ͼ��д洢�޹�)
PLAYSDKAPI int Player_RecordingToLocalFile(int playID, const char *filename);
PLAYSDKAPI int Player_StopRecordingToLocalFile(int playID);


//����ָ��������,��ȡ������¼�����ݵ�����(int: 4x8=32,ÿ�����31��,����һ��unsigned int����)
PLAYSDKAPI	int	Player_GetRecDate(unsigned int _year, unsigned int _month, unsigned int *_date);
//�������ڻ�ȡ����������¼�����ݵ��豸USN
PLAYSDKAPI	int	Player_GetDeviceListByDate(REC_DATE_T *pRecDate, unsigned int usn_num, char **pUSN);
//�������ں��豸USN,��¼������, ��ȡһ��24Сʱ������¼��ʱ��(��ȷ������) _time[1440]:ÿ���ֽڱ�ʾ��ǰÿ���ӵ�¼������
//PLAYSDKAPI	int	Player_GetRecTimeByUSN(REC_DATE_T *pRecDate, char *pUSN, unsigned short _recMode, UINT64 _time[48]);
PLAYSDKAPI	int	Player_GetRecTimeByUSN(REC_DATE_T *pRecDate, char *pUSN, unsigned char _time[1440]);


//��ʼ������
PLAYSDKAPI int Player_SetStorageStrategy(REC_STRATEGY_ENUM	_strategy);		//���ô洢����(������ʷ�ļ�  or  ֹͣ¼��)
PLAYSDKAPI int Player_SetRecordingStrategy(unsigned int _strategy);			//����¼���߼����� (¼���߼����ⲿ����:0x00 or �ڲ�����:0x01)
PLAYSDKAPI int Player_InitialDisk(char *pDisk, unsigned int reserveSize=1024);	//reserveSize:������С Ĭ��1G
PLAYSDKAPI int Player_GetDiskList(char disk[16]);		//��ȡ�����õĴ����б�
PLAYSDKAPI int Player_GetDiskStatus(char disk);		//��ȡ����״̬  -1:δ��ʼ��		0:�ѳ�ʼ��
PLAYSDKAPI int Player_AddDisk(char disk, unsigned int reserveSize=2048);			//��Ӵ���	reserveSize:������С	Ĭ��2G(д������ռ��)
PLAYSDKAPI int Player_DelDisk(char disk);			//ɾ������




//��ק����
PLAYSDKAPI int Player_SetDragStartPoint(int playID, float fX, float fY);		//������ק���
PLAYSDKAPI int Player_SetDragEndPoint(int playID, float fX, float fY);		//������ק�յ�
PLAYSDKAPI int Player_SetZoomIn(int playID, int zoomIn);			//�Ƿ�Ŵ���ʾ
PLAYSDKAPI int Player_ResetDragPoint(int playID);					//�����ק�����յ�
//������ʾ����
PLAYSDKAPI int Player_SetRenderRect(int playID, LPRECT lpRectSrc);




//2015.09.30
PLAYSDKAPI int Player_PauseStream(int playID);		//��ͣ����
PLAYSDKAPI int Player_ResumeStream(int playID);		//��������
PLAYSDKAPI int Player_FastPlay(int playID, float fScale);	//�������ʲ���	1/16 1/8 1/4 1/2 1 2 4 8 16
PLAYSDKAPI int Player_Seek(int playID, double starttime, double endtime, float fScale);		//���㲥��, ���ݻ�ȡ�����ļ���ʱ��,������ʼ�ͽ�������ʱ�� �� ����
PLAYSDKAPI int Player_GetStreamTime(int playID, double *currplaytime, double *totaltime);	//��ȡ��ǰ����ʱ�����ʱ��
PLAYSDKAPI int Player_StreamIsDisconnect(int playID);		//��ȡ����״̬

#ifdef _DEBUG
PLAYSDKAPI int Player_Debug_Channel(int channelId);
#endif

#ifdef __cplusplus
}
#endif

#endif
