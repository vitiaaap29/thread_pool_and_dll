#include "stdafx.h"
#include "copyFileInfo.h"
//функция исполнимая в потоке
void copyFile(void* context)
{
	CopyFilesInfo* cfi = new CopyFilesInfo(context);
	wprintf(TEXT("Непосредственное копирование из %s в %s\n", cfi->existingFileName,  cfi->newFileName));
	int result = CopyFile(cfi->existingFileName, cfi->newFileName, cfi->bFailIfExists);
	delete cfi;
}