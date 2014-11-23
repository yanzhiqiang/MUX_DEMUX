#ifndef __FLV_DEMUX_H
#define __FLV_DEMUX_H

#include <Windows.h>
#include "Get_DTool.h"

class Simple_Queue;
class VA_Receive;
class Safe_Memory;

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
	
	/*
		�����Ƿ�������
		1	�������
		0	δ���
	*/
	int check_recover();

	/*
		��ȡ�ű�����Ƶ����Ƶ
	*/
	void*	pop_videoitem();
	int		get_scriptitem(void* dst,int* dst_len);
	void*	pop_audioitem();
	int		get_videocount();
	int		get_audiocount();
	int		get_flvhead(void* dst,int* dst_len);
	int		analy_taghead(unsigned char* src,int size,unsigned int* timestamp=NULL);

	bool	judge_video();
	bool	judge_audio();

	/*
		��tag�з���ʱ���
	*/
	unsigned int	get_tagtimestamp(unsigned char* src);
private:
	char* m_FileName;
	unsigned char* m_Content;
	bool	m_BFirst;
	bool	b_Rec;
	bool	b_Video;
	bool	b_Audio;

	//private 
	int setinit_params();
	int start_recieve();

	int analy_flvhead(unsigned char* src);
	
	int analy_scripttag(unsigned char* src,int size);
	
	int analy_scriptdata(unsigned char* src,int size);
	
	int analy_audiotag(unsigned char* src,int size);
	int analy_audioinfo(unsigned char* src);

	int analy_videotag(unsigned char* src,int size);
	int analy_videoinfo(unsigned char* src);

	//thread variable
	HANDLE Rec_Thread;
	int		m_Rec;
	

	// all stop 
	bool	b_stop;
	//��Ƶ����Ƶ����
	Simple_Queue* m_VideoQueue;
	Simple_Queue* m_AudioQueue;
	
	struct Video_Frame m_VideoFrame;
	struct Audio_Frame m_AudioFrame;

	int m_TagDataLength;

	VA_Receive* m_Reciever;

	Safe_Memory* m_ScriptContent;
	int		m_scriptnum;

	Safe_Memory* m_FlvheadContent;
};

#endif