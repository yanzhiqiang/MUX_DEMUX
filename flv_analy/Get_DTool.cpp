#include "Get_DTool.h"
#include <Windows.h>
#include <stdio.h>

unsigned int Get_Int(unsigned char*src,int btyes)
{
	unsigned int tmp=0;
	for(int i=0;i<btyes;i++)
	{
		tmp = tmp*256+(*(src+i));
		//printf("tmp %u\n",tmp);
	}
	return tmp;

}