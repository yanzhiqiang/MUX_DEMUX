#include "Flv_Demux.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "Judge_Type.h"
#include "Simple_Queue.h"
#include "Get_DTool.h"

#define VIDEO_NUM	25*2
#define AUDIO_NUM	50*2
#define READ_BUFFERSIZE	1024*1024


DWORD	WINAPI	Rec_Func(LPVOID	lpParam)
{
	FLV_Demux* t_p = (FLV_Demux*)lpParam;
	if(t_p)
	{
		t_p->receive_data();
	}
	return 0;
}

DWORD	WINAPI	Dec_Func(LPVOID	lpParam)
{
	FLV_Demux* t_p = (FLV_Demux*)lpParam;
	if(t_p)
	{
		t_p->decoder_data();
	}
	return 0;
}


FLV_Demux::FLV_Demux()
{
	setinit_params();
}


FLV_Demux::~FLV_Demux()
{
	b_stop = true;
	if(b_Dec || b_Rec)
	{
		Sleep(1000);
	}

	if(b_Dec)
	{
		printf("Dec Thread is not over,so killed\n");
		TerminateThread(Dec_Thread,0);
	}
	else
	{
		CloseHandle(Dec_Thread); 
	}
	Dec_Thread=NULL;

	if(b_Rec)
	{
		printf("Rec Thread is not over,so killed\n");
		TerminateThread(Rec_Thread,0);
	}
	else
	{
		CloseHandle(Rec_Thread); 
	}
	Rec_Thread=NULL;

	if(m_FileName)
	{
		free(m_FileName);
		m_FileName = NULL;
	}
	if(m_Content)
	{
		free(m_Content);
		m_Content=NULL;
	}
}

int FLV_Demux::setinit_params()
{
	m_FileName=NULL;
	m_Content=NULL;
	
	m_BFirst=true;
	b_Rec=false;
	b_Dec=false;
	b_stop = false;
	m_VideoQueue = NULL;
	m_AudioQueue = NULL;
	return 0;
}

int FLV_Demux::init(const char* filename)
{
	int ret = 0;
	if(filename == NULL)
	{
		log_to_file(FLOG_ERR,"FLV_Demux::file name is NULL");
		ret = -1;
		goto Init_End;
	}

	setinit_params();

	//set file name
	m_FileName = (char*)calloc(strlen(filename)+1,sizeof(char));
	memcpy(m_FileName,filename,strlen(filename));

	
	//alloca m_Content;
	m_Content = (unsigned char*) calloc(READ_BUFFERSIZE,sizeof(unsigned char));
	if(!m_Content)
	{
		printf("FLV_Demux::init calloc failed\n");
		ret = -1;
		goto Init_End;
	}
	
	m_VideoQueue = new Simple_Queue();
	m_VideoQueue->init(VIDEO_NUM,sizeof(m_VideoFrame));
	m_AudioQueue = new Simple_Queue();
	m_AudioQueue->init(AUDIO_NUM,sizeof(m_AudioFrame));

	log_to_file(FLOG_NORMAL,"FLV_Demux::file name is %s",m_FileName);
	printf("FLV_Demux::file name is %s\n",m_FileName);

	//start two thread ,one for receive,one for decoder
	Rec_Thread = CreateThread(NULL,0,Rec_Func,(LPVOID)this,0,NULL); 

	Dec_Thread = CreateThread(NULL,0,Dec_Func,(LPVOID)this,0,NULL); 

	for(int i=0;i<3;i++)
	{
		if(b_Dec && b_Rec)
		{
			goto Init_End;
		}
		else
		{
			Sleep(1000);
		}
	}

Init_End:
	printf("init end return %d\n",ret);
	return ret;
}

int FLV_Demux::start_recieve()
{
	int ret = 0;

	//open file
	FILE* fp =  NULL;
	fp = fopen(m_FileName,"rb");
	if(fp)
	{	
		printf("start_recieve fopen %s success\n"
						,m_FileName);
				
		while(!feof(fp))
		{
			if(b_stop)
			{
				break;
			}
			int analypos = 0;
			int readsize = fread(m_Content,sizeof(unsigned char),READ_BUFFERSIZE
				,fp);

			if(m_BFirst)
			{
				if(readsize > 3)
				{
					if(CheckFLV(m_Content) != 0)
					{
						log_to_file(FLOG_NORMAL,"FLV_Demux::start_analy %s is not flv file",m_FileName);
						goto START_ANALY_END;
					}

					
					m_BFirst = false;

					log_to_file(FLOG_NORMAL,"FLV_Demux::start_analy %s is flv file",m_FileName);
					printf("FLV_Demux::start_analy %s is flv file\n",m_FileName);

					//add analy header
					analypos += analy_flvhead(m_Content);


				}
				else
				{
					//log_to_file(FLOG_NORMAL,"");
					printf("FLV_Demux::start_analy readsize(%d) <3\n",readsize);
					goto START_ANALY_END;
				}
			}

			if(*(m_Content+analypos+4) == 0x12)
			{
				//analy script

			}
			else if(*(m_Content+analypos+4) == 0x08)
			{
				//audio

			}
			else if(*(m_Content+analypos+4) == 0x09)
			{
				//video
			}
			else
			{
				printf("no such tag,%0x",*(m_Content+analypos+4));
			}
			
		}
		//printf("start_recieve read %s over\n"
			//			,m_FileName);
			
	}

START_ANALY_END:

	if(fp)
	{
		fclose(fp);
		fp=NULL;
	}
	return ret;
}

int FLV_Demux::analy_scripttag(unsigned char* src)
{
	int size = 0;
	return size;
}

int FLV_Demux::analy_flv()
{

	return 0;
}


int FLV_Demux::analy_flvhead(unsigned char* src)
{
	int size = 0;
	size += 3;

	printf("version is %d\n",*(src+size));	//版本信息
	size ++;

	unsigned int tmp = (unsigned int) (*(src+size));	//音视频标记
	size++;

	if(tmp &0x04)
	{
		printf("have audio\n");
	}
	if(tmp & 0x01)
	{
		printf("have video\n");
	}

	//获取数据的偏移量值，这个值总为9。从File Header开始到File Body开始的字节数
	tmp=Get_Int(src+size,4);
	if(tmp != 9)
	{
		printf("header contain more ,should look at header(%u)\n"
			,tmp);
		return -2;
	}
	size+=4;

	return size;
}

int FLV_Demux::receive_data()
{
	printf("receive data start\n");
	b_Rec=true;

	Sleep(3000);
	start_recieve();

	b_Rec=false;
	printf("receive data end\n");
	return 0;
}

int FLV_Demux::decoder_data()
{
	b_Dec=true;
	printf("decoder data start\n");

	Sleep(3000);

	//处理线程，视频优先处理，没有视频处理音频
	while(true)
	{
		if(b_stop)
		{
			break;
		}
		
		//analy tag

	}


	printf("decoder data end\n");
	b_Dec=false;
	return 0;
}