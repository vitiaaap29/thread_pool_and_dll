#include "stdafx.h"
#include "copyFileInfo.h"
//������� ���������� � ������
void copyFile(void* context)
{
	CopyFilesInfo* cfi = new CopyFilesInfo(context);
	wprintf(TEXT("���������������� ����������� �� %s � %s\n", cfi->existingFileName,  cfi->newFileName));
	int result = CopyFile(cfi->existingFileName, cfi->newFileName, cfi->bFailIfExists);
	delete cfi;
}