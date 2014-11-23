#include "Merge_FLV.h"
#include <stdio.h>

Merge_FLV::Merge_FLV()
{
	init_params();
}

Merge_FLV::~Merge_FLV()
{

}

int Merge_FLV::init_params()
{
	return 0;
}

int Merge_FLV::merge_flv(const char* src_filename1,const char* src_filename2,const char* dst_filename)
{
	FILE* fp_src1=NULL;
	FILE* fp_src2=NULL;
	FILE* fp_dst=NULL;
	int ret = 0;

	//open and init fp_src1,
	fopen_s(&fp_src1,src_filename1,"rb");
	fopen_s(&fp_src2,src_filename2,"rb");
	fopen_s(&fp_dst,dst_filename,"wb");

	if(!fp_src1 || !fp_src2 || !fp_dst)
	{
		printf("Merge_FLV::merge_flv open [%s],return:[%x],open:[%s],return:[%x],open:[%s],return:[%x]\n"
			,src_filename1,fp_src1,src_filename2,fp_src2,dst_filename,fp_dst);
	}

MERGE_FLV_END:
	if(fp_src1)
	{
		fclose(fp_src1);
		fp_src1 = NULL;
	}
	if(fp_src2)
	{
		fclose(fp_src2);
		fp_src2 = NULL;
	}
	if(fp_dst)
	{
		fclose(fp_dst);
		fp_dst = NULL;
	}
	printf("Merge_FLV::merge_flv return [%d]\n",ret);
	return ret;
}