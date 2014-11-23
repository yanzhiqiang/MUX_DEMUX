#include "Judge_Type.h"

int CheckFLV(unsigned char* src)
{
	if(src[0] == 0x46 && src[1] == 0x4C && src[2] == 0x56)
	{
		return 0;
	}

	return -1;
}