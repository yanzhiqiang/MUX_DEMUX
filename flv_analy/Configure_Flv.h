#ifndef __CONFIGURE_FLV_H_
#define __CONFIGURE_FLV_H_


//定义接收模式是在线还是离线。
#define LIVE_PRE		"http://"
#define HTTP_STATUSEND	"\r\n"
#define HTTP_HEADEND	"\r\n\r\n"
#define HTTP_STATUSOK	200

#define LIVE_MODE		0
#define OFFLINE_MODE	1

//flv_demux define变量
#define VIDEO_NUM	25
#define AUDIO_NUM	50
#define READ_BUFFERSIZE	1024*1024
#define SUM_NUM		6

#define LINE_SIZE	1024

#define STATE_STOP 0
#define STATE_RUNNING 1
#define STATE_OVER 2

#define TIMESTAMP_MAX 0xFFFFFFFF
#define TIMESTAMP_ROLLBACK	30*60*1000

#define INVALID_SOCKET	0xFFFFFFFF

#define	TIMEOUT_SOCKET	15000

#define	ERROR_IGNORECOUNT	3

#endif