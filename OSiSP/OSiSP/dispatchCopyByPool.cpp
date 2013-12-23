#include "stdafx.h"
#include "copyFileInfo.h"

void dispatchCopyByPool(TCHAR* nameSource, TCHAR* destinationName)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	hFind = FindFirstFile(nameSource, &ffd);
	if (INVALID_HANDLE_VALUE == hFind) 
	{
		std::cout << _T("\r\nОшибка копирования\r\n") << std::endl;
		return;
	} 

	WIN32_FIND_DATA ffdDestination;
	HANDLE hFindDestination = INVALID_HANDLE_VALUE;
	hFindDestination = FindFirstFile(nameSource, &ffdDestination);
	if (INVALID_HANDLE_VALUE == hFindDestination) 
	{
		std::cout << _T("Приёмник - не папка\r\n") << std::endl;
		return;
	} 


	if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		//создать в каталоге destinationName папку, соответствующую короткому имени 
		//nameSource и нарастить destinationName на эту новую папку
		TCHAR* shortDirName = getShortName(nameSource);
		if (shortDirName != NULL)
		{
			LPWSTR nameNewDir = (LPWSTR)(&std::wstring(destinationName).append(L"\\")[0]);
			nameNewDir = lstrcatW(nameNewDir, (LPWSTR)shortDirName);
			destinationName = nameNewDir;

			if (CreateDirectory(destinationName, NULL))
			{
				wprintf(TEXT("Создаём подкаталог %s в %s"),nameNewDir, destinationName );

				//проходим по директории и копируем файлы в директорию-приёмник
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

						//можно упростить за счёт рекурсии
						if (nextFfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
						{
							dispatchCopyByPool(nextFfd.cFileName, pnameInsideDir);
						}
						else
						{
							wprintf(TEXT(" Начинаем копирование\n"));
							TCHAR* pSource = &nextFfd.cFileName[0];
							CopyFilesInfo *cfi = new CopyFilesInfo(pSource, pnameInsideDir);
						}
					}
					else
					{
						wprintf(TEXT(" Ошибка определение короткого имени файла\n"));
					}
				}
				while (FindNextFile(hFind, &ffd) != 0);
			}
			else
			{
				wprintf(TEXT(" Ошибка создания новой директории в директории-приёмнике\n"));
				return;
			}
		}
		else
		{
			wprintf(TEXT(" Ошибка определение короткого имени файла\n"));
			return;
		} 
	}
	else
	{
		TCHAR* shortName = getShortName(nameSource);
		if (shortName != NULL)
		{
			wprintf(TEXT(" Начинаем копирование\n"));
			std::wstring nameNewFile = std::wstring(destinationName).append(L"\\");
			nameNewFile += std::wstring(shortName);
			TCHAR* pname = &nameNewFile[0];

			CopyFilesInfo *cfi = new CopyFilesInfo(nameSource, pname);
		}
	}
}