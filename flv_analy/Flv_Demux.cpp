#include "Flv_Demux.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "Judge_Type.h"


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
		for(int i=0;i<m_ContentLength;i++)
		{
			if(m_Content[i])
			{
				free(m_Content[i]);
				m_Content[i]=NULL;
			}
		}
		free(m_Content);
		m_Content=NULL;
	}
}

int FLV_Demux::setinit_params()
{
	m_FileName=NULL;
	m_Content=NULL;
	m_ContentLength=16;
	m_ContentSize=1024*1024;
	m_BFirst=true;
	b_Rec=false;
	b_Dec=false;
	m_writenum=0;
	m_readnum=0;
	m_writepos=0;
	m_readpos=0;
	b_stop = false;
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
	m_Content = (unsigned char**) calloc(m_ContentLength,sizeof(unsigned char*));
	if(!m_Content)
	{
		printf("FLV_Demux::init calloc failed\n");
		ret = -1;
		goto Init_End;
	}

	for(int i=0;i<m_ContentLength;i++)
	{
		m_Content[i] = (unsigned char*)calloc(m_ContentSize,sizeof(unsigned char));
		if(!m_Content[i])
		{
			printf("FLV_Demux::init calloc m_content failed\n");
			ret = -1;
			goto Init_End;
		}
	}


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

			if(m_writenum > m_readnum +m_ContentLength-1)
			{
				Sleep(100);
				printf("start_recieve %d %d %d\n"
						,m_writenum,m_readnum,m_ContentLength-1);
				continue;
			}
			
			int readsize = fread(m_Content[m_writepos],sizeof(unsigned char),m_ContentSize
				,fp);

			if(m_BFirst)
			{
				if(readsize > 3)
				{
					if(CheckFLV(m_Content[m_writepos]) != 0)
					{
						log_to_file(FLOG_NORMAL,"FLV_Demux::start_analy %s is not flv file",m_FileName);
						goto START_ANALY_END;
					}

					//add analy header

					m_BFirst = false;

					log_to_file(FLOG_NORMAL,"FLV_Demux::start_analy %s is flv file",m_FileName);
					printf("FLV_Demux::start_analy %s is flv file\n",m_FileName);

				}
				else
				{
					//log_to_file(FLOG_NORMAL,"");
					printf("FLV_Demux::start_analy readsize(%d) <3\n",readsize);
					goto START_ANALY_END;
				}
			}

			m_writepos = (m_writepos+1)%m_ContentLength;
			m_writenum++;
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


int FLV_Demux::analy_flv()
{

	return 0;
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

	while(true)
	{
		if(b_stop)
		{
			break;
		}
		//no data
		if(m_readnum <= m_writenum)
		{
			Sleep(3);
			continue;
		}

		//analy tag

	}


	printf("decoder data end\n");
	b_Dec=false;
	return 0;
}