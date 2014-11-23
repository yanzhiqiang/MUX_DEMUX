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
	sprintf_s(t_filename,strlen(file_name)+1,"%s",file_name);
	fopen_s(&fp,t_filename,"wb+");
	
	char pre_size[4]={0};
	int pre_size_len = 4;

	if(fp)
	{
		unsigned int video_timestamp = TIMESTAMP_MAX;
		unsigned int audio_timestamp = TIMESTAMP_MAX;
		unsigned int base_timestamp = 0;
		unsigned int video_lttimestamp = 0;
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
				base_timestamp = m_flv->get_tagtimestamp((unsigned char*)m_scriptcontent);
				printf("base timestamp is %u\n",base_timestamp);
				fwrite(m_scriptcontent,sizeof(unsigned char),len_script,fp);
				memcpy(pre_size,&len_script,pre_size_len);
				reverse_str(pre_size,pre_size_len);
				fwrite(pre_size,sizeof(int),1,fp);
				
				b_script = true;
			}

			//修改一下，修改成取音频和视频的对应时间戳小的写入flv文件中。
			//总体思路是：如果flv视频文件中存在视频，取视频，如果取成功了，获取时间戳。音频也是如此.
			//改进，start_time是在基准时间上+start_time。基准时间是可以改变的。加个超时时间限制，当长时间来单一的tag。不在取值范围的要进行处理。

			t_VideoFrame = (struct Video_Frame*)m_flv->pop_videoitem();
			if(t_VideoFrame)
			{
				video_timestamp=m_flv->get_tagtimestamp(t_VideoFrame->v_Buffer);
			}


			t_AudioFrame = (struct Audio_Frame*)m_flv->pop_audioitem();
			if(t_AudioFrame)
			{
				audio_timestamp=m_flv->get_tagtimestamp(t_AudioFrame->a_Buffer);	
			}

			
			//如果视频存在
			if(t_VideoFrame)
			{
				if(video_timestamp > video_lttimestamp + TIMESTAMP_ROLLBACK)
				{
					start_time += video_timestamp;
					end_time += video_timestamp;
					printf("timestamp rollback [%u] to [%u] \n ",video_lttimestamp,video_timestamp);
				}
			}

			//设置上次时间戳
			video_lttimestamp=video_timestamp;

			//先处理音频
			if(t_AudioFrame)
			{
				if(audio_timestamp >= start_time && audio_timestamp < end_time)
				{
					fwrite(t_AudioFrame->a_Buffer,sizeof(unsigned char),t_AudioFrame->size,fp);

					memcpy(pre_size,&t_AudioFrame->size,pre_size_len);
					reverse_str(pre_size,pre_size_len);
					
					fwrite(pre_size,sizeof(int),1,fp);

				}
				free(t_AudioFrame->a_Buffer);

			}
			if(t_VideoFrame)
			{
				if(video_timestamp >= start_time && video_timestamp < end_time)
				{
					fwrite(t_VideoFrame->v_Buffer,sizeof(unsigned char),t_VideoFrame->size,fp);

					memcpy(pre_size,&t_VideoFrame->size,pre_size_len);
					reverse_str(pre_size,pre_size_len);

					
					fwrite(pre_size,sizeof(int),1,fp);

				}
				
				free(t_VideoFrame->v_Buffer);
				//printf("free video videotime=%u buf_addr=%x\n",video_timestamp,t_VideoFrame->v_Buffer);
			}
		}

		//rename
		
		ret = rename(t_filename,file_name);
		printf("rename %s to %s return:[%d]\n"
				,t_filename,file_name,ret);
	}

//CUT_FLV_END:
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