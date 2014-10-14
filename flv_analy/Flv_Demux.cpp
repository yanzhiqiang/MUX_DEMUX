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
#define SUM_NUM		6


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

	if(m_VideoQueue)
	{
		delete m_VideoQueue;
		m_VideoQueue = NULL;
	}

	if(m_AudioQueue)
	{
		delete m_AudioQueue;
		m_AudioQueue = NULL;
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
	m_TagDataLength=0;
	
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
		
		
		int readsize = 0;
		int readpos=0;
		while(!feof(fp))
		{
			if(b_stop)
			{
				goto START_RECIEVE_END;
			}
			
			int analypos = 0;

			readsize = readpos + fread(m_Content+readpos,sizeof(unsigned char),READ_BUFFERSIZE-readpos
						,fp);

			if(m_BFirst)
			{
				if(readsize <3)
				{
					printf("FLV_Demux::start_recieve readsize(%d) < 3\n",readsize);
					continue;
				}
				if(CheckFLV(m_Content) != 0)
				{
					log_to_file(FLOG_NORMAL,"FLV_Demux::start_recieve %s is not flv file",m_FileName);
					goto START_RECIEVE_END;
				}

					
				m_BFirst = false;

				log_to_file(FLOG_NORMAL,"FLV_Demux::start_recieve %s is flv file",m_FileName);
				printf("FLV_Demux::start_recieve %s is flv file\n",m_FileName);

				//add analy header
				if(readsize <9)
				{
					printf("FLV_Demux::start_recieve readsize(%d) < 9,can't analy flvheander\n",readsize);
					continue;
				}
				analypos += analy_flvhead(m_Content);
				
			}

			if(readsize < analypos + 5)
			{
				printf("FLV_Demux::start_recieve readsize(%d) <analypos(%d) + 5,can't analy flvtag\n"
					,readsize,analypos);
					
				continue;
			}
			
			//pre tag length
			while(analypos < readsize)
			{
				if(b_stop)
				{
					goto START_RECIEVE_END;
					
				}
				printf("pre tag length is %u\n"
						,Get_Int(m_Content+analypos,4));
				analypos+=4;
				
				if(analypos >= readsize)
				{
					printf("analy normal,%d %d\n"
						,analypos,readsize);
					break;
				}
				if(*(m_Content+analypos) == 0x12)
				{
					//analy script
					int ret = analy_scripttag(m_Content+analypos,readsize-analypos);
					if(ret > 0)
					{
						analypos +=ret;
						
					}
					else
					{
						break;
					}
				}
				else if(*(m_Content+analypos) == 0x08)
				{
					//audio
					int ret = analy_audiotag(m_Content+analypos,readsize-analypos);
					if(ret > 0)
					{
						analypos+=ret;
						
					}
					else
					{
						break;
					}
				}
				else if(*(m_Content+analypos) == 0x09)
				{
					//video
					int ret = analy_videotag(m_Content+analypos,readsize-analypos);
					if(ret > 0)
					{
						analypos+=ret;
						
					}
					else
					{
						break;
					}
				}
				else
				{
					printf("no such tag,%0x\n",*(m_Content+analypos));
					ret = -1;
					goto START_RECIEVE_END;
				}
			}
			
			analypos-=4;	//analypos break出来的时候要还原。

			printf("analypos=%d,readsize=%d\n"
					,analypos,readsize);
			
			//copy
			if(analypos < readsize)
			{
				//同一片内存区域，所以尽量自己保证拷贝正确。
				for(int i=0;i<readsize-analypos;i++)
				{
					m_Content[i]=m_Content[analypos+i];
				}
				readpos = readsize - analypos;
			}
			else if(analypos == readsize)
			{
				readpos = 0;
			}
			else
			{
				printf("read over,%d > %d\n",analypos,readsize);
			}
			
			if(readpos > READ_BUFFERSIZE)
			{
				printf("single tag length > %d,it should be created more\n",READ_BUFFERSIZE);
				ret=-1;
				goto START_RECIEVE_END;
			}

		}
		
	}
	else
	{
		printf("open %s failed",m_FileName);
	}

START_RECIEVE_END:

	if(fp)
	{
		fclose(fp);
		fp=NULL;
	}
	return ret;
}

