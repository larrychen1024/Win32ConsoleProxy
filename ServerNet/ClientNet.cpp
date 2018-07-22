#include "stdafx.h"
#include "ClientNet.h"

ClientNet::ClientNet()
{
}


ClientNet::~ClientNet()
{
}

/*客户端Socket连接*/
int ClientNet::ClientConnect(const char* address, int port)
{
	int rlt = 0;   //connectflag  0-success 1-WSAStartfailed 2-socketfailed 3-connectfailed

	// 记录错误信息
	int iErrMsg;
	// 启动WinSock
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(1, 1);
	iErrMsg = WSAStartup(wVersionRequested, &wsaData);
	if (iErrMsg != NO_ERROR) // WSAStartup出现了错误
	{
		printf("failed with WSAStartup error: %d\n", iErrMsg);
		rlt = 1;
		return rlt;
	}

	// 创建Socket
	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sock == INVALID_SOCKET) // socket出现了错误
	{
		printf("failed with socket error: %d\n", WSAGetLastError());
		rlt = 2;
		return rlt;
	}


	// 目标服务器数据
	SOCKADDR_IN servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = port;
	servaddr.sin_addr.s_addr = inet_addr(address);

	// sock与目标服务器连接
	iErrMsg = connect(m_sock, (SOCKADDR*)&servaddr, sizeof(servaddr));
	if (iErrMsg != NO_ERROR)
	{
		printf("failed with connect error: %d\n", iErrMsg);
		rlt = 3;
		return rlt;
	}

	// success
	return rlt;

}

/*客户端发送消息*/
int ClientNet::ClientSend(const char* msg, int len)
{
	int rlt = 0;

	int iErrMsg = 0;
	int bufSize = len + 3 + 2; // 换行符+回车符+NULL字符
	char * buf = (char *)malloc(bufSize);
		
	ZeroMemory(buf, bufSize);
	// windows命令以换号回车结束
	sprintf_s(buf, bufSize, "%s\r\n", msg);
	
	// 指定sock发送消息
	iErrMsg = send(m_sock, buf, bufSize, 0);
	if (iErrMsg < 0)// 发送失败
	{
		printf("send msg failed with error: %d\n", iErrMsg);
		rlt = 1;
	}
	else
	{
		//printf("send msg successfully\n");
		//
	}
	
	free(buf);
	
	return rlt;
}
int ClientNet::ClientRecv(PCHAR msg)
{
	int rlt = 0;

	int iErrMsg = 0;

	// 指定sock发送消息
	iErrMsg = recv(m_sock, msg, BUF_SIZE, 0);
	if (iErrMsg < 0)// 发送失败
	{
		printf("recv msg failed with error: %d\n", iErrMsg);
		rlt = 1;
		return rlt;
	}
	//printf("recv msg successfully\n");
	return rlt;
}

/*客户端关闭Socket*/
void ClientNet::ClientClose()
{
	closesocket(m_sock);
}