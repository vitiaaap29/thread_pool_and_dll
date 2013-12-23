#include "stdafx.h"

TCHAR* getShortName(TCHAR* fullName)
{
	int size = std::wstring(fullName).size();
	int i;
	for (i = size - 1; i > 0 && fullName[i] != '\\'; i--);

	int newSize = size - i;
	TCHAR* result = new TCHAR[newSize];
	for (int j = 0; j < newSize; j++)
	{
		result[j] = fullName[i + j + 1];
	}
	return result;
}