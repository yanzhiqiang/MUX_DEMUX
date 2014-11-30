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
	return 0;
}

int Merge_FLV::merge_flv(const char* src_filename1,const char* src_filename2,const char* dst_filename)
{
	int script_size = READ_BUFFERSIZE;
	unsigned char* script_content1 = NULL;
	unsigned char* script_content2 = NULL;

	script_content1 = new unsigned char[script_size];
	script_content2 = new unsigned char[script_size];
	unsigned int duration = 0;
	handle_filetag(src_filename1,dst_filename,script_content1,&duration);
	handle_filetag(src_filename2,dst_filename,script_content2,&duration);

	delete[] script_content1;
	script_content1=NULL;
	delete[] script_content2;
	script_content2=NULL;
	return 0;
}

int Merge_FLV::handle_filetag(const char* src_filename,const char* dst_filename,unsigned char* script_content,unsigned int* duration)
{
	FILE* fp_src=NULL;
	FILE* fp_dst=NULL;
	int ret = 0;
	FLV_Demux* t_flvdemux = NULL;
	int file_len = 2;

	//open and init fp_src1,
	fopen_s(&fp_src,src_filename,"rb");
	fopen_s(&fp_dst,dst_filename,"a+");

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
	while(!t_flvdemux->check_recover() || (t_flvdemux->get_videocount() != 0) || (t_flvdemux->get_audiocount() != 0))
	{
		if((*duration) == 0)
		{
			int len_flvhead = 0;
			//写头部

			if( 0 != t_flvdemux->get_flvhead(script_content,&len_flvhead))
			{
				continue;
			}
			fwrite(script_content,sizeof(unsigned char),len_flvhead,fp_dst);
			//fwrite(
			memset(pre_size,0,pre_size_len);
			fwrite(pre_size,sizeof(int),1,fp_dst);

		}

		if(!b_script)
		{
			if( 0 != t_flvdemux->get_scriptitem(script_content,&len_script))
			{
				continue;
			}

			if((*duration) == 0)
			{

				fwrite(script_content,sizeof(unsigned char),len_script,fp_dst);
				memcpy(pre_size,&len_script,pre_size_len);
				reverse_str(pre_size,pre_size_len);
				fwrite(pre_size,sizeof(int),1,fp_dst);

			}
			b_script=true;
		}
		
		t_VideoFrame = (struct Video_Frame*)t_flvdemux->pop_videoitem();
		t_AudioFrame = (struct Audio_Frame*)t_flvdemux->pop_audioitem();
		
		//先处理音频
		if(t_AudioFrame)
		{
			if((*duration)!=0)
			{
				//将timestamp+=duration，然后写到文件中
				tag_adjusttimestamp(t_AudioFrame->a_Buffer,*duration);
			}
			fwrite(t_AudioFrame->a_Buffer,sizeof(unsigned char),t_AudioFrame->size,fp_dst);

			memcpy(pre_size,&t_VideoFrame->size,pre_size_len);
			reverse_str(pre_size,pre_size_len);
			fwrite(pre_size,sizeof(int),1,fp_dst);
			
			free(t_AudioFrame->a_Buffer);

		}
		if(t_VideoFrame)
		{
			if((*duration) != 0)
			{
				/**/
				tag_adjusttimestamp(t_VideoFrame->v_Buffer,*duration);

			}
			fwrite(t_VideoFrame->v_Buffer,sizeof(unsigned char),t_VideoFrame->size,fp_dst);

			memcpy(pre_size,&t_VideoFrame->size,pre_size_len);
			reverse_str(pre_size,pre_size_len);
			fwrite(pre_size,sizeof(int),1,fp_dst);
			
			free(t_VideoFrame->v_Buffer);
			
		}
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

	unsigned time_stamp = Get_TimeStamp(src_tagcontent);

	time_stamp+=duration;

	printf("timestamp adjust before is %u\n",time_stamp);

	memcpy(src_tagcontent+4,&time_stamp,sizeof(unsigned char)*4);
	
	//check 
	time_stamp = Get_TimeStamp(src_tagcontent);
	
	printf("timestamp adjust after is %u\n",time_stamp);
	return ret;
}