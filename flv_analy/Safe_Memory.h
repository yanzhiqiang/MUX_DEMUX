#ifndef __SAFE_MEMORY_H_
#define __SAFE_MEMORY_H_

#include <Windows.h>

class Safe_Memory
{

public:
	Safe_Memory();
	~Safe_Memory();
	
	int memcpy_push(void* src,int src_len);
	int memcpy_front(void* dst,int* dst_len);


private:
	void* m_buffer;
	int   m_len;
	CRITICAL_SECTION m_cslock;
};

#endif