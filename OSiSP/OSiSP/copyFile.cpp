#include "stdafx.h"
#include "copyFileInfo.h"
//функция исполнимая в потоке
void copyFile(void* context)
{
	CopyFilesInfo* cfi = (CopyFilesInfo*)context;
	wprintf(L"Непосредственное копирование из %s в %s\n", cfi->existingFileName,  cfi->newFileName);
	int result = CopyFile(cfi->existingFileName, cfi->newFileName, cfi->bFailIfExists);
	printf("result = %d", result);
	delete cfi;
}