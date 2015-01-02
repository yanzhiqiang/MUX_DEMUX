#ifndef __GET_DTOOL_H_
#define __GET_DTOOL_H_

#include <stdio.h>

struct Video_Frame
{
	int size;
	unsigned char* v_Buffer;
};

struct Audio_Frame
{
	int size;
	unsigned char* a_Buffer;
};


unsigned int Get_Int(unsigned char*src,int bytes);
unsigned int Get_Int_Reverse(unsigned char* src,int bytes);

double char2double(unsigned char * buf,unsigned int size);

unsigned int Get_Bits(unsigned char* src,unsigned int bits);

int reverse_str(char* src,int src_len);
/*
��ȡtimestamp��
*/
unsigned int Get_TimeStamp(unsigned char* src);

/*
����timestamp
*/
unsigned int Add_TimeStamp(unsigned char* src,unsigned int duration);

/*
��ȡduration
*/
int Get_Duration(unsigned char* src,int src_size,char* match_word,double* dst_content);

/*
����duration
*/
int Set_ScriptWord(unsigned char* src,int src_size,char* match_word,double dst_duration);

/*
double2char 
for example  'duration' in scriptcontent
*/
double double2char(double dst_duration,unsigned char *buf);

/*
char* תint
*/
int  chararray2intarray(char* src,char* delim,int* dst,int dst_num);

/*
�ַ�������ƥ��
*/
int	 strstrend(char* src,char* pattern);


int  get_socketvalue(int socket,unsigned char* recv_buf,int recv_len);
#endif