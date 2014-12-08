//
//  main.cpp
//  flv_analy
//
//  Created by 闫志强 on 14-9-24.
//  Copyright (c) 2014年 闫志强. All rights reserved.
//

#include <iostream>
#include "log.h"
#include "Flv_Demux.h"
#include "Cut_Flv.h"
#include <Windows.h>
#include "Merge_FLV.h"
#include <direct.h>

int g_cut = 0;
int g_merge = 0;

int Read_Config();

int main(int argc, const char * argv[])
{
	
	set_log_filename("flv_demux");
	set_log_level(FLOG_NORMAL);
	log_to_file(FLOG_NORMAL,"flv demux version v1.0.0 d20141201");
	
	char* cur_dir = _getcwd(NULL,0);
	printf("cur dir is [%s]\n",cur_dir);
	free(cur_dir);

	//添加读写配置文件
	Read_Config();
	printf("cut:[%d] merge:[%d]\n",g_cut,g_merge);
	if(g_cut)
	{
		if(argc < 4)
		{
			printf("argc should >=4 \n");
			log_to_file(FLOG_ERR,"argc should >=4");
			return 0;
		}

		
		FLV_Demux* flv_demux =  new FLV_Demux();
		CUT_Flv*   t_cutflv = new CUT_Flv();
		if(flv_demux)
		{
			t_cutflv->set_flvdemux(flv_demux);
			flv_demux->init(argv[1]);

			//两个问题
			t_cutflv->cut_flv(atoi(argv[2]),atoi(argv[3]),(char*)argv[4]);
			delete flv_demux;
		}
	}
	if(g_merge)
	{
		if(argc < 4)
		{
			printf("argc should >=4 \n");
			log_to_file(FLOG_ERR,"argc should >=4");
			return 0;
		}

		
		Merge_FLV*   t_cutflv = new Merge_FLV();
		t_cutflv->merge_flv(argv[2],argv[3],argv[1]);
	}
	getchar();
	return 0;
}

int Read_Config()
{
	FILE* fp = fopen("config.ini","rb");
	if(fp)
	{
		char content[1024]={0};
		fread(content,sizeof(char),1024,fp);
		if(strstr(content,"CUT_ON"))
		{
			char* index = strstr(content,"CUT_ON");
			
			sscanf(index,"CUT_ON=%d",&g_cut);
		}
		if(strstr(content,"MER_ON"))
		{
			char* index = strstr(content,"MER_ON");
			
			sscanf(index,"MER_ON=%d",&g_merge);
		}

		fclose(fp);
		fp=NULL;
	}
	else
	{
		//printf("fopen  error ,reason is [%s]",strerror(errno));
	}
	return 0;
}