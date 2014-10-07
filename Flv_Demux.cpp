#include "Flv_Demux.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "Judge_Type.h"

FLV_Demux::FLV_Demux()
{
	setinit_params();
}


FLV_Demux::~FLV_Demux()
{
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
	m_ContentLength=2;
	m_ContentSize=1024;
	m_BFirst=true;
	return 0;
}

int FLV_Demux::init(const char* filename)
{
	int ret = 0;
	if(filename == NULL)
	{
		log_to_file(FLOG_ERR,"FLV_Demux::file name is NULL");
		return -1;
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
		return -1;
	}

	for(int i=0;i<m_ContentLength;i++)
	{
		m_Content[i] = (unsigned char*)calloc(m_ContentSize,sizeof(unsigned char));
		if(!m_Content[i])
		{
			printf("FLV_Demux::init calloc m_content failed\n");
			return -1;
		}
	}


	log_to_file(FLOG_NORMAL,"FLV_Demux::file name is %s",m_FileName);
	printf("FLV_Demux::file name is %s\n",m_FileName);

	ret =  start_analy();
	return ret;
}

int FLV_Demux::start_analy()
{
	int ret = 0;
	int index_content=0;
	
	//open file
	FILE* fp =  NULL;
	fp = fopen(m_FileName,"rb");
	if(fp)
	{	
		while(!feof(fp))
		{
			
			int readsize = fread(m_Content[index_content],sizeof(unsigned char),m_ContentSize
				,fp);

			if(m_BFirst)
			{
				if(readsize > 3)
				{
					if(CheckFLV(m_Content[index_content]) != 0)
					{
						log_to_file(FLOG_NORMAL,"FLV_Demux::start_analy %s is not flv file",m_FileName);
						goto START_ANALY_END;
					}
					m_BFirst = false;

					log_to_file(FLOG_NORMAL,"FLV_Demux::start_analy %s is flv file",m_FileName);
					printf("FLV_Demux::start_analy %s is flv file",m_FileName);

				}
				else
				{
					//log_to_file(FLOG_NORMAL,"");
					printf("FLV_Demux::start_analy readsize(%d) <3",readsize);
					goto START_ANALY_END;
				}
			}
			
			//analy
			

			index_content = (index_content+1)%m_ContentLength;
		}
	}

START_ANALY_END:

	if(fp)
	{
		fclose(fp);
		fp=NULL;
	}
	return ret;
}
