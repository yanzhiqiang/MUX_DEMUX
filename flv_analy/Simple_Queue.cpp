#include "Simple_Queue.h"
#include <Windows.h>

Simple_Queue::Simple_Queue()
{
	//m_mutex = CreateMutex(NULL, FALSE, NULL);
	initparam();
}

Simple_Queue::~Simple_Queue()
{
	//CloseHandle(m_mutex);
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

	for(int i=0;i<m_num;i++)
	{
		m_itemlink[i] = (void*)calloc(size,sizeof(char));
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
	memcpy(m_itemlink[m_wpos],item,m_size);
	m_wpos = (m_wpos+1)%m_num;
	m_wnum++;
	//ReleaseMutex(m_mutex);
}

void* Simple_Queue::PopItem()
{
	int t_pos = m_rpos;
	m_rnum++;
	m_rpos = (m_rpos+1)%m_num;
	return m_itemlink[t_pos];
}