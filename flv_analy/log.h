#ifndef __LOG_H_
#define __LOG_H_


#define	FLOG_DEBUG  1
#define	FLOG_WARNNING  2
#define	FLOG_NORMAL  3
#define	FLOG_ERR  4



int set_log_filename(const char* file_name);
int set_log_level(const int log_level);
int log_to_file(const int log_level,const char* format, ...);

#endif