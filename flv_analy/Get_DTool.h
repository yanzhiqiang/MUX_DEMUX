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
ªÒ»°timestamp°£
*/
unsigned int Get_TimeStamp(unsigned char* src);

#endif