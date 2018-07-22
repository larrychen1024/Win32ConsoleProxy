#pragma once
#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <stdio.h> 
#include <strsafe.h>
#include <iostream>
#include <string>
using namespace std;
#define CMD_POSTFIX "\r\n\0"
class Console
{
public:
	Console();
	~Console();
	BOOL CreateChildProcess();
	BOOL Write(const CHAR * msg, DWORD msgLength);
	BOOL Read(CHAR * buff, DWORD buffSize);
private:
	HANDLE m_hChildStd_IN_Rd;
	HANDLE m_hChildStd_IN_Wr;
	HANDLE m_hChildStd_OUT_Rd;
	HANDLE m_hChildStd_OUT_Wr;
	HANDLE m_hChildProcess;
};

