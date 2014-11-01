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


unsigned int Get_Int(unsigned char*src,int bytes);
unsigned int Get_Int_Reverse(unsigned char* src,int bytes);

double char2double(unsigned char * buf,unsigned int size);

unsigned int Get_Bits(unsigned char* src,unsigned int bits);

#endif