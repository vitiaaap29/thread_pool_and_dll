// OSiSP.cpp: определяет точку входа для консольного приложения.
//
#include "stdafx.h"
#include <iostream>
#include <tchar.h>
#include <Windows.h>
#include <locale.h>
#include <vector>

/*Указатели на загружаемые функции*/
void (*list) (std::wstring dirName);

int loadAllLibrary(std::vector<std::wstring> dllName, std::vector<std::wstring> functionNames);
void status();
void exit(PTP_POOL pointerPoll);
void copy(PTP_CALLBACK_ENVIRON pce);
void size();

int listFileData(TCHAR* dirName);
DWORD WINAPI copyFile(PVOID context);
void copy(TCHAR* nameSource, TCHAR* destinationName, PTP_CALLBACK_ENVIRON pce);

struct CopyFilesInfo
{
	LPCTSTR lpExistingFileName;
	LPCTSTR lpNewFileName;
	BOOL bFailIfExists;
	CopyFilesInfo(LPCTSTR existingFileName, LPCTSTR newFileName, BOOL flag = false)
		:lpExistingFileName(existingFileName),
		lpNewFileName(newFileName),
		bFailIfExists(flag){};
};

int main(int argc, char** argv)
{
	if (argc < 4)
	{
		wprintf(TEXT("Не правильное число агрументов. Идём лесом.\n"));
		system("pause");
		return -1;
	}

	int min = atoi(argv[1]);
	int max = atoi(argv[2]);
	int timeLive = atoi(argv[3]);

	if (min < 1 || max > 500 || timeLive < 0)
	{
		wprintf(TEXT("Аргументы имеют недопустимое значение. Идём лесом.\n"));
		system("pause");
		return -1;
	}

	setlocale( LC_ALL,"Russian" );
	std::vector<std::wstring> dllNames;
	dllNames.push_back(L"list.dll");

	std::vector<std::wstring> functionNames;
	functionNames.push_back(L"listFileData");

	PTP_POOL pool = CreateThreadpool(NULL);
	SetThreadpoolThreadMinimum(pool, min);
	SetThreadpoolThreadMinimum(pool, max);
	TP_CALLBACK_ENVIRON callBackEnviron;
	InitializeThreadpoolEnvironment(&callBackEnviron);
	SetThreadpoolCallbackPool(&callBackEnviron, pool);

	loadAllLibrary(dllNames, functionNames);
	int input;

	TCHAR dirName[MAX_PATH];

	do{
		std::cout<<"1-List\n";
		std::cout<<"2-Status\n";
		std::cout<<"3-Exit\n";
		std::cout<<"4-Copy\n";
		std::cout<<"5-Size\n";
		std::cout<<"Selection: ";
		std::cin>> input;

		switch ( input ) 
		{
			case 1:
				std::cout<<"Vvedite polni put' k direktorii:";
				wscanf_s(L"%s",dirName, _countof(dirName));
				listFileData(dirName);
				break;
			case 2:
				status();
				break;
			case 3:
				exit(pool);
				break;
			case 4:
				copy(&callBackEnviron);
				break;
			case 5:
				size();
				break;
			default:
				std::cout<<"Error, bad input, quitting\n";
				break;
		}
	}while(input != 3);

	system("pause");
	return 0;

}

