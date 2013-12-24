#include "stdafx.h"
#include "CopyFileInfo.h"
#define MAX_PATH_LENGTH 260

CopyFilesInfo::CopyFilesInfo(TCHAR* existingFileName, TCHAR* newFileName, bool flag)
{
	bFailIfExists = flag;
	this->existingFileName = new TCHAR[MAX_PATH_LENGTH];
	this->newFileName = new TCHAR[MAX_PATH_LENGTH];
	wcscpy_s(this->existingFileName, MAX_PATH_LENGTH, existingFileName);
	wcscpy_s(this->newFileName, MAX_PATH_LENGTH, newFileName);
}

CopyFilesInfo::CopyFilesInfo(void* context)
{
	this->existingFileName = new TCHAR[MAX_PATH_LENGTH];
	this->newFileName = new TCHAR[MAX_PATH_LENGTH];
	wcscpy_s(this->existingFileName, MAX_PATH_LENGTH, (reinterpret_cast<CopyFilesInfo*>(context))->existingFileName);
	wcscpy_s(this->newFileName, MAX_PATH_LENGTH, (reinterpret_cast<CopyFilesInfo*>(context))->newFileName);
	bFailIfExists = reinterpret_cast<CopyFilesInfo*>(context)->bFailIfExists;
}
