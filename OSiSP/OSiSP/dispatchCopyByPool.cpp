#include "stdafx.h"
#include "Prototypes.h"
#include "copyFileInfo.h"

void dispatchCopyByPool(TCHAR* nameSource, TCHAR* destinationName, PFunction function)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	hFind = FindFirstFile(nameSource, &ffd);
	if (INVALID_HANDLE_VALUE == hFind) 
	{
		wprintf(TEXT("������ �����������\r\n"));
		return;
	} 

	WIN32_FIND_DATA ffdDestination;
	HANDLE hFindDestination = INVALID_HANDLE_VALUE;
	hFindDestination = FindFirstFile(nameSource, &ffdDestination);
	if (INVALID_HANDLE_VALUE == hFindDestination) 
	{
		wprintf(TEXT("������� - �� �����\r\n"));
		return;
	} 

	if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		//������� � �������� destinationName �����, ��������������� ��������� ����� 
		//nameSource � ��������� destinationName �� ��� ����� �����
		TCHAR* shortDirName = getShortName(nameSource);
		if (shortDirName != NULL)
		{
			LPWSTR nameNewDir = (LPWSTR)(&std::wstring(destinationName).append(L"\\")[0]);
			nameNewDir = lstrcatW(nameNewDir, (LPWSTR)shortDirName);
			destinationName = nameNewDir;

			if (CreateDirectory(destinationName, NULL))
			{
				wprintf(TEXT("������ ���������� %s � %s"),nameNewDir, destinationName );

				//�������� �� ���������� � �������� ����� � ����������-�������
				WIN32_FIND_DATA nextFfd;
				LPCWSTR nameNext = (LPCWSTR)(&std::wstring(nameSource).append(L"\\*")[0]);
				HANDLE handleNextFile = FindFirstFile(nameNext, &nextFfd);
				do{
					TCHAR* shortName = getShortName(nextFfd.cFileName);
					if (shortName != NULL)
					{
						std::wstring nameInsideDir = std::wstring(destinationName).append(L"\\");
						nameInsideDir += shortName;
						TCHAR* pnameInsideDir = &nameInsideDir[0];

						//����� ��������� �� ���� ��������
						if (nextFfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
						{
							dispatchCopyByPool(nextFfd.cFileName, pnameInsideDir, copyFile);
						}
						else
						{
							wprintf(TEXT(" �������� �����������\n"));
							TCHAR* pSource = &nextFfd.cFileName[0];
							CopyFilesInfo *cfi = new CopyFilesInfo(pSource, pnameInsideDir);
							function(cfi);
						}
					}
					else
					{
						wprintf(TEXT(" ������ ����������� ��������� ����� �����\n"));
					}
				}
				while (FindNextFile(hFind, &ffd) != 0);
			}
			else
			{
				wprintf(TEXT(" ������ �������� ����� ���������� � ����������-��������\n"));
				return;
			}
		}
		else
		{
			wprintf(TEXT(" ������ ����������� ��������� ����� �����\n"));
			return;
		} 
	}
	else
	{
		TCHAR* shortName = getShortName(nameSource);
		if (shortName != NULL)
		{
			wprintf(TEXT(" �������� �����������\n"));
			std::wstring nameNewFile = std::wstring(destinationName).append(L"\\");
			nameNewFile += std::wstring(shortName);
			TCHAR* pname = &nameNewFile[0];

			CopyFilesInfo *cfi = new CopyFilesInfo(nameSource, pname);
			function(cfi);
		}
	}
}

