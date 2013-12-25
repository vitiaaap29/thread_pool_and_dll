#include "stdafx.h"
#include "Prototypes.h"
#include "copyFileInfo.h"
#include "Pool.h"

void dispatchCopyByPool(TCHAR* nameSource, TCHAR* destinationName, PFunction function, Pool* pool)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	hFind = FindFirstFile(nameSource, &ffd);
	if (INVALID_HANDLE_VALUE == hFind) 
	{
		wprintf(TEXT("Ошибка копирования\r\n"));
		return;
	} 

	WIN32_FIND_DATA ffdDestination;
	HANDLE hFindDestination = INVALID_HANDLE_VALUE;
	hFindDestination = FindFirstFile(destinationName, &ffdDestination);
	if (INVALID_HANDLE_VALUE == hFindDestination) 
	{
		wprintf(TEXT("Приёмник - не папка\r\n"));
		return;
	} 

	if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		//создать в каталоге destinationName папку, соответствующую короткому имени 
		//nameSource и нарастить destinationName на эту новую папку
		TCHAR* shortDirName = getShortName(nameSource);
		if (shortDirName != NULL)
		{
			//if (CreateDirectory(destinationName, NULL);
			std::wstring *nd = new std::wstring(destinationName);
			nd->append(L"\\");
			nd->append(shortDirName);
			destinationName = (TCHAR*)&(nd->c_str()[0]);
			wprintf(L"Имя нового каталога %s\n", destinationName );

			if (CreateDirectory(destinationName, NULL))
			{
				wprintf(L"Создаём подкаталог %s в %s",shortDirName, destinationName );

				//проходим по директории и копируем файлы в директорию-приёмник
				WIN32_FIND_DATA nextFfd;
				std::wstring* nextFile = new std::wstring(nameSource);
				nextFile->append(L"\\*");
				LPCWSTR nameNext = (LPCWSTR)&(nextFile->c_str()[0]);
				HANDLE handleNextFile = FindFirstFile(nameNext, &nextFfd);

				if (lstrcmpW(L".", nextFfd.cFileName) != 0 && lstrcmpW(L"..", nextFfd.cFileName) != 0)
				{
					do{
						TCHAR* shortName = getShortName(nextFfd.cFileName);
						if (shortName != NULL)
						{
							std::wstring *nameInsideDir = new std::wstring(destinationName);
							nameInsideDir->append(L"\\");
							nameInsideDir->append(shortName);
							TCHAR* pnameInsideDir = (TCHAR*)&(nameInsideDir->c_str()[0]);

							//можно упростить за счёт рекурсии
							if (nextFfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
							{
								dispatchCopyByPool(nextFfd.cFileName, pnameInsideDir, copyFile, pool);
							}
							else
							{
								wprintf(TEXT(" Начинаем копирование\n"));
								TCHAR* pSource = &nextFfd.cFileName[0];
								CopyFilesInfo *cfi = new CopyFilesInfo(pSource, pnameInsideDir);
								Pool::WorkItem work(function, cfi);
								pool->addWorkToQueue(work);
							}
						}
						else
						{
							wprintf(TEXT(" Ошибка определение короткого имени файла\n"));
						}
					}while (FindNextFile(hFind, &ffd) != 0);
				}
			}
			else
			{
				DWORD codeError = GetLastError();
				if (codeError == 183)
				{
					wprintf(L" Ошибка: %s файл уже существует\n", destinationName);
				}
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
			std::wstring *nameNewFile = new std::wstring(destinationName);
			nameNewFile->append(L"\\");
			nameNewFile->append(shortName);
			TCHAR* pname = (TCHAR*)&(nameNewFile->c_str()[0]);

			CopyFilesInfo *cfi = new CopyFilesInfo(nameSource, pname);
			Pool::WorkItem work(function, cfi);
			pool->addWorkToQueue(work);
		}
	}
}

