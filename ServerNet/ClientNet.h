#pragma once
#include<stdio.h>
#include<windows.h>
#include <iostream>
#pragma comment(lib, "Ws2_32.lib")
#define BUF_SIZE 4096
class ClientNet
{
public:
	ClientNet();
	~ClientNet();

	// 连接指定的服务器
	int ClientConnect(const char* address, int port);
	// 发送信息
	int ClientSend(const char* msg, int len);
	// 接收信息
	int ClientRecv(PCHAR msg);
	// 关闭连接
	void ClientClose();

private:
	SOCKET m_sock;
	
};

