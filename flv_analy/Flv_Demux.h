#ifndef __FLV_DEMUX_H
#define __FLV_DEMUX_H

#include <Windows.h>
#include "Get_DTool.h"

class Simple_Queue;

class FLV_Demux
{
public:
	FLV_Demux();
	~FLV_Demux();

	/*
	param: 
		filename	//filename shoule be a file name,not url 
	return 
		0	ok
		<0  error
	*/
	int init(const char* filename);
	int receive_data();
	int decoder_data();
private:
	char* m_FileName;
	unsigned char* m_Content;
	bool m_BFirst;

	//private 
	int setinit_params();
	int start_recieve();
	int analy_flv();
	int analy_flvhead(unsigned char* src);
	int analy_scripttag(unsigned char* src);

	//thread variable
	HANDLE Rec_Thread;
	HANDLE Dec_Thread;
	bool	b_Rec;
	bool	b_Dec;

	// all stop 
	bool	b_stop;
	//视频和音频队列
	Simple_Queue* m_VideoQueue;
	Simple_Queue* m_AudioQueue;
	
	struct Video_Frame m_VideoFrame;
	struct Audio_Frame m_AudioFrame;
};

#endif