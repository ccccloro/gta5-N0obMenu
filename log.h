#pragma once
#include <stdio.h>
#include <Windows.h>

#define LOG_BUFFER_SIZE 128

typedef enum LOGTYPE
{
    BLACK = 0,
    DARKBLUE = FOREGROUND_BLUE,
    DARKGREEN = FOREGROUND_GREEN,
    DARKCYAN = FOREGROUND_GREEN | FOREGROUND_BLUE,
    DARKRED = FOREGROUND_RED,
    DARKMAGENTA = FOREGROUND_RED | FOREGROUND_BLUE,
    DARKYELLOW = FOREGROUND_RED | FOREGROUND_GREEN,
    DARKGRAY = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
    GRAY = FOREGROUND_INTENSITY,
    BLUE = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
    GREEN = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
    CYAN = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE,
    RED = FOREGROUND_INTENSITY | FOREGROUND_RED,
    MAGENTA = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE,
    YELLOW = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
    WHITE = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
} LOGTYPE;

class cmd
{
private:

	static __forceinline VOID SetTextColor(DWORD color)
	{
		CONSOLE_SCREEN_BUFFER_INFO buffer;
		HANDLE outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		GetConsoleScreenBufferInfo(outputHandle, &buffer);

		WORD attributes = buffer.wAttributes & ~FOREGROUND_RED & ~FOREGROUND_GREEN & ~FOREGROUND_BLUE & ~FOREGROUND_INTENSITY;
		attributes |= color;

		SetConsoleTextAttribute(outputHandle, attributes);
	}

public:

	static VOID write(LOGTYPE logtype, LPCSTR format, ...)
	{
		char buff[LOG_BUFFER_SIZE];

		va_list list;
		va_start(list, format);
		vsprintf_s(buff, format, list);
		va_end(list);

		//then print message
		SetTextColor(logtype);
		char buff2[LOG_BUFFER_SIZE];
		sprintf_s(buff2, "%s\n", buff);
		printf(buff2);
	}

	static VOID write_nr(LOGTYPE logtype, LPCSTR format, ...)
	{
		char buff[LOG_BUFFER_SIZE];

		va_list list;
		va_start(list, format);
		vsprintf_s(buff, format, list);
		va_end(list);

		SetTextColor(logtype);
		char buff2[LOG_BUFFER_SIZE];

		sprintf_s(buff2, "%s", buff);
		printf(buff2);
	}

	static VOID write_inline(LOGTYPE logtype, LPCSTR format, ...)
	{
		char buff[LOG_BUFFER_SIZE];

		va_list list;
		va_start(list, format);
		vsprintf_s(buff, format, list);
		va_end(list);

		//then print message
		SetTextColor(logtype);
		char buff2[LOG_BUFFER_SIZE];

		sprintf_s(buff2, "%s\n", buff);
		printf(buff2);
	}
};