void copy(TCHAR* nameSource, TCHAR* destinationName, PTP_CALLBACK_ENVIRON pce)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	hFind = FindFirstFile(nameSource, &ffd);
	if (INVALID_HANDLE_VALUE == hFind) 
	{
		std::cout << _T("\r\nОшибка копирования\r\n") << std::endl;
		return;
	} 

	if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		if (CreateDirectory(destinationName, NULL))
		{
			wprintf(TEXT("Копируем %s в %s\n"), ffd.cFileName, destinationName);
			WIN32_FIND_DATA nextFfd;
			LPCWSTR nameNext = (LPCWSTR)(&std::wstring(nameSource).append(L"\\*")[0]);
			HANDLE handleNextFile = FindFirstFile(nameNext, &nextFfd);
			do
			{
				int lengthShortPartName = GetShortPathName(nextFfd.cFileName, NULL, 0);
				TCHAR* shortName = new TCHAR[lengthShortPartName];
				lengthShortPartName = GetShortPathName(nextFfd.cFileName, shortName, lengthShortPartName);
				if (lengthShortPartName != 0)
				{
					LPWSTR nameInsideDir = (LPWSTR)(&std::wstring(destinationName).append(L"\\")[0]);
					nameInsideDir = lstrcatW(nameInsideDir, (LPWSTR)shortName);
					if (nextFfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						copy(nextFfd.cFileName, nameInsideDir, pce);
					}
					else
					{
						CopyFilesInfo cfi(nextFfd.cFileName, nameInsideDir);
						PTP_WORK_CALLBACK workCallBack = (PTP_WORK_CALLBACK)copyFile;
						PTP_WORK work = CreateThreadpoolWork(workCallBack, (PVOID)(&cfi), pce);
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
			wprintf(TEXT("Ошибка создания новой директории\n"));
		}
	}
	else
	{
		CopyFilesInfo cfi(nameSource, destinationName);
		PTP_WORK_CALLBACK workCallBack = (PTP_WORK_CALLBACK)copyFile;
		PTP_WORK work = CreateThreadpoolWork(workCallBack, (PVOID)(&cfi), pce);
	}
}

DWORD WINAPI copyFile(PVOID context)
{
	CopyFilesInfo* cfi = (CopyFilesInfo*)context;
	int result = CopyFile(cfi->lpExistingFileName, cfi->lpNewFileName, cfi->bFailIfExists);
	return result;
}

int listFileData(TCHAR* dirName)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	hFind = FindFirstFile(dirName, &ffd);
	if (INVALID_HANDLE_VALUE == hFind) 
	{
		return -1;
	} 

	// List all the files in the directory with some info about them.
	LARGE_INTEGER filesize;
	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			wprintf(TEXT("  %s   <DIR>\n"), ffd.cFileName);
		}
		else
		{
			filesize.LowPart = ffd.nFileSizeLow;
			filesize.HighPart = ffd.nFileSizeHigh;
			wprintf(TEXT("  %s   %ld bytes\n"), ffd.cFileName, filesize.QuadPart);
		}
	}
	while (FindNextFile(hFind, &ffd) != 0);

	FindClose(hFind);

	return 0;
}

int loadAllLibrary(std::vector<std::wstring> dllNames, std::vector<std::wstring> functionNames)
{
	std::vector<void*> adress;
	adress.push_back(&list); //list
	//
	//
	//
	//
	int countLibrary = dllNames.size();

	for (int i = 0; i < countLibrary; i++)
	{
		HMODULE hDll;
		// Указатель на функцию
		//void (*adress) (void);

		LPCWSTR name = const_cast<LPCWSTR>(&dllNames[i][0]);
		// Загружаем динамически подключаемую библиотеку
		hDll = LoadLibrary(name);
		if(!hDll)
		{
			std::cout << _T("Динамическая библиотека не загружена") << std::endl;
			return GetLastError();
		}
		// Настраиваем адрес функции
		adress[i] = (void (*)(void))GetProcAddress(hDll, (LPCSTR)(&functionNames.at(i)[0]));
		if(!adress[i])
		{
			std::cout << _T("Ошибка получения адреса функции") << std::endl;
			return GetLastError();
		}
		// Вызываем функцию из библиотеки
		//adress();

		// Отключаем библиотеку
		if(!FreeLibrary(hDll))
		{
			std::cout << _T("Ошибка выгрузки библиотеки из памяти") << std::endl;
			return GetLastError();
		}

	}

	return 0;

}

void status()
{
	std::cout << "2-Status\n";
}

void exit(PTP_POOL pointerPoll)
{
	std::cout << "3-Exit\n";
	CloseThreadpool(pointerPoll);
}

void copy(PTP_CALLBACK_ENVIRON pce)
{
	std::cout << "4-Copy\n";
	wprintf(TEXT("Введите файл/папку для копирования "));
	TCHAR sourceName[MAX_PATH];
	wscanf_s(L"%s",sourceName, _countof(sourceName));

	wprintf(TEXT("Введите файл/папку - место назначения "));
	TCHAR destinationName[MAX_PATH];
	wscanf_s(L"%s",destinationName, _countof(destinationName));

	copy(sourceName, destinationName, pce);
}

void size()
{
	std::cout << "5-Size\n";
}