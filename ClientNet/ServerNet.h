#pragma once

#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <stdio.h> 
#include <strsafe.h>
#include <iostream>
#include <string>
#pragma comment(lib, "Ws2_32.lib")
#define SOCKET_BUFSIZE 4096
#define PIPE_BUFSIZE 4096
using namespace std;
class ServerNet
{

public:
	ServerNet();
	~ServerNet();
	//  初始化服务器
	int ServerInit(const char* address, int port);

	// 更新数据
	DWORD ServerRun();
	BOOL CreateChildProcess();
	BOOL ReadFromPipe(CHAR * buff, DWORD buffSize);
	BOOL WriteToPipe(CHAR * msg, DWORD msgLength);

	DWORD ErrorExit(LPCTSTR lpszFunction);
private:
	SOCKET m_sock;
	HANDLE m_hChildStd_IN_Rd;
	HANDLE m_hChildStd_IN_Wr;
	HANDLE m_hChildStd_OUT_Rd;
	HANDLE m_hChildStd_OUT_Wr;
	HANDLE m_hChildProcess;
};