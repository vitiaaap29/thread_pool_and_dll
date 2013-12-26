#include "stdafx.h"
#include "Prototypes.h"
#include "sizeFileInfo.h"
#include "Pool.h"

CRITICAL_SECTION cs;

void dispatchSizeByPool(TCHAR* name, float* sum, PFunction function, Pool* pool)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	hFind = FindFirstFile(name, &ffd);
	if (INVALID_HANDLE_VALUE == hFind) 
	{
		wprintf(TEXT("Ошибка поиска файла\r\n"));
		return;
	} 

	if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		WIN32_FIND_DATA nextFfd;
		std::wstring* nextFile = new std::wstring(name);
		nextFile->append(L"\\*");
		LPWSTR nameNext = (LPWSTR)&(nextFile->c_str()[0]);
		HANDLE handleNextFile = FindFirstFile(nameNext, &nextFfd);
		do{
			if (nextFfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				dispatchSizeByPool(nameNext, sum, function, pool);
			}
			else
			{
				SizeFileInfo *sfi = new SizeFileInfo(ffd, sum, &cs);
				Pool::WorkItem work(function, (void*)sfi);
				pool->addWorkToQueue(work);
			}
		}while (FindNextFile(handleNextFile, &nextFfd) != 0);
	}
	else
	{
		SizeFileInfo *sfi = new SizeFileInfo(ffd, sum, &cs);
		Pool::WorkItem work(function, (void*)sfi);
		pool->addWorkToQueue(work);
	}
}