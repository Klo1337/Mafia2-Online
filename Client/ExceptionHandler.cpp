// Copyright (C) 2016 by M2O Team

#include <Windows.h>
#include <stdio.h>
#include <Dbghelp.h>

#include "ExceptionHandler.h"

char minidumpsPath[MAX_PATH + 1] = { 0 };

inline LONG GetFilterReturnCode(void)
{
#ifdef _DEBUG
	if (IsDebuggerPresent())
		return EXCEPTION_CONTINUE_SEARCH;
#endif
	return EXCEPTION_EXECUTE_HANDLER;
}

static LONG WINAPI ExceptionFilter(PEXCEPTION_POINTERS exceptionInfo)
{
	const BOOL createFolderResult = CreateDirectory(minidumpsPath, NULL);
	DWORD lastError = GetLastError();
	if (!createFolderResult && (lastError != ERROR_ALREADY_EXISTS)) {
		char message[512] = { 0 };
		sprintf(message, "Multiplayer mod has crashed. Unfortunately due to some problem the creation of dumps folder failed, try running mod as administrator.\nIf it does not help please send bug report with screenshot of this dialog on http://bugs.mafia2-online.com\nError code: %X", lastError);
		MessageBox(NULL, message, "Mafia2-Online has crashed!", MB_ICONERROR);
		return GetFilterReturnCode();
	}

	char dumpFileName[32] = { 0 };

	SYSTEMTIME t;
	GetSystemTime(&t);
	sprintf(dumpFileName,"M2O_%02u_%02u_%04u_%02u_%02u_%02u", t.wDay, t.wMonth, t.wYear, t.wHour, t.wMinute, t.wSecond);

	char exceptionSavePath[MAX_PATH + 1];
	sprintf(exceptionSavePath, "%s\\%s.dmp", minidumpsPath, dumpFileName);

	const HANDLE file = CreateFile(exceptionSavePath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	lastError = GetLastError();
	if (!file) {
		char message[512] = { 0 };
		sprintf(message, "Multiplayer mod has crashed. Unfortunately due to some problem the creation of dump file failed, try running mod as administrator.\nIf it does not help please send bug report with screenshot of this dialog on http://bugs.mafia2-online.com\nError code: %X", lastError);
		MessageBox(NULL, message, "Mafia2-Online has crashed!", MB_ICONERROR);
		return GetFilterReturnCode();
	}

	MINIDUMP_EXCEPTION_INFORMATION info;
	info.ThreadId = GetCurrentThreadId();
	info.ExceptionPointers = exceptionInfo;
	info.ClientPointers = FALSE;

	const BOOL miniDumpWriteResult = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file, MINIDUMP_TYPE(MiniDumpNormal), &info, NULL, NULL);
	lastError = GetLastError();
	if (!miniDumpWriteResult) {
		char message[512] = { 0 };
		sprintf(message, "Multiplayer mod has crashed. Unfortunately due to some problem the write of dump file failed, try running mod as administrator.\nIf it does not help please send bug report with screenshot of this dialog on http://bugs.mafia2-online.com\nError code: %X", lastError);
		MessageBox(NULL, message, "Mafia2-Online has crashed!", MB_ICONERROR);

		CloseHandle(file);
		return GetFilterReturnCode();
	}

	CloseHandle(file);

	char message[512] = { 0 };
	sprintf(message, "Multiplayer mod has crashed. This is serious problem mostly caused by some bug in the code.\nPlease report it on http://bugs.mafia2-online.com and attach this file:\n%s", exceptionSavePath);
	MessageBox(NULL, message, "Mafia2-Online has crashed!", MB_ICONERROR);

	return GetFilterReturnCode();
}

bool ExceptionHandler::Install(const char *const path)
{
	if (!path || strlen(path) > MAX_PATH)
		return false;

	strncpy(minidumpsPath, path, MAX_PATH);

	SetUnhandledExceptionFilter(ExceptionFilter);
	return true;
}