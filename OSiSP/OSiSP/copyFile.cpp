#include "stdafx.h"
#include "copyFileInfo.h"
//������� ���������� � ������
void copyFile(void* context)
{
	CopyFilesInfo* cfi = (CopyFilesInfo*)context;
	wprintf(L"���������������� ����������� �� %s � %s\n", cfi->existingFileName,  cfi->newFileName);
	int result = CopyFile(cfi->existingFileName, cfi->newFileName, cfi->bFailIfExists);
	printf("result = %d", result);
	delete cfi;
}