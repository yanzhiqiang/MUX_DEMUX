#include "Merge_FLV.h"
#include <stdio.h>
#include "Flv_Demux.h"
#include "Configure_Flv.h"

Merge_FLV::Merge_FLV()
{
	init_params();
}

Merge_FLV::~Merge_FLV()
{

}

int Merge_FLV::init_params()
{
	m_uVideoTimestamp = TIMESTAMP_MAX;
	m_uAudioTimestamp = TIMESTAMP_MAX;
	m_lFileIndex = 0;
	m_uScriptLen = 0;
	return 0;
}

int Merge_FLV::merge_flv(const char* src_filename1,const char* src_filename2,const char* dst_filename)
{
	int script_size = READ_BUFFERSIZE;
	unsigned char* script_content1 = NULL;
	unsigned char* script_content2 = NULL;

	char dst_filenametmp[100]={0};
	memset(dst_filenametmp,0,100);
	sprintf_s(dst_filenametmp,"%_tmp",dst_filename);

	script_content1 = new unsigned char[script_size];
	script_content2 = new unsigned char[script_size];
	memset(script_content1,0,READ_BUFFERSIZE);
	memset(script_content2,0,READ_BUFFERSIZE);
	double duration = 0;
	double sum_duration=0;
	handle_filetag(src_filename1,dst_filenametmp,script_content1,&duration);
	sum_duration += duration;
	handle_filetag(src_filename2,dst_filenametmp,script_content2,&duration);
	sum_duration += duration;

	//处理离线文件的duration
	Set_ScriptWord(script_content1+11,m_uScriptLen-11,"duration",sum_duration);
	Get_Duration(script_content1+11,m_uScriptLen-11,"duration",&duration);
	printf("after adjust duration is %lf,should be is %lf",duration,sum_duration);

	//replace_filescript(dst_filenametmp,dst_filename,script_content1);

	delete[] script_content1;
	script_content1=NULL;
	delete[] script_content2;
	script_content2=NULL;
	return 0;
}

int Merge_FLV::handle_filetag(const char* src_filename,const char* dst_filename,unsigned char* script_content,double* duration)
{
	FILE* fp_src=NULL;
	FILE* fp_dst=NULL;
	int ret = 0;
	FLV_Demux* t_flvdemux = NULL;
	int file_len = 2;

	//open and init fp_src1,
	fopen_s(&fp_src,src_filename,"rb");
	fopen_s(&fp_dst,dst_filename,"ab+");

	if(!fp_src || !fp_dst)
	{
		printf("Merge_FLV::handle_filetag open [%s],return:[%x],open:[%s],return:[%x]\n"
			,src_filename,fp_src,dst_filename,fp_dst);
		ret = -1;
		goto HANDLE_FILETAG_END;
	}

	t_flvdemux = new FLV_Demux();
	if(!t_flvdemux)
	{
		printf("Merge_FLV::handle_filetag FLV_Demux(%x) new failed\n"
			,t_flvdemux);
		ret = -1;
		goto HANDLE_FILETAG_END;
	}

	t_flvdemux->init(src_filename);
	
	struct Video_Frame *t_VideoFrame=NULL;
	struct Audio_Frame *t_AudioFrame=NULL;
	int len_flvhead=0;
	int	pre_size_len=4;
	char pre_size[4] = {0};
	int len_script = 0;
	bool b_script = false;
	int  b_head = false;

	double t_duration = 0;
	unsigned lt_uduration = (unsigned int)(*duration);
	while(!t_flvdemux->check_recover() || (t_flvdemux->get_videocount() != 0) || (t_flvdemux->get_audiocount() != 0))
	{
		if(lt_uduration == 0)
		{
			if(!b_head)
			{
				int len_flvhead = 0;
				//写头部
				char flvhead_content[100]={0};
				if( 0 != t_flvdemux->get_flvhead(flvhead_content,&len_flvhead))
				{
					continue;
				}
				fwrite(flvhead_content,sizeof(unsigned char),len_flvhead,fp_dst);
				//fwrite(
				memset(pre_size,0,pre_size_len);
				fwrite(pre_size,sizeof(int),1,fp_dst);
				b_head =true;
				continue;
			}
		
		}

		if(!b_script)
		{
			memset(script_content,0,READ_BUFFERSIZE);
			if( 0 != t_flvdemux->get_scriptitem(script_content,&len_script))
			{
				continue;
			}

			//printf("get len script:[%d]\n",len_script);
			Get_Duration(script_content+11,len_script-11,"duration",&t_duration);
			if(lt_uduration == 0)
			{
				m_uScriptLen = len_script;
				m_lFileIndex = ftell(fp_dst);
				fwrite(script_content,sizeof(unsigned char),len_script,fp_dst);
				memcpy(pre_size,&len_script,pre_size_len);
				reverse_str(pre_size,pre_size_len);
				fwrite(pre_size,sizeof(int),1,fp_dst);

			}
			b_script=true;
			continue;
		}
		
	
		
		t_VideoFrame = (struct Video_Frame*)t_flvdemux->pop_videoitem();
		t_AudioFrame = (struct Audio_Frame*)t_flvdemux->pop_audioitem();
		
		//先处理音频
		if(t_AudioFrame)
		{
			if(lt_uduration!=0)
			{
				//将timestamp+=duration，然后写到文件中
				tag_adjusttimestamp(t_AudioFrame->a_Buffer,lt_uduration);
			}
			m_uAudioTimestamp = Get_TimeStamp(t_AudioFrame->a_Buffer);

			fwrite(t_AudioFrame->a_Buffer,sizeof(unsigned char),t_AudioFrame->size,fp_dst);

			memcpy(pre_size,&t_AudioFrame->size,pre_size_len);
			reverse_str(pre_size,pre_size_len);
			fwrite(pre_size,sizeof(int),1,fp_dst);
			
			free(t_AudioFrame->a_Buffer);

		}
		if(t_VideoFrame)
		{
			if((*duration) != 0)
			{
				/**/
				tag_adjusttimestamp(t_VideoFrame->v_Buffer,lt_uduration);

			}
			m_uVideoTimestamp = Get_TimeStamp(t_VideoFrame->v_Buffer);

			fwrite(t_VideoFrame->v_Buffer,sizeof(unsigned char),t_VideoFrame->size,fp_dst);

			memcpy(pre_size,&t_VideoFrame->size,pre_size_len);
			reverse_str(pre_size,pre_size_len);
			fwrite(pre_size,sizeof(int),1,fp_dst);
			
			free(t_VideoFrame->v_Buffer);
			
		}
	}


	printf("v_lasttimestamp:[%u],a_lasttimestamp:[%u],duration:[%lf]\n"
			,m_uVideoTimestamp,m_uAudioTimestamp,t_duration);

	if(t_duration)
	{
		*duration = t_duration*1000;
	}
HANDLE_FILETAG_END:
	if(fp_src)
	{
		fclose(fp_src);
		fp_src = NULL;
	}
	if(fp_dst)
	{
		fclose(fp_dst);
		fp_dst = NULL;
	}
	if(t_flvdemux)
	{
		delete t_flvdemux;
		t_flvdemux=NULL;
	}
	printf("Merge_FLV::merge_flv return [%d]\n",ret);
	return ret;
}

