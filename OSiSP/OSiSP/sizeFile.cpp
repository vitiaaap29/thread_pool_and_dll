#include "stdafx.h"
#include "sizeFileInfo.h"

void sizeFile(void* context)
{
	SizeFileInfo* sizeFileInfo = (SizeFileInfo*)context;
	WIN32_FIND_DATA ffd = sizeFileInfo->ffd;

	// List all the files in the directory with some info about them.
	LARGE_INTEGER filesize;
	filesize.LowPart = ffd.nFileSizeLow;
	filesize.HighPart = ffd.nFileSizeHigh;

	EnterCriticalSection(sizeFileInfo->incSizeCriticalSection);
	float temp = *(sizeFileInfo->sum); 
	*(sizeFileInfo->sum) = temp + filesize.QuadPart;
	LeaveCriticalSection(sizeFileInfo->incSizeCriticalSection);
}