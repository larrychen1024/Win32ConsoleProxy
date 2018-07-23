#include "stdafx.h"
#include "ServerNet.h"

ServerNet::ServerNet() {

}

ServerNet::~ServerNet() {

	closesocket(m_sock);
}

int ServerNet::ServerInit(const char* address, int port)
{
	int rlt = 0;
	int iErrorMsg;
		
	WSAData wsaData;
	iErrorMsg = WSAStartup(MAKEWORD(1, 1), &wsaData); //初始化WinSock  

	if (iErrorMsg != NO_ERROR)//初始化WinSock失败  
	{
		printf("server wsastartup failed with error : %d\n", iErrorMsg);
		rlt = 1;
		return rlt;
	}
	
	m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // 创建服务器端socket  
	if (m_sock == INVALID_SOCKET)// 创建socket出现了异常  
	{
		printf("server socket failed with error: %d\n", WSAGetLastError());
		rlt = 2;
		return rlt;
	}

	SOCKADDR_IN servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = port;
	servaddr.sin_addr.s_addr = inet_addr(address);
		
	iErrorMsg = bind(m_sock, (SOCKADDR*)&servaddr, sizeof(servaddr)); //绑定  
	if (iErrorMsg < 0) //绑定失败  
	{		
		printf("bind failed with error : %d\n", iErrorMsg);
		rlt = 3;
		return rlt;
	}

	iErrorMsg = m_console.CreateChildProcess();
	if ( !iErrorMsg )
	{
		printf("CreateChildProcess Failed.");
	}
	return rlt;
}

DWORD ServerNet::ServerRun()
{	
	CHAR buff[PIPE_BUFSIZE];
	string msg;
	SOCKADDR_IN tcpAddr;
	int len = sizeof(sockaddr);
	SOCKET newSocket;
	int rval = TRUE;
	BOOL bSuccess;
	
	listen(m_sock, 5); // 开始侦听指定端口
	
	do
	{		
		newSocket = accept(m_sock, (sockaddr*)&tcpAddr, &len);	// 接收信息  
		if (newSocket == INVALID_SOCKET)						// 非可用socket  
		{
			printf("invalid socket occured.\n");
		}
		else													// 可用的新socket连接  
		{			
			printf("new socket connect: %lld\n", newSocket);

						
			do															// 消息处理  
			{			
				rval = recv(newSocket, buff, sizeof(buff), NULL);		// 接收数据 
				
				if (rval == SOCKET_ERROR)								// 该异常通常发生在未closeSocket就退出时  
				{
					printf("recv socket error.\n");
					break;
				}

				else if (rval == 0)										// 0表示正常退出  
				{
					printf("socket %lld connect end.\n", newSocket);
				}
				else													// 显示接收到的数据  
				{
					if (!m_console.Write(buff, strlen(buff))) 
					{
						printf("%s:[%d] write failed. \r\n", __FILE__, __LINE__);
					}

					printf("%s", buff);

					if (!m_console.Read(buff, sizeof(buff)))
					{
						printf("%s:[%d] Read failed. \r\n", __FILE__, __LINE__);
					}

					send(newSocket, buff, strlen(buff), NULL);

					printf("%s", buff);
				}
				
			} while (rval != 0);

			closesocket(newSocket); // 关闭接收的socket  
		}
	} while (1);

	// 关闭自身socket  
	//closesocket(m_sock);
}
