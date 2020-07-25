#include "base.h"

FILE* g_LogFile;
const char* g_LogFileName;

void LogInit(const char* name)
{
	g_LogFileName = name;
	g_LogFile = fopen(g_LogFileName, "wb");
	ASSERT(g_LogFile);
}

struct Logger
{
	~Logger() {
		fclose(g_LogFile);
	}
};

static Logger g_Logger;
