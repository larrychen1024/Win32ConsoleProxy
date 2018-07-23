#include "stdafx.h"
#include "Console.h"


Console::Console()
{
	m_hChildStd_IN_Rd = NULL;
	m_hChildStd_IN_Wr = NULL;
	m_hChildStd_OUT_Rd = NULL;
	m_hChildStd_OUT_Wr = NULL;
	m_hChildProcess = NULL;
}


Console::~Console()
{
	CloseHandle(m_hChildStd_IN_Rd);
	CloseHandle(m_hChildStd_IN_Wr);
	CloseHandle(m_hChildStd_OUT_Rd);
	CloseHandle(m_hChildStd_OUT_Wr);
	CloseHandle(m_hChildProcess);
}

BOOL Console::CreateChildProcess() 
{
	printf("\n->create pipe ...\n");
	SECURITY_ATTRIBUTES saAttr;
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
	siStartInfo.wShowWindow = SW_SHOW; //SW_HIDE
	siStartInfo.hStdError = m_hChildStd_OUT_Wr;
	siStartInfo.hStdOutput = m_hChildStd_OUT_Wr;
	siStartInfo.hStdInput = m_hChildStd_IN_Rd;
	siStartInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW; //STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW

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
		// CloseHandle(piProcInfo.hProcess);
		// CloseHandle(piProcInfo.hThread);
	}
	m_hChildProcess = piProcInfo.hProcess;
	return TRUE;
}

BOOL Console::Write(const CHAR * msg, DWORD msgLength) 
{
	/*
	DWORD bufSize = msgLength + sizeof(CMD_POSTFIX);				// 换行符+回车符+NULL字符
	CHAR * buff = (CHAR *)malloc(bufSize);
	ZeroMemory(buff, bufSize);
	sprintf_s(buff, bufSize, "%s%s", msg, CMD_POSTFIX);	// windows命令以换号+回车结束
	
	DWORD dwWritten;
	BOOL bSuccess = TRUE;
	bSuccess = WriteFile(m_hChildStd_IN_Wr, buff, strlen(buff), &dwWritten, NULL);
	
	free(buff);
	return bSuccess;
	*/
	DWORD dwWritten;
	BOOL bSuccess = TRUE;
	bSuccess = WriteFile(m_hChildStd_IN_Wr, msg, msgLength, &dwWritten, NULL);
	return bSuccess;
}

BOOL Console::Read(CHAR * buff, DWORD buffSize)
{
	BOOL bSuccess;
	DWORD dwRead;
	DWORD dwOffset = 0;
	ZeroMemory(buff, buffSize);
	while (TRUE)
	{
		Sleep(100);

		bSuccess = ReadFile(m_hChildStd_OUT_Rd, buff + dwOffset, buffSize, &dwRead, NULL);
		if ( !bSuccess )
		{
			break;
		}
		
		if ( dwRead > 1 && buff[dwRead - 1] == '>' )
		{
			break;
		}

		dwOffset += dwRead;
	}

	return bSuccess;
}