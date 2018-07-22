#include "stdafx.h"
#include "ServerNet.h"

ServerNet::ServerNet() {
	m_hChildStd_IN_Rd = NULL;
	m_hChildStd_IN_Wr = NULL;
	m_hChildStd_OUT_Rd = NULL;
	m_hChildStd_OUT_Wr = NULL;
	m_hChildProcess = NULL;
}

ServerNet::~ServerNet() {
	CloseHandle(m_hChildStd_IN_Rd);
	CloseHandle(m_hChildStd_IN_Wr);
	CloseHandle(m_hChildStd_OUT_Rd);
	CloseHandle(m_hChildStd_OUT_Wr);
	CloseHandle(m_hChildProcess);
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
			printf("new socket connect: %d\n", newSocket);
			bSuccess = CreateChildProcess();					// 创建子进程
			if (!bSuccess)
			{
				return ErrorExit(" CreateChildProcess failed. \r\n");
			}
						
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
					printf("socket %d connect end.\n", newSocket);
				}
				else													// 显示接收到的数据  
				{
					//printf("%s", buff);
					
					bSuccess = WriteToPipe(buff, strlen(buff)); // 写入时不需要带上NULL字符
					if (!bSuccess)
					{
						return ErrorExit(" WriteToPipe failed. \r\n");
					}
										
					bSuccess = ReadFromPipe(buff, sizeof(buff));
					if (!bSuccess)
					{
						return ErrorExit("ReadFromPipe failed. \r\n");
					}
					
					printf("%s", buff);
					send(newSocket, buff, strlen(buff), NULL);
				}
				
			} while (rval != 0);

			closesocket(newSocket); // 关闭接收的socket  
		}
	} while (1);

	// 关闭自身socket  
	//closesocket(m_sock);
}


// Create the child process. 
// Create a child process that uses the previously created pipes for STDIN and STDOUT.
BOOL ServerNet::CreateChildProcess() 
{
	SECURITY_ATTRIBUTES saAttr;

	printf("\n->create pipe ...\n");
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	if (!CreatePipe(&m_hChildStd_OUT_Rd, &m_hChildStd_OUT_Wr, &saAttr, 0))
	{
		return FALSE;
	}

	if (!SetHandleInformation(m_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
	{
		return FALSE;
	}

	if (!CreatePipe(&m_hChildStd_IN_Rd, &m_hChildStd_IN_Wr, &saAttr, 0))
	{
		return FALSE;
	}

	if (!SetHandleInformation(m_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
	{
		return FALSE;
	}

	printf("\n->create child process ...\n");
	
	STARTUPINFO siStartInfo;
	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.wShowWindow = SW_HIDE;
	siStartInfo.hStdError = m_hChildStd_OUT_Wr;
	siStartInfo.hStdOutput = m_hChildStd_OUT_Wr;
	siStartInfo.hStdInput = m_hChildStd_IN_Rd;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

	BOOL bSuccess = FALSE;
	PROCESS_INFORMATION piProcInfo;
	TCHAR childProcess[] = TEXT("C:\\Windows\\SysWOW64\\cmd.exe");
	bSuccess = CreateProcess(childProcess,
		NULL,				// command line 
		NULL,				// process security attributes 
		NULL,				// primary thread security attributes 
		TRUE,				// handles are inherited 
		0,					// creation flags 
		NULL,				// use parent's environment 
		NULL,				// use parent's current directory 
		&siStartInfo,		// STARTUPINFO pointer 
		&piProcInfo);		// receives PROCESS_INFORMATION 

	if (!bSuccess)
	{
		return FALSE;
	}
	else
	{
		// Close handles to the child process and its primary thread.
		// Some applications might keep these handles to monitor the status
		// of the child process, for example. 
		//CloseHandle(piProcInfo.hProcess);
		//CloseHandle(piProcInfo.hThread);
	}
	m_hChildProcess = piProcInfo.hProcess;
	return TRUE;
}

// Read from a file and write its contents to the pipe for the child's STDIN.
// Stop when there is no more data. 
BOOL ServerNet::WriteToPipe(CHAR * msg, DWORD msgLength)
{
	DWORD dwWritten;
	BOOL bSuccess = TRUE;

	bSuccess = WriteFile(m_hChildStd_IN_Wr, msg, msgLength, &dwWritten, NULL);

	return bSuccess;
}

// Read output from the child process's pipe for STDOUT
// and write to the parent process's pipe for STDOUT. 
// Stop when there is no more data. 
BOOL ServerNet::ReadFromPipe(CHAR * buff, DWORD buffSize)
{
	DWORD dwRead;
	BOOL bSuccess = FALSE;
	/*
	CHAR * buff = new char[PIPE_BUFSIZE];
	while (TRUE) 
	{
		Sleep(100);
		ZeroMemory(buff, sizeof(buff));
		bSuccess = ReadFile(m_hChildStd_OUT_Rd, buff, PIPE_BUFSIZE, &dwRead, NULL);
		if (!bSuccess) 
		{
			delete buff;
			return FALSE;
		}

		*((CHAR *)buff + dwRead)= '\0';
		msg += buff;
		if (buff[dwRead] == '>') 
		{
			delete buff;
			return TRUE;
		}


	}
	*/

	//管道是否有数据可读
	/*
	while (PeekNamedPipe(m_hChildStd_OUT_Rd, msg, PIPE_BUFSIZE, &dwRead, NULL, NULL))
	{	
		if (dwRead)
		{
			ZeroMemory(&msg, PIPE_BUFSIZE);
			bSuccess = ReadFile(m_hChildStd_OUT_Rd, msg, PIPE_BUFSIZE, &dwRead, NULL);
			if (!bSuccess || dwRead == 0)
			{
				break;
			}
			return TRUE;
		}
	}
	return FALSE;
	*/
	//bSuccess = ReadFile(m_hChildStd_OUT_Rd, msg, PIPE_BUFSIZE, &dwRead, NULL);

	ZeroMemory(buff, buffSize);
	// bSuccess = PeekNamedPipe(m_hChildStd_OUT_Rd, buff, buffSize, &dwRead, NULL, NULL);
	bSuccess =  ReadFile(m_hChildStd_OUT_Rd, buff, buffSize, &dwRead, NULL);
	if (!bSuccess || dwRead == 0)
	{
		return FALSE;
	}
	return TRUE;
	
}

void emptyPipe(HANDLE hReadPipe1) {
	BOOL ret;
	DWORD bytesRead;
	char *buffer = new char[1024];
	while (true)
	{
		memset(buffer, 0, 1024);
		ret = PeekNamedPipe(hReadPipe1, buffer, 1024, &bytesRead, 0, 0);
		if (bytesRead == 0 || !ret)
		{
			delete buffer;
			return;
		}
		ReadFile(hReadPipe1, buffer, bytesRead, &bytesRead, 0);
	}
}


DWORD ServerNet::ErrorExit(LPCTSTR lpszFunction)

// Format a readable error message, display a message box, 
// and exit from the application.
{
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0, NULL);

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(dw);
	return 1;
}