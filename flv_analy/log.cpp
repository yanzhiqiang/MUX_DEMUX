#include "log.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

//int log_level = FLOG_NORMAL;
int log_level=3;
char file_name[20]="log";
int MAX_FILE_SIZE=8*1024*1024;
int FILE_NUM=8;

int set_log_filename(const char* t_file_name)
{
	if(file_name)
	{
		if(strlen(t_file_name) < 20 )
		{
			memset(file_name,0,20);
			memcpy(file_name,t_file_name,strlen(t_file_name));
		}
	}
	return 0;
}

int set_log_level(const int t_log_level)
{	
	log_level=t_log_level;
	return 0;
}

int log_to_file(const int t_log_level,const char* format, ...)
{
	
	if(!file_name)
	{
		return -1;
	}


	char current_file_name[20]={0};
	sprintf_s(current_file_name,20,"%s_log.txt",file_name);

	FILE* fp = NULL;
	fopen_s(&fp,current_file_name,"a");
	if(fp)
	{

		struct stat file_stat;
		if( stat(current_file_name,&file_stat) < 0 )
		{
			return -2;
		}

		if(file_stat.st_size>MAX_FILE_SIZE)
		{
			for(int i=FILE_NUM-2;i>=0;i--)
			{
				char s_file_name[20]={0};
				char t_file_name[20]={0};
				if(i!=0)
				{
					sprintf_s(s_file_name,20,"%s_log%d.txt",file_name,i);
				}
				else
				{
					sprintf_s(s_file_name,20,"%s_log.txt",file_name);
				}
				sprintf_s(t_file_name,20,"%s_log%d.txt",file_name,i+1);
				//if(access(s_file_name,F_OK)==0)
				{
					rename(s_file_name,t_file_name);
				}
			}

		}	

		if(t_log_level < log_level)
			return 0;
		time_t cur_time = time(NULL);
		char timestr[32];
		memset(timestr,0,32);

		//sprintf_s(timestr,"%s : ",ctime_s(&cur_time));
		ctime_s(timestr,32,&cur_time);

		timestr[24] = 0;

		fprintf(fp, "%s: ", timestr);

		va_list paramlist;
		va_start(paramlist,format);
		vfprintf(fp, format, paramlist);
		va_end(paramlist);

		fprintf(fp,"\n");
		fflush(fp);
		fclose(fp);
	}
	return 0;
}