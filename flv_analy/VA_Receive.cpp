#include "VA_Receive.h"
#include "Configure_Flv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <winsock.h>
#include <winsock2.h>
#include "Get_DTool.h"

#pragma comment(lib,"ws2_32.lib")

VA_Receive::VA_Receive()
{
	m_SrcAddr = NULL;
	m_Mode = OFFLINE_MODE;
	m_Fp=NULL;
	m_iSocket = INVALID_SOCKET;
}

VA_Receive::~VA_Receive()
{
	if(m_SrcAddr)
	{
		free(m_SrcAddr);
		m_SrcAddr=NULL;
	}
	if(m_Fp)
	{
		fclose(m_Fp);
		m_Fp=NULL;
	}
	if(LIVE_MODE == m_Mode)
	{
		WSACleanup();
	}
}
int	VA_Receive::init_socket()
{
	int ret = 0;
	WORD wVersionRequested=MAKEWORD( 2, 2 );
	WSADATA wsaData;

	ret =  WSAStartup(wVersionRequested,&wsaData);
	if(0!=ret)
	{
		printf("no useable winsock dll for us\n");
		ret = -1;
		goto Init_Socket_End;
	}

	char* t_sSrcAddr = strstr(m_SrcAddr,LIVE_PRE);
	if(!t_sSrcAddr)
	{
		printf("socket address is not a net address(%s),not include http\n"
				,m_SrcAddr);
		ret = -1;
		goto Init_Socket_End;
	}
	m_iSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	int port = 80;
	if(t_sSrcAddr[0] == 'w' && t_sSrcAddr[1] == 'w' && t_sSrcAddr[2] == 'w')
	{
		char dominSrcAddr[100]={0};
		memset(dominSrcAddr,0,100);
		char* p = strstr(t_sSrcAddr,"/");
		if(p)
		{
			memcpy(dominSrcAddr,t_sSrcAddr,p-t_sSrcAddr);
			struct hostent* t_hostent = gethostbyname(dominSrcAddr);
			//获取第一个ip地址。
			memcpy(&saddr.sin_addr,t_hostent->h_addr,4);
		}
		saddr.sin_port = htons(port);
	}
	else 
	{
		t_sSrcAddr += strlen(LIVE_PRE);
		//解析类似 192.168.1.1:898或者192.168.1.1
		char* t_ipend = strstr(t_sSrcAddr,"/");
		if(!t_ipend)
		{
			printf("address(%s) is error\n"
					,m_SrcAddr);
			ret = -1;
			goto Init_Socket_End;
		}

		char* t_portstart = strstr(t_sSrcAddr,":");
		if(t_portstart && t_portstart < t_ipend)
		{
			char s_port[10]={0};
			memcpy(s_port,t_portstart+1,t_ipend-t_portstart);
			port = atoi(s_port);
			t_ipend = t_portstart;
		}

		char ipaddr[100]={0};
		memcpy(ipaddr,t_sSrcAddr,t_ipend-t_sSrcAddr);
		//    addrSrv.sin_addr.S_un.S_addr=inet_addr("127.0.0.1");
		printf("addr ip[%s],port:[%d]\n",ipaddr,port);
		saddr.sin_addr.s_addr = inet_addr(ipaddr);
		saddr.sin_port = htons(port);
	}

	unsigned long ul = 1;

	if((ioctlsocket(m_iSocket, FIONBIO, (unsigned long*)&ul)) == SOCKET_ERROR)
	{
		printf("no blocking connect error,errorcode:[%u]"
				,GetLastError());
		ret = -2;
		goto Init_Socket_End;
	}
	
	//客户端用select方式去连接，服务器用epoll
	if(connect(m_iSocket,(SOCKADDR *)&saddr,sizeof(saddr))!=0)  
    {  
        printf("Connect to server(%s) failed!,errorcode:[%u]\n"
				,m_SrcAddr,WSAGetLastError());  
		ret = -2;
       // goto Init_Socket_End; 
    
	
	//进行select调用。
		if(WSAGetLastError() == WSAEWOULDBLOCK )
		{
			struct timeval timeout;
			fd_set r;
			FD_ZERO(&r);
			FD_SET(m_iSocket,&r);
			timeout.tv_sec = TIMEOUT_SOCKET/1000;
			timeout.tv_usec = 0;
			ret = select(0,0,&r,0,&timeout);
			if(ret <=0)
			{
				printf("select socket failed,errorcode:[%u]\n"
					,WSAGetLastError());
				ret = -2;
				goto Init_Socket_End;
			}
			else
			{
				ret = 0;
			}
		}
	} 

Init_Socket_End:
	return ret;
}
int VA_Receive::set_init(const char* src_addr,int mode)
{
	//修改模式，通过地址来自动判断。
	if(strstr(src_addr,LIVE_PRE))
	{
		m_Mode = LIVE_MODE;
	}
	else
	{
		m_Mode = OFFLINE_MODE;
	}
	m_SrcAddr = (char*)calloc(sizeof(char),strlen(src_addr)+1);
	memcpy(m_SrcAddr,src_addr,strlen(src_addr));
	int ret = 0;

	if(m_Mode == LIVE_MODE)
	{
		ret = init_socket();
		if(0 != ret)
		{
			printf("init socket failed\n");
			goto Set_Init_End;
		}

		ret = get_httpstatus();
	}

Set_Init_End:
	return ret;
}

