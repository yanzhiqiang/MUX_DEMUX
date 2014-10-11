#ifndef __SIMPLE_QUEUE_H_
#define __SIMPLE_QUEUE_H_


class Simple_Queue
{
public:
	Simple_Queue();
	~Simple_Queue();

	int init(int num,int single_size);
	int PushItem(void* item);
	void* PopItem();

private:
	int initparam();

	//HANDLE	m_mutex;
	void**	m_itemlink;
	int		m_wnum;
	int		m_rnum;
	int		m_wpos;
	int		m_rpos;
	int		m_num;
	int		m_size;
};

#endif