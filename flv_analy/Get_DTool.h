#ifndef __GET_DTOOL_H_
#define __GET_DTOOL_H_

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


unsigned int Get_Int(unsigned char*src,int btyes);


#endif