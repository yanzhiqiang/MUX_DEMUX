#include "Get_DTool.h"
#include <Windows.h>
#include "Configure_Flv.h"

unsigned int Get_Int(unsigned char*src,int bytes)
{
	unsigned int tmp=0;
	for(int i=0;i<bytes;i++)
	{
		tmp = tmp*256+(*(src+i));
		//printf("tmp[%u] [%d] %u\n",tmp,i,src[i]);
	}
 	return tmp;

}

unsigned int Get_Int_Reverse(unsigned char* src,int bytes)
{
	unsigned int tmp=0;
	for(int i=0;i<bytes;i++)
	{
		tmp = tmp*256+(*(src+bytes-i));
	}
	return tmp;
}

unsigned int Get_Bits(unsigned char* src,unsigned int bits)
{
	return (*src) & bits;
}


double char2double(unsigned char * buf,unsigned int size)
{
	 double scr = 0.0;
	 unsigned char buf_1[8];
	 unsigned char buf_2[8];
	 memcpy(buf_1,buf,size);
	 //��С������
	 buf_2[0] = buf_1[7];
	 buf_2[1] = buf_1[6];
	 buf_2[2] = buf_1[5];
	 buf_2[3] = buf_1[4];
	 buf_2[4] = buf_1[3];
	 buf_2[5] = buf_1[2];
	 buf_2[6] = buf_1[1];
	 buf_2[7] = buf_1[0];
	 scr = *(double *)buf_2;
	 return scr;
}


double double2char(double dst_duration,unsigned char *buf)
{
	 unsigned char buf_1[8];
	 unsigned char* buf_2= buf;
	 memcpy(buf_1,&dst_duration,8);
	 //��С������
	 buf_2[0] = buf_1[7];
	 buf_2[1] = buf_1[6];
	 buf_2[2] = buf_1[5];
	 buf_2[3] = buf_1[4];
	 buf_2[4] = buf_1[3];
	 buf_2[5] = buf_1[2];
	 buf_2[6] = buf_1[1];
	 buf_2[7] = buf_1[0];
	 
	 return 0.0;
}



int reverse_str(char* src,int src_len)
{
	int i=0;
	int j=src_len-1;
	char t;
	while(i<j)
	{
		t=src[i];
		src[i]=src[j];
		src[j]=t;
		i++;
		j--;
	}
	return 0;
}


unsigned int Get_TimeStamp(unsigned char* src)
{
	int index_timestamp=4;	//��tag�ĵ�4���ֽڿ�ʼ��������4���ֽڡ�

	unsigned int timestamp = Get_Int(src+index_timestamp,3);
	index_timestamp +=3;
	timestamp += (*(src+index_timestamp))*256*256*256;
	return timestamp;
}


unsigned int Add_TimeStamp(unsigned char* src,unsigned int duration)
{
	int index_timestamp =4;
	int content_len=2;
	int extra_num = 0;
	unsigned char content_modify[4]={0};
	memcpy(content_modify,&duration,sizeof(unsigned int));
	int num_sum = 0;
	int num_b = 0;

	for(int i=content_len;i>=0;i--)
	{
		num_sum = *(src+index_timestamp+i);
		num_b = content_modify[content_len-i];
		num_sum=num_sum+num_b +extra_num;
		extra_num = num_sum/256;
		*(src+index_timestamp+i) = num_sum%256;
	}
	*(src+index_timestamp+4)+=content_modify[3]+extra_num;
	return 0;
}

int Get_Duration(unsigned char* src,int src_size,char* match_word,double* dst_content)
{
	//��һ���ֽ��ǹ̶��ġ�
	int size = 0;
	if(src[0]!=0x02)
	{
		printf("script tag 1 head(%x) is not 0x02\n"
				,src[0]);
		return -1;
	}
	size++;
	
	unsigned int tmp = Get_Int(src+size,2);
	if(tmp!=10)
	{
		printf("script tag 1 head length(%x) is not 0x0A\n"
				,*(src+size));
		return -2;
	}
	size+=2;

	//��ʼ�̶���ͷ������
	size+=10;	//�����̶���ͷ onMetaData ���Ǹ�flashvideo ����api

	//
	if((*(src+size)) != 0x08)
	{
		printf("not second tag\n");
	}
	size+=1;
	
	unsigned int array_size = Get_Int(src+size,4);
	//printf("second tag array length is %u\n"
	//		,array_size);
	size+=4;

	//analy array
	for(int i=0;i<array_size;i++)
	{
		unsigned int name_size = Get_Int(src+size,2);
		size+=2;

		char tmp_name[1024]={0};
		memcpy(tmp_name,src+size,name_size);
		size+=name_size;
		//printf("\n \t name is %s ",tmp_name);

		int tmp_size = 0;
		switch((*(src+size)))
		{
		case 0:
			tmp_size = 8;break;
		case 1:
			tmp_size = 1;break;
		case 2:
			tmp_size = -1;break;
		case 3:
			tmp_size = 2; break;
		case 'C':
			tmp_size = -2;break;
		default:
			printf("tag class is %d\n",(*(src+size)));
			break;
		}
		int tmp_class=(*(src+size));
		size++;

		if(tmp_size == 0)
		{
			printf("tag size is %d\n",tmp_size);
			//return -1;
			continue;
		}

		if(tmp_size > 0)
		{
			double tmp_value = 0.0;
			if(tmp_class == 0)
			{
				tmp_value = char2double(src+size,tmp_size);
			}
			else
			{
				tmp_value = Get_Int(src+size,tmp_size);
			}
			if(strstr(tmp_name,match_word))
			{
				*dst_content = tmp_value;
				break;
			}
			printf("%s : %lf\n",tmp_name,tmp_value);
			size+=tmp_size;
		}
		else if(tmp_size == -1)
		{
			unsigned int t_size = Get_Int(src+size,2);
			size+=2;

			char tmp_value[1024]={0};
			memcpy(tmp_value,src+size,t_size);
			printf("%s : %s\n",tmp_name,tmp_value);
			size+=t_size;
		}
		else if(tmp_size == -2)
		{
			int t_size = Get_Int(src+size,4);
			size+=4;

			char tmp_value[1024]={0};
			memcpy(tmp_value,src+size,t_size);
			printf("%s : %s\n",tmp_name,tmp_value);
			size+=tmp_size;
		}
	}

	size+=3;	//00 00 09 array final tag

	return size;
}


int Set_ScriptWord(unsigned char* src,int src_size,char* match_word,double dst_duration)
{
	//��һ���ֽ��ǹ̶��ġ�
	int size = 0;
	if(src[0]!=0x02)
	{
		printf("script tag 1 head(%x) is not 0x02\n"
				,src[0]);
		return -1;
	}
	size++;
	
	unsigned int tmp = Get_Int(src+size,2);
	if(tmp!=10)
	{
		printf("script tag 1 head length(%x) is not 0x0A\n"
				,*(src+size));
		return -2;
	}
	size+=2;

	//��ʼ�̶���ͷ������
	size+=10;	//�����̶���ͷ onMetaData ���Ǹ�flashvideo ����api

	//
	if((*(src+size)) != 0x08)
	{
		printf("not second tag\n");
	}
	size+=1;
	
	unsigned int array_size = Get_Int(src+size,4);
	//printf("second tag array length is %u\n"
	//		,array_size);
	size+=4;

	//analy array
	for(int i=0;i<array_size;i++)
	{
		unsigned int name_size = Get_Int(src+size,2);
		size+=2;

		char tmp_name[1024]={0};
		memcpy(tmp_name,src+size,name_size);
		size+=name_size;
		//printf("\n \t name is %s ",tmp_name);

		int tmp_size = 0;
		switch((*(src+size)))
		{
		case 0:
			tmp_size = 8;break;
		case 1:
			tmp_size = 1;break;
		case 2:
			tmp_size = -1;break;
		case 3:
			tmp_size = 2; break;
		case 'C':
			tmp_size = -2;break;
		default:
			printf("tag class is %d\n",(*(src+size)));
			break;
		}
		int tmp_class=(*(src+size));
		size++;

		if(tmp_size == 0)
		{
			printf("tag size is %d\n",tmp_size);
			//return -1;
			continue;
		}

		if(tmp_size > 0)
		{
			double tmp_value = 0.0;
			if(tmp_class == 0)
			{
				tmp_value = char2double(src+size,tmp_size);
			}
			else
			{
				tmp_value = Get_Int(src+size,tmp_size);
			}

			if(strstr(tmp_name,match_word) && tmp_class == 0 )
			{
				//*dst_content = tmp_value;
				double2char(dst_duration,src+size);
				break;
			}
			printf("%s : %lf\n",tmp_name,tmp_value);
			size+=tmp_size;
		}
		else if(tmp_size == -1)
		{
			unsigned int t_size = Get_Int(src+size,2);
			size+=2;

			char tmp_value[1024]={0};
			memcpy(tmp_value,src+size,t_size);
			printf("%s : %s\n",tmp_name,tmp_value);
			size+=t_size;
		}
		else if(tmp_size == -2)
		{
			int t_size = Get_Int(src+size,4);
			size+=4;

			char tmp_value[1024]={0};
			memcpy(tmp_value,src+size,t_size);
			printf("%s : %s\n",tmp_name,tmp_value);
			size+=tmp_size;
		}
	}

	size+=3;	//00 00 09 array final tag

	return size;
}



int  chararray2intarray(char* src,char* delim,int* dst,int dst_num)
{
	if(!src)
	{
		return -1;
	}
	char* t_src = src;
	for(int i=0;i<dst_num;i++)
	{
		t_src = strstr(src,delim);
		if(t_src)
		{
			char tmp[20]={0};
			memcpy(tmp,src,t_src-src);
			dst[i]=atoi(tmp);
		}
		else
		{
			return -1;
		}
		src = t_src+1;
	}
	return 0;
}


int	 strstrend(char* src,char* pattern)
{
	if(!src || !pattern)
	{
		/*printf("src(%x) or pattern(%x) is NULL\n"
				,src,pattern);*/
		return -1;
	}

	if(strlen(pattern) > strlen(src))
	{
		printf("pattern len(%d) > src len(%d)\n"
				,strlen(pattern),strlen(src));
		return -1;
	}

	int index_p = strlen(src)-strlen(pattern);
	for(int i =  index_p;i<strlen(src);i++)
	{
		if(src[i] != pattern[i-index_p])
		{
			
			return -1;
		}
	}
	printf("src(%s)  include pattern(%s)\n"
					,src,pattern);
	return 0;
}


int  get_socketvalue(int socket,unsigned char* recv_buf,int recv_len)
{
	ULONGLONG timestamp = GetTickCount64();
	int w_pos = 0;
	//bool http_status = false;
	while(GetTickCount64() <= timestamp + TIMEOUT_SOCKET*3)
	{
		ULONGLONG now = GetTickCount64();
		//����select
		fd_set fd;  
		FD_ZERO(&fd);  
		FD_SET(socket,&fd);
		timeval t = {TIMEOUT_SOCKET,1};
		int iResult = select(0,&fd,NULL,NULL,&t);
		if(iResult<0)
		{
			printf("select error\n");
			return -1;
		}
		else if(iResult == 0)
		{
			continue;
		}
		else
		{
			int ac_recvlen = recv(socket,(char*)(recv_buf+w_pos),recv_len-w_pos,0);
			
			while(ac_recvlen > 0)
			{
				w_pos+=ac_recvlen;

				if(w_pos >= recv_len)
				{
					//printf("recieve data already \n");
					return w_pos;
				}

				ac_recvlen = recv(socket,(char*)(recv_buf+w_pos),recv_len-w_pos,0);
			}
			

		}
	}
	return w_pos;
}