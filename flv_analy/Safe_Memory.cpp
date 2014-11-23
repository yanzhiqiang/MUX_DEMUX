#include "Safe_Memory.h"
#include <stdio.h>
#include <string.h>

Safe_Memory::Safe_Memory()
{
	InitializeCriticalSection(&m_cslock);
	m_buffer=NULL;
	m_len=0;
}

Safe_Memory::~Safe_Memory()
{
	if(m_buffer)
	{
		free(m_buffer);
		m_buffer=NULL;
	}
}

int Safe_Memory::memcpy_push(void* src,int src_len)
{
	int ret = 0;
	EnterCriticalSection(&m_cslock);
	if(m_buffer)
	{
		free(m_buffer);
		m_buffer=NULL;
		
	}
	if(!m_buffer)
	{
		m_len=src_len;
		m_buffer = calloc(sizeof(char),m_len);
		
		if(!m_buffer)
		{
			ret = -1;
			goto memcpy_safe_end;
		}
	}
	memcpy(m_buffer,src,src_len);
memcpy_safe_end:

	LeaveCriticalSection(&m_cslock);
	return ret;
}

int Safe_Memory::memcpy_front(void* dst,int* dst_len)
{
	int ret = 0;
	
	EnterCriticalSection(&m_cslock);
	if(m_buffer == NULL)
	{
		ret = -1;
	}
	else
	{
		memcpy(dst,m_buffer,m_len);
		*dst_len=m_len;
	}
	LeaveCriticalSection(&m_cslock);
	return ret;
}