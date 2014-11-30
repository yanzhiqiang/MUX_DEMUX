#ifndef __MERGE_FLV_H_
#define __MERGE_FLV_H_

class Merge_FLV
{
public:
	Merge_FLV();
	~Merge_FLV();

	int init_params();
	int merge_flv(const char* src_filename1,const char* src_filename2,const char* dst_filename);

private:
	int handle_filetag(const char* src_filename,const char* dst_filename,unsigned char* script_content,unsigned int* duration);
	
	int tag_adjusttimestamp(unsigned char* src_tagcontent,unsigned int duration);
};


#endif