int FLV_Demux::analy_videotag(unsigned char* src,int src_size)
{
	int size = 0;

	int ret = analy_taghead(src+size,src_size);
	if(ret <= 0)
	{
		return 0;
	}
	size += ret;
	src_size -= size;

	if(src_size <m_TagDataLength)
	{
		printf("video tag length(%d) < datalength(%u)\n"
			,src_size,m_TagDataLength);
		return -1;
	}

	analy_videoinfo(src+size);

	size += m_TagDataLength;

	return size;
}

int FLV_Demux::analy_videoinfo(unsigned char* src)
{
	char log_info[1024]={0};
	int size = 0;

	unsigned int tmp = Get_Bits(src,0xF0);
	printf("video key frame is %d\n",tmp/16);
	
	switch(tmp/16)
	{
	case 1:
		sprintf(log_info+strlen(log_info),"%s","key frame : ");
		break;
	case 2:
		sprintf(log_info+strlen(log_info),"%s","inter frame : ");
		break;
	case 3:
		sprintf(log_info+strlen(log_info),"%s","disposable frame : ");
		break;
	case 4:
		sprintf(log_info+strlen(log_info),"%s","generated key frame : ");
		break;
	case 5:
		sprintf(log_info+strlen(log_info),"%s","video inf/command frame : ");
		break;

	default:
		log_to_file(FLOG_NORMAL,"video class is not defined %d",tmp/16);
		return -1;
	}
	//log_to_file(FLOG_NORMAL,"video key frame is %d",tmp/16);


	unsigned int code_id =  Get_Bits(src,0x0F);
	printf("video encode id is %d\n",code_id);

	switch(code_id)
	{
	
	case 2:
		sprintf(log_info+strlen(log_info),"%s"," Sorenson H.263 : ");
		break;
	case 3:
		sprintf(log_info+strlen(log_info),"%s"," Screen video : ");
		break;
	case 4:
		sprintf(log_info+strlen(log_info),"%s"," On2 VP6 : ");
		break;
	case 5:
		sprintf(log_info+strlen(log_info),"%s"," On2 VP6 with alpha channel : ");
		break;
	case 6:
		sprintf(log_info+strlen(log_info),"%s"," Screen video version 2 ");
		break;
	case 7:
		sprintf(log_info+strlen(log_info),"%s"," AVC : ");
		break;

	default:
		log_to_file(FLOG_NORMAL,"video code_id is not defined %d",code_id);
		return -1;
	}

	if(code_id == 7)
	{
		//AVC
		size ++;
		unsigned int AVC_PacketType = Get_Int(src+size,1);
		switch(AVC_PacketType)
		{
		case 0:
			sprintf(log_info+strlen(log_info),"%s"," AVC sequence header : ");
			break;
		case 1:
			sprintf(log_info+strlen(log_info),"%s"," AVC NALU : ");
			break;
		case 2:
			sprintf(log_info+strlen(log_info),"%s"," AVC end of sequence : ");
			break;
		default:
			log_to_file(FLOG_ERR,"AVC_PacketType(%d) isnot defined ",AVC_PacketType);
			return -1;
		}
		size++;
		//time offset

	}
	log_to_file(FLOG_NORMAL,"%s",log_info);
	return 0;
}

int FLV_Demux::analy_audiotag(unsigned char* src,int src_size)
{
	int size  = 0;

	int ret = analy_taghead(src+size,src_size); 
	if(ret <= 0)
	{
		return 0;
	}
	size += ret;
	src_size -= size;

	if(src_size <m_TagDataLength)
	{
		printf("audio tag length(%d) < datalength(%u)\n"
			,src_size,m_TagDataLength);
		return -1;
	}
	
	analy_audioinfo(src+size);

	size += m_TagDataLength;

	return size;
}

int FLV_Demux::analy_audioinfo(unsigned char* src)
{
	unsigned int tmp = Get_Bits(src,0xF0);
	printf("audio format is %u\n",tmp/16);

	tmp = Get_Bits(src,0x0C);
	printf("audio samplerate is %u\n",tmp/4);

	tmp = Get_Bits(src,0x02);
	printf("audio bits is %u\n",tmp/2);

	tmp =  Get_Bits(src,0x01);
	printf("audio class is %u\n",tmp);

	return 0;
}

int FLV_Demux::analy_scripttag(unsigned char* src,int src_size)
{
	int size = 0;

	

	int ret = analy_taghead(src+size,src_size); 
	if(ret <= 0)
	{
		return 0;
	}
	size += ret;
	src_size -= size;

	if(src_size <m_TagDataLength)
	{
		printf("tag length(%d) < datalength(%u)\n"
			,src_size,m_TagDataLength);
		return -1;
	}

	if(m_TagDataLength != analy_scriptdata(src+size,src_size))
	{
		printf("script data len != (%d)\n"
		,m_TagDataLength);
		return -1;
	}
	
	size+=m_TagDataLength;
	//m_TagDataLength != ret
	
	return size;
}

int FLV_Demux::analy_scriptdata(unsigned char* src,int src_size)
{
	//第一个字节是固定的。
	int size = 0;
	if(src[0]!=0x02)
	{
		printf("script tag 1 head(%x) is not 0x02\n"
				,src[0]);
		return -1;
	}
	size++;
	
	unsigned int tmp = Get_Int(src+size,2);
	if(tmp!=10)
	{
		printf("script tag 1 head length(%x) is not 0x0A\n"
				,*(src+size));
		return -2;
	}
	size+=2;

	//开始固定的头部分析
	size+=10;	//跳过固定的头 onMetaData 这是给flashvideo 调用api

	//
	if((*(src+size)) != 0x08)
	{
		printf("not second tag\n");
	}
	size+=1;
	
	unsigned int array_size = Get_Int(src+size,4);
	printf("second tag array length is %u\n"
			,array_size);
	size+=4;

	//analy array
	for(int i=0;i<array_size;i++)
	{
		unsigned int name_size = Get_Int(src+size,2);
		size+=2;

		char tmp_name[1024]={0};
		memcpy(tmp_name,src+size,name_size);
		size+=name_size;
		//printf("\n \t name is %s ",tmp_name);

		int tmp_size = 0;
		switch((*(src+size)))
		{
		case 0:
			tmp_size = 8;break;
		case 1:
			tmp_size = 1;break;
		case 2:
			tmp_size = -1;break;
		case 3:
			tmp_size = 2; break;
		case 'C':
			tmp_size = -2;break;
		default:
			printf("tag class is %d\n",(*(src+size)));
			break;
		}
		int tmp_class=(*(src+size));
		size++;

		if(tmp_size == 0)
		{
			printf("tag size is %d\n",tmp_size);
			//return -1;
			continue;
		}

		if(tmp_size > 0)
		{
			double tmp_value = 0.0;
			if(tmp_class == 0)
			{
				tmp_value = char2double(src+size,tmp_size);
			}
			else
			{
				tmp_value = Get_Int(src+size,tmp_size);
			}
			printf("%s : %lf\n",tmp_name,tmp_value);
			size+=tmp_size;
		}
		else if(tmp_size == -1)
		{
			unsigned int t_size = Get_Int(src+size,2);
			size+=2;

			char tmp_value[1024]={0};
			memcpy(tmp_value,src+size,t_size);
			printf("%s : %s\n",tmp_name,tmp_value);
			size+=t_size;
		}
		else if(tmp_size == -2)
		{
			int t_size = Get_Int(src+size,4);
			size+=4;

			char tmp_value[1024]={0};
			memcpy(tmp_value,src+size,t_size);
			printf("%s : %s\n",tmp_name,tmp_value);
			size+=tmp_size;
		}
	}

	size+=3;	//00 00 09 array final tag

	return size;
}


int FLV_Demux::analy_taghead(unsigned char* src,int src_size)
{
	int size = 0;
	unsigned int tmp=0;
	
	if(src_size <11)
	{
		return -1;
	}

	char tag_name[20]={0};
	if(*(src+size) == 0x12)
	{
		sprintf(tag_name,"%s","script");
	}
	else if(*(src+size) == 0x08)
	{
		sprintf(tag_name,"%s","audio");
	}
	else if(*(src+size) == 0x09)
	{
		sprintf(tag_name,"%s","video");
	}
	size++;	//0x12,0x08,0x09的标记

	m_TagDataLength = Get_Int(src+size,3);
	printf("%s header length is %u\n"
			,tag_name,m_TagDataLength);
	
	size+=3;

	unsigned int time_stamp = Get_Int_Reverse(src+size,4);
	printf("%s timestamp is %u\n",tag_name,time_stamp);
	size+=4;

	unsigned int tmp_id =  Get_Int(src+size,3);
	printf("%s stream id is %u\n",tag_name,tmp_id);
	
	size+=3;


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