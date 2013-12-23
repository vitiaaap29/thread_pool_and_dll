#include "stdafx.h"

struct CopyFilesInfo
{
	TCHAR* existingFileName;
	TCHAR* newFileName;
	bool bFailIfExists;
	CopyFilesInfo(TCHAR* existingFileName, TCHAR* newFileName, BOOL flag = false);
	~CopyFilesInfo();
};

CopyFilesInfo::CopyFilesInfo(TCHAR* existingFileName, TCHAR* newFileName, BOOL flag)
{
	bFailIfExists = flag;
	this->existingFileName = new TCHAR[MAX_PATH];
	this->newFileName = new TCHAR[MAX_PATH];
	wcscpy_s(this->existingFileName, MAX_PATH, existingFileName);
	wcscpy_s(this->newFileName, MAX_PATH, newFileName);
}