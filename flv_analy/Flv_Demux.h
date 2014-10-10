#ifndef __FLV_DEMUX_H
#define __FLV_DEMUX_H

#include <Windows.h>

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
	unsigned char** m_Content;
	int	m_ContentLength;
	int m_ContentSize;
	bool m_BFirst;
	int setinit_params();
	int start_recieve();
	int analy_flv();

	//thread variable
	HANDLE Rec_Thread;
	HANDLE Dec_Thread;
	bool	b_Rec;
	bool	b_Dec;
	int		m_writenum;
	int		m_readnum;
	int		m_writepos;
	int		m_readpos;
	// all stop 
	bool	b_stop;
};

#endif