#include "Flv_Demux.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "Judge_Type.h"
#include "Simple_Queue.h"
#include "Get_DTool.h"
#include "Configure_flv.h"
#include "VA_Receive.h"
#include "Safe_Memory.h"



DWORD	WINAPI	Rec_Func(LPVOID	lpParam)
{
	FLV_Demux* t_p = (FLV_Demux*)lpParam;
	if(t_p)
	{
		t_p->receive_data();
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
	if(m_Rec == STATE_RUNNING)
	{
		Sleep(1000);
	}

	
	if(m_Rec == STATE_RUNNING)
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

	if(m_Reciever)
	{
		delete m_Reciever;
		m_Reciever=NULL;
	}
	if(m_ScriptContent)
	{
		delete m_ScriptContent;
		m_ScriptContent=NULL;
	}
}

int FLV_Demux::setinit_params()
{
	m_FileName=NULL;
	m_Content=NULL;
	
	m_BFirst=true;
	m_Rec=STATE_STOP;
	b_stop = false;
	m_VideoQueue = NULL;
	m_AudioQueue = NULL;
	m_TagDataLength=0;
	m_Reciever=NULL;
	m_ScriptContent=NULL;
	m_scriptnum=0;
	m_FlvheadContent=NULL;

	return 0;
}

int FLV_Demux::init(const char* filename)
{
	int ret = 0;
	int i=0;
	if(filename == NULL)
	{
		log_to_file(FLOG_ERR,"FLV_Demux::file name(%s) is NULL",filename);
		ret = -1;
		goto Init_End;
	}

	setinit_params();

	//set file name
	
	if(!m_Reciever)
	{
		m_Reciever = new VA_Receive();
		if(!m_Reciever)
		{
			printf("%s %s %d create reciever failed"
					,__FILE__,__FUNCTION__,__LINE__);
			ret = -1;
			goto Init_End; 
		}
	}
	m_Reciever->set_init(filename,OFFLINE_MODE);
	//alloca m_Content;
	m_Content = (unsigned char*) calloc(READ_BUFFERSIZE,sizeof(unsigned char));
	if(!m_Content)
	{
		printf("FLV_Demux::init calloc failed\n");
		ret = -1;
		goto Init_End;
	}
	
	m_VideoQueue = new Simple_Queue();
	ret = m_VideoQueue->init(VIDEO_NUM,sizeof(struct Video_Frame));
	if(ret != 0)
	{
		printf("FLV_Demux::m_VideoQueue init retutn %d\n",ret);
	}
	m_AudioQueue = new Simple_Queue();
	ret = m_AudioQueue->init(AUDIO_NUM,sizeof(struct Audio_Frame));
	if(ret != 0)
	{
		printf("FLV_Demux::m_VideoQueue init retutn %d\n",ret);
	}
	
	printf("v_size is %d,a_size is %d",sizeof(struct Video_Frame),sizeof(m_AudioFrame));
	m_ScriptContent = new Safe_Memory();
	m_FlvheadContent = new Safe_Memory();

	log_to_file(FLOG_NORMAL,"FLV_Demux::file name is %s",filename);
	printf("FLV_Demux::file name is %s\n",filename);

	//start two thread ,one for receive,one for decoder
	Rec_Thread = CreateThread(NULL,0,Rec_Func,(LPVOID)this,0,NULL); 

	for(i=0;i<3;i++)
	{
		if(m_Rec != STATE_STOP )
		{
			goto Init_End;
		}
		else
		{
			Sleep(1000);
		}
	}

	if(i>=3)
	{
		log_to_file(FLOG_ERR,"have thread not start");
		printf("have thread not start\n");
		ret = -1;
	}

Init_End:
	printf("init end return %d\n",ret);
	return ret;
}

int FLV_Demux::start_recieve()
{
	int ret = 0;
	bool b_overflag = false;
	m_Rec = STATE_RUNNING;
		
	int readsize = 0;
	int readpos=0;
	
	
	while(!b_overflag)
	{
		if(b_stop)
		{
			goto START_RECIEVE_END;
		}
			
		int analypos = 0;

		int reciever_size = 0;
		for(int k=0;k<3;k++)
		{
			reciever_size = m_Reciever->rec_data(READ_BUFFERSIZE-readpos,m_Content+readpos,&b_overflag);
			if(reciever_size > 0)
			{
				break;
			}
		}
		if(reciever_size <= 0)
		{
			printf("%s %s %d read size(%d) <= 0"
					,__FILE__,__FUNCTION__,__LINE__,reciever_size);
			break;
		}
		printf("reciever_size is %d\n",reciever_size);
		readsize = readpos + reciever_size;
			
		log_to_file(FLOG_NORMAL,"read file readpos=%d,readsize=%d analypos=%d rec_size =%d start"
						,readpos,readsize,analypos,reciever_size);
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
			m_FlvheadContent->memcpy_push(m_Content,analypos);
							
		}

		if(readsize < analypos + 5)
		{
			printf("FLV_Demux::start_recieve readsize(%d) <analypos(%d) + 5,can't analy flvtag\n"
				,readsize,analypos);
				
			continue;
		}
		
		analypos+=4;
		printf("analypos is [%d]\n",analypos);
		//pre tag length
		while(analypos < readsize)
		{
			if(b_stop)
			{
				goto START_RECIEVE_END;		
			}
			//printf("pre tag length is %u\n"
			//		,Get_Int(m_Content+analypos,4));
			
			
				
			if(analypos >= readsize)
			{
				printf("analy normal,%d %d\n"
					,analypos,readsize);
				log_to_file(FLOG_NORMAL,"analy normal,%d %d\n"
						,analypos,readsize);
				break;
			}
			
			if(*(m_Content+analypos) == 0x12)
			{
				//analy script
				int ret = analy_scripttag(m_Content+analypos,readsize-analypos);
				if(ret > 0)
				{
					m_scriptnum++;
					if(m_scriptnum <=1)
					{
						//在这里保存脚本数据
						/*int k=0;
						for(k=0;k<3;k++)
						{*/
							if(0 != m_ScriptContent->memcpy_push(m_Content+analypos,ret))
							{
								Sleep(20);
								continue;
							}
						/*	else
							{
								break;
							}
						}
						if(k>=3)
						{
							printf("%s %s %d push script item failed"
									,__FILE__,__FUNCTION__,__LINE__);
						}*/
					}
					analypos +=ret;
					analypos+=4;
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
					//保存音频数据
					m_AudioFrame.size = ret;
					m_AudioFrame.a_Buffer = (unsigned char*)calloc(m_AudioFrame.size,sizeof(unsigned char));
					memcpy(m_AudioFrame.a_Buffer,m_Content+analypos,m_AudioFrame.size);
					//printf("audio size is %d,buffer=%x\n",m_AudioFrame.size,m_AudioFrame.a_Buffer);
					/*int k=0;
					for(k=0;k<3;k++)
					{*/
						if(0 != m_AudioQueue->PushItem((void*)&m_AudioFrame))
						{
							//if(m_mode == OFFLINE_MODE)
							Sleep(20);
							continue;
						}
						/*else
						{
							break;
						}*/
					/*}
					if(k>=3)
					{
						printf("%s %s %d push audio item failed,count:[%d]\n"
								,__FILE__,__FUNCTION__,__LINE__,m_AudioQueue->GetCount());
					}*/
					

					analypos+=ret;	
					analypos+=4;
									
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
					//视频
					m_VideoFrame.size = ret;
					m_VideoFrame.v_Buffer = (unsigned char*)calloc(m_VideoFrame.size,sizeof(unsigned char));
					memcpy(m_VideoFrame.v_Buffer,m_Content+analypos,m_VideoFrame.size);
					/*int k=0;
					for(k=0;k<3;k++)
					{*/
						if(0 != m_VideoQueue->PushItem(&m_VideoFrame))
						{
							Sleep(20);
							continue;
						}
						/*else
						{
							break;
						}*/
					/*}
					if(k>=3)
					{
						printf("%s %s %d push audio item failed,count:[%d]"
								,__FILE__,__FUNCTION__,__LINE__,m_VideoQueue->GetCount());
					}*/

					analypos+=ret;
					analypos+=4;
					
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
			log_to_file(FLOG_ERR,"read over,%d > %d\n",analypos,readsize);
		}
			
		if(readpos > READ_BUFFERSIZE)
		{
			printf("single tag length > %d,it should be created more\n",READ_BUFFERSIZE);
			log_to_file(FLOG_ERR,"single tag length > %d,it should be created more\n",READ_BUFFERSIZE);
			ret=-1;
			goto START_RECIEVE_END;
		}
			log_to_file(FLOG_NORMAL,"read file readpos=%d,readsize=%d analypos=%d end"
					,readpos,readsize,analypos);
	}
		
	
START_RECIEVE_END:
	m_Rec = STATE_OVER;
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
	//printf("video key frame is %d\n",tmp/16);
	
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
	//printf("video encode id is %d\n",code_id);

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
	//printf("audio format is %u\n",tmp/16);

	tmp = Get_Bits(src,0x0C);
	//printf("audio samplerate is %u\n",tmp/4);

	tmp = Get_Bits(src,0x02);
	//printf("audio bits is %u\n",tmp/2);

	tmp =  Get_Bits(src,0x01);
	//printf("audio class is %u\n",tmp);

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
	//printf("second tag array length is %u\n"
	//		,array_size);
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
			log_to_file(FLOG_NORMAL,"%s : %lf\n",tmp_name,tmp_value);
			size+=tmp_size;
		}
		else if(tmp_size == -1)
		{
			unsigned int t_size = Get_Int(src+size,2);
			size+=2;

			char tmp_value[1024]={0};
			memcpy(tmp_value,src+size,t_size);
			printf("%s : %s\n",tmp_name,tmp_value);
			log_to_file(FLOG_NORMAL,"%s : %s\n",tmp_name,tmp_value);
			size+=t_size;
		}
		else if(tmp_size == -2)
		{
			int t_size = Get_Int(src+size,4);
			size+=4;

			char tmp_value[1024]={0};
			memcpy(tmp_value,src+size,t_size);
			printf("%s : %s\n",tmp_name,tmp_value);
			log_to_file(FLOG_NORMAL,"%s : %s\n",tmp_name,tmp_value);
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
	//printf("%s header length is %u\n"
	//		,tag_name,m_TagDataLength);
	
	log_to_file(FLOG_DEBUG,"%s header length is %u\n",tag_name,m_TagDataLength);

	size+=3;

	//unsigned int time_stamp = Get_Int_Reverse(src+size,4);
	
	unsigned int time_stamp = Get_Int(src+size,3);
	size +=3;
	time_stamp += (*(src+size))*256*256*256;

	size++;
//	printf("%s timestamp is %u\n",tag_name,time_stamp);
	log_to_file(FLOG_NORMAL,"%s timestamp is %u\n",tag_name,time_stamp);

	unsigned int tmp_id =  Get_Int(src+size,3);
//	printf("%s stream id is %u\n",tag_name,tmp_id);
	
	size+=3;


	return size;
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
	
	
	start_recieve();

	printf("receive data end\n");
	return 0;
}

int FLV_Demux::check_recover()
{
	if(m_Rec == STATE_OVER)
	{
		return 1;
	}
	return 0;
}

void* FLV_Demux::pop_videoitem()
{
	return m_VideoQueue->PopItem();
}

void* FLV_Demux::pop_audioitem()
{
	return m_AudioQueue->PopItem();
}

int FLV_Demux::get_scriptitem(void* dst,int* dst_len)
{
	return m_ScriptContent->memcpy_front(dst,dst_len);
}

int FLV_Demux::get_videocount()
{
	return m_VideoQueue->GetCount();
}

int FLV_Demux::get_audiocount()
{
	return m_AudioQueue->GetCount();
}

int	FLV_Demux::get_flvhead(void* dst,int* dst_len)
{
	return m_FlvheadContent->memcpy_front(dst,dst_len);
}