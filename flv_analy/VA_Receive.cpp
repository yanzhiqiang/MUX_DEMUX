#include "VA_Receive.h"
#include "Configure_Flv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

VA_Receive::VA_Receive()
{
	m_SrcAddr = NULL;
	m_Mode = OFFLINE_MODE;
	m_Fp=NULL;
}

VA_Receive::~VA_Receive()
{
	if(m_SrcAddr)
	{
		free(m_SrcAddr);
		m_SrcAddr=NULL;
	}
	if(m_Fp)
	{
		fclose(m_Fp);
		m_Fp=NULL;
	}
}

int VA_Receive::set_init(const char* src_addr,int mode)
{
	m_Mode=mode;
	m_SrcAddr = (char*)calloc(sizeof(char),strlen(src_addr)+1);
	memcpy(m_SrcAddr,src_addr,strlen(src_addr));
	return 0;
}

int VA_Receive::rec_data(int src_len,unsigned char* src_content,bool* over_flag)
{
	//check 
	int ret_size = 0;
	if(m_Mode == OFFLINE_MODE )
	{
		if(m_Fp == NULL)
		{
			//图像处理需要用rb。
			m_Fp = fopen(m_SrcAddr,"rb");
			if(!m_Fp)
			{
				printf("%s %s %d open file(%s) failed,[%s]",
					__FILE__,__FUNCTION__,__LINE__,m_SrcAddr,strerror(errno));
				return -1;
			}
		}
		
		ret_size = fread(src_content,sizeof(unsigned char),src_len,m_Fp);
			
		//printf("src:[%d],dst:[%d]\n",src_len,ret_size);
		
		if(feof(m_Fp))
		{
			*over_flag = true;
		}
	}

	return ret_size;
}