int Merge_FLV::tag_adjusttimestamp(unsigned char* src_tagcontent,unsigned int duration)
{
	int ret = 0;

	//unsigned time_stamp = Get_TimeStamp(src_tagcontent);
	//printf("timestamp adjust before is %u\n",time_stamp);

	

	//printf("timestamp adjust after should be %u\n",time_stamp+duration);
	
	Add_TimeStamp(src_tagcontent,duration);
	//check 
	//time_stamp = Get_TimeStamp(src_tagcontent);
	
	//printf("timestamp adjust after is %u\n",time_stamp);
	return ret;
}

int Merge_FLV::replace_filescript(const char* src_filename,const char* dst_filename,unsigned char* script_content)
{
	int ret = 0;
	//将script_content 写到文件中。
	FILE* fp_r = NULL;
	fopen_s(&fp_r,src_filename,"rb");
	FILE* fp_w =NULL;
	fopen_s(&fp_w,dst_filename,"wb+");
	unsigned char* content = NULL;
	content = (unsigned char*)calloc(READ_BUFFERSIZE,sizeof(unsigned char));
	if(fp_r && fp_w)
	{
		int r_pos = 0;
		bool bfirst = false;
		while(!feof(fp_r))
		{
			int read_len = READ_BUFFERSIZE;
			memset(content,0,READ_BUFFERSIZE);
			if(!bfirst)
			{
				if(r_pos + READ_BUFFERSIZE > m_lFileIndex)
				{
					read_len = m_lFileIndex;
					bfirst = true;
				}
			}
			if(r_pos == m_lFileIndex)
			{
				read_len = m_uScriptLen;
			}
			int actual_read = fread(content,sizeof(unsigned char),read_len,fp_r);
			
			if(r_pos == m_lFileIndex)
			{
				fwrite(script_content,sizeof(unsigned char),m_uScriptLen,fp_w);
			}
			else
			{
				fwrite(content,sizeof(unsigned char),actual_read,fp_w);
			}
			r_pos += actual_read;
			
		}
	}
	if(fp_r)
	{
		fclose(fp_r);
		fp_r = NULL;
	}
	if(fp_w)
	{
		fclose(fp_w);
		fp_w = NULL;
	}
	if(content)
	{
		delete[] content;
		content = NULL;
	}
	return ret;
}
