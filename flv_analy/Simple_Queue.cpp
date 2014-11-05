#include "Simple_Queue.h"
#include <Windows.h>
#include <stdio.h>

Simple_Queue::Simple_Queue()
{
	//InitializeCriticalSection(&m_cslock);
	initparam();
}

Simple_Queue::~Simple_Queue()
{
	
	for(int i=0;i<m_num;i++)
	{
		if(m_itemlink[i])
		{
			free(m_itemlink[i]);
			m_itemlink[i]=NULL;
		}
	}
	if(m_itemlink)
	{
		free(m_itemlink);
		m_itemlink=NULL;
	}
}

int Simple_Queue::initparam()
{
	m_itemlink = NULL;
	m_wnum = 0;
	m_rnum = 0;
	m_wpos = 0;
	m_rpos = 0;
	m_num  = 0;
	m_size = 0;
	return 0;
}

//³õÊ¼»¯
int Simple_Queue::init(int num,int size)
{
	int ret = 0;
	m_itemlink = (void**)calloc(num,sizeof(char*));

	if(!m_itemlink)
	{
		ret = -1;
		goto init_end;
	}
	m_num = num;
	m_size = size;

	for(int i=0;i<m_num;i++)
	{
		m_itemlink[i] = (void*)calloc(m_size,sizeof(char));
		if(!m_itemlink[i])
		{
			ret = -1;
			goto init_end;
		}
	}

init_end:
	return ret;
}

int Simple_Queue::PushItem(void* item)
{
	if(m_wnum >= m_rnum + m_num)
	{
		return -1;
	}

	//no need to lock mutex and instant copy
	//WaitForSingleObject(m_mutex, INFINITE);
	//EnterCriticalSection(&m_cslock);
	//printf("m_size is %d\n",m_size);
	memcpy(m_itemlink[m_wpos],item,m_size);
	m_wpos = (m_wpos+1)%m_num;
	m_wnum++;
	return 0;
	//LeaveCriticalSection(&m_cslock);
	//ReleaseMutex(m_mutex);
}

int Simple_Queue::GetCount()
{
	if(m_rnum >= m_wnum)
	{
		return 0;
	}
	
	return m_wnum - m_rnum;
}

void* Simple_Queue::PopItem()
{
	if(m_rnum >= m_wnum )
	{
		return NULL;
	}

	int t_pos = m_rpos;

	m_rnum++;
	m_rpos = (m_rpos+1)%m_num;
	return m_itemlink[t_pos];
}