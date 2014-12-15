#ifndef __VA_RECEIVE_H_
#define __VA_RECEIVE_H_

#include <stdio.h>

class VA_Receive
{
public:
	VA_Receive();
	~VA_Receive();

	int set_init(const char* src_addr,int mode);
	/*
		返回读取到的字节数，
			0		没有读取到。
			-1		打开文件失败
			>0		正常实际读取到的字节数。
	*/
	int rec_data(int src_len,unsigned char* src_content,bool* over_flag);


	int	get_httpstatus();

	int get_socketret(int socket,char* send_buffer,int send_len,char* recv_buf,int recv_len);
private:
	int	init_socket();
	char*	m_SrcAddr;
	int		m_Mode;
	FILE*	m_Fp;
	int		m_iSocket;
};

#endif