int VA_Receive::get_httpstatus()
{
	int ret = 0;
	char send_content[LINE_SIZE]={0};
	memset(send_content,0,LINE_SIZE);

	char* t_path = m_SrcAddr+strlen(LIVE_PRE);
	if(t_path)
	{
		t_path = strstr(t_path,"/");
	}

	sprintf_s(send_content,sizeof(send_content),"GET %s HTTP/1.1\r\nHOST: localhost\
												\r\nConnection: Keep-Alive\r\n\r\n",t_path);

	char recv_buf[LINE_SIZE*10]={0};
	memset(recv_buf,0,LINE_SIZE*10); 
	//int recv_len = LINE_SIZE*10;
	int recv_len = 300;

	ret = get_socketret(m_iSocket,send_content,strlen(send_content),recv_buf,recv_len);
	
	//返回的数据类似HTTP/1.1 200 OK 以\r\n\r\n为http开头的结束。所以可针对的进行处理。
	printf("rec_buf:[%s]\n",recv_buf);
	return ret;
}

int VA_Receive::get_socketret(int socket,char* send_buffer,int send_len,char* recv_buf,int recv_len)
{
	
	int send_aclen = send(socket,send_buffer,send_len,0);
	if(send_aclen != send_len)
	{
		printf("send content failed len(%d) != (%d)\n"
			,send_aclen,send_len);
		return -1;
	}

	ULONGLONG timestamp = GetTickCount64();
	int w_pos = 0;
	while(GetTickCount64() <= timestamp + TIMEOUT_SOCKET*3)
	{
		//运用select
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
			int ac_recvlen = recv(socket,recv_buf,recv_len - w_pos,0);
			if(w_pos+ac_recvlen >= recv_len)
			{
				break;
			}
		}
	}
	return 0;
}


int VA_Receive::rec_data(int src_len,unsigned char* src_content,bool* over_flag)
{
	//check 
	int ret_size = 0;
	if(m_Mode == OFFLINE_MODE )
	{
		if(m_Fp == NULL)
		{
			//图像处理需要用rb。
			fopen_s(&m_Fp,m_SrcAddr,"rb");
			if(!m_Fp)
			{
				printf("%s %s %d open file(%s) failed",
					__FILE__,__FUNCTION__,__LINE__,m_SrcAddr );
				return -1;
			}
		}
		
		ret_size = fread(src_content,sizeof(unsigned char),src_len,m_Fp);
			
		//printf("src:[%d],dst:[%d]\n",src_len,ret_size);
		
		if(feof(m_Fp))
		{
			*over_flag = true;
		}
	}

	return ret_size;
}
