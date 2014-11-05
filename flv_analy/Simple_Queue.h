#ifndef __SIMPLE_QUEUE_H_
#define __SIMPLE_QUEUE_H_

#include <Windows.h>

class Simple_Queue
{
public:
	Simple_Queue();
	~Simple_Queue();

	int init(int num,int single_size);
	int PushItem(void* item);
	void* PopItem();
	int GetCount();

private:
	int initparam();

	void**	m_itemlink;
	int		m_wnum;
	int		m_rnum;
	int		m_wpos;
	int		m_rpos;
	int		m_num;
	int		m_size;
	CRITICAL_SECTION m_cslock;
};

#endif