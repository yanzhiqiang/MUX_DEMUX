#ifndef __CUT_FLV_H_
#define	__CUT_FLV_H_

class FLV_Demux;
class Simple_Queue;

class CUT_Flv
{
private:
	FLV_Demux* m_flv;
	char* m_scriptcontent;
	struct Video_Frame m_VideoFrame;
	struct Audio_Frame m_AudioFrame;
	bool  b_script;
	int	init_param();
	bool  b_flvhead;
	char* m_flvheadcontent;

public:
	CUT_Flv();
	~CUT_Flv();
	//set flv
	int set_flvdemux(FLV_Demux* d_flv);
	//切割flv 文件，ms级别的。
	int cut_flv(int start_time,int end_time,char* file_name);

};

#endif