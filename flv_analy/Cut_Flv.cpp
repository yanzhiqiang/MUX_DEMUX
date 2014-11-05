#include "Simple_Queue.h"
#include "Flv_Demux.h"
#include "Cut_Flv.h"
#include <stdio.h>
#include "configure_flv.h"

CUT_Flv::CUT_Flv()
{
	init_param();
}

CUT_Flv::~CUT_Flv()
{
	if(m_scriptcontent)
	{
		free(m_scriptcontent);
		m_scriptcontent=NULL;
	}
	if(m_flvheadcontent)
	{
		free(m_flvheadcontent);
		m_flvheadcontent=NULL;
	}
}

int CUT_Flv::init_param()
{
	int ret = 0;
	m_flv=NULL;
	m_scriptcontent=NULL;
	b_script=false;
	m_flvheadcontent=NULL;
	b_flvhead=false;
	return ret;
}

int CUT_Flv::set_flvdemux(FLV_Demux* d_flv)
{
	if(d_flv)
	{
		m_flv = d_flv;
	}
	return 0;
}

int CUT_Flv::cut_flv(int start_time,int end_time,char* file_name)
{
	int ret = 0;
	FILE* fp = NULL;
	char*  t_filename = (char*)calloc(strlen(file_name)+1,sizeof(char));
	m_scriptcontent = (char*)calloc(READ_BUFFERSIZE,sizeof(unsigned char));
	m_flvheadcontent = (char*)calloc(READ_BUFFERSIZE,sizeof(unsigned char));
	struct Video_Frame *t_VideoFrame=NULL;
	struct Audio_Frame *t_AudioFrame=NULL;
	//open file_bak
	sprintf(t_filename,"%s",file_name);
	fp = fopen(t_filename,"wb+");
	//int pre_size = 0;//int 是四个字节
	char pre_size[4]={0};
	int pre_size_len = 4;
	if(fp)
	{
		
		while(!m_flv->check_recover() || (m_flv->get_videocount() != 0) || (m_flv->get_audiocount() != 0))
		{
			if(!b_flvhead)
			{
				int len_flvhead = 0;
				//写头部

				if( 0 != m_flv->get_flvhead(m_flvheadcontent,&len_flvhead))
				{
					continue;
				}
				fwrite(m_flvheadcontent,sizeof(unsigned char),len_flvhead,fp);
				//fwrite(
				memset(pre_size,0,pre_size_len);
				fwrite(pre_size,sizeof(int),1,fp);

				b_flvhead=true;
			}

			if(!b_script)
			{
				int len_script = 0;
				//写头部
				//if( 0 != m_flv->get_flvhead(

				if( 0 != m_flv->get_scriptitem(m_scriptcontent,&len_script))
				{
					continue;
				}
				fwrite(m_scriptcontent,sizeof(unsigned char),len_script,fp);
				//pre_size = len_script;
				//char* t_size = len_script
				memcpy(pre_size,&len_script,pre_size_len);
				reverse_str(pre_size,pre_size_len);
				fwrite(pre_size,sizeof(int),1,fp);
				
				b_script = true;
			}

			//t_VideoFrame=NULL;
			//t_AudioFrame=NULL;
			t_VideoFrame = (struct Video_Frame*)m_flv->pop_videoitem();
			t_AudioFrame = (struct Audio_Frame*)m_flv->pop_audioitem();
			//先处理音频
			if(t_AudioFrame)
			{
				fwrite(t_AudioFrame->a_Buffer,sizeof(unsigned char),t_AudioFrame->size,fp);
				
				memcpy(pre_size,&t_AudioFrame->size,pre_size_len);
				reverse_str(pre_size,pre_size_len);
				free(t_AudioFrame->a_Buffer);
				fwrite(pre_size,sizeof(int),1,fp);
			}
			if(t_VideoFrame)
			{
				fwrite(t_VideoFrame->v_Buffer,sizeof(unsigned char),t_VideoFrame->size,fp);
				
				memcpy(pre_size,&t_VideoFrame->size,pre_size_len);
				reverse_str(pre_size,pre_size_len);

				free(t_VideoFrame->v_Buffer);
				fwrite(pre_size,sizeof(int),1,fp);
			}
		}

		//rename
		
		ret = rename(t_filename,file_name);
		printf("rename %s to %s return:[%d],reason:[%s]\n"
				,t_filename,file_name,ret,strerror(errno));
	}

CUT_FLV_END:
	if(t_filename)
	{
		free(t_filename);
		t_filename=NULL;
	}
	if(fp)
	{
		fclose(fp);
		fp=NULL;
	}
	return ret;
}