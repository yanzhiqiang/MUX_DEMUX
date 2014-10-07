#ifndef __FLV_DEMUX_H
#define __FLV_DEMUX_H


class FLV_Demux
{
public:
	FLV_Demux();
	~FLV_Demux();

	/*
	param: 
		filename	//filename shoule be a file name,not url 
	return 
		0	ok
		<0  error
	*/
	int init(const char* filename);
private:
	char* m_FileName;
	unsigned char** m_Content;
	int	m_ContentLength;
	int m_ContentSize;
	bool m_BFirst;
	int setinit_params();
	int start_analy();
};

#endif