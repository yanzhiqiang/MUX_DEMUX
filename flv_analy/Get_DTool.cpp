#include "Get_DTool.h"
#include <Windows.h>
#include <stdio.h>

unsigned int Get_Int(unsigned char*src,int bytes)
{
	unsigned int tmp=0;
	for(int i=0;i<bytes;i++)
	{
		tmp = tmp*256+(*(src+i));
		//printf("tmp %u\n",tmp);
	}
	return tmp;

}

unsigned int Get_Int_Reverse(unsigned char* src,int bytes)
{
	unsigned int tmp=0;
	for(int i=0;i<bytes;i++)
	{
		tmp = tmp*256+(*(src+bytes-i));
	}
	return tmp;
}

unsigned int Get_Bits(unsigned char* src,unsigned int bits)
{
	return (*src) & bits;
}


double char2double(unsigned char * buf,unsigned int size)
{
	 double scr = 0.0;
	 unsigned char buf_1[8];
	 unsigned char buf_2[8];
	 memcpy(buf_1,buf,size);
	 //大小端问题
	 buf_2[0] = buf_1[7];
	 buf_2[1] = buf_1[6];
	 buf_2[2] = buf_1[5];
	 buf_2[3] = buf_1[4];
	 buf_2[4] = buf_1[3];
	 buf_2[5] = buf_1[2];
	 buf_2[6] = buf_1[1];
	 buf_2[7] = buf_1[0];
	 scr = *(double *)buf_2;
	 return scr;
}

int reverse_str(char* src,int src_len)
{
	int i=0;
	int j=src_len-1;
	char t;
	while(i<j)
	{
		t=src[i];
		src[i]=src[j];
		src[j]=t;
		i++;
		j--;
	}
	return 0;
}
