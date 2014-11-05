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
		���ض�ȡ�����ֽ�����
			0		û�ж�ȡ����
			-1		���ļ�ʧ��
			>0		����ʵ�ʶ�ȡ�����ֽ�����
	*/
	int rec_data(int src_len,unsigned char* src_content,bool* over_flag);

private:
	char*	m_SrcAddr;
	int		m_Mode;
	FILE*	m_Fp;
};

#endif