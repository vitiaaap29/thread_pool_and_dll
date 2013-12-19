// OSiSP.cpp: определяет точку входа для консольного приложения.
//
#include "stdafx.h"
#include <iostream>
#include <tchar.h>
#include <Windows.h>
#include <locale.h>
#include <vector>
#include <string.h>

/*Указатели на загружаемые функции*/
void (*list) (std::wstring dirName);

int loadAllLibrary(std::vector<std::wstring> dllName, std::vector<std::wstring> functionNames);
void status();
void exit(PTP_POOL pointerPoll);
void copy(PTP_CALLBACK_ENVIRON pce);
void size();

int listFileData(TCHAR* dirName);
DWORD WINAPI copyFile(void* context);
void copy(TCHAR* nameSource, TCHAR* destinationName, PTP_CALLBACK_ENVIRON pce);

struct CopyFilesInfo
{
	TCHAR* existingFileName;
	TCHAR* newFileName;
	BOOL bFailIfExists;
	CopyFilesInfo(TCHAR* existingFileName, TCHAR* newFileName, BOOL flag = false);
	//CopyFilesInfo(CopyFilesInfo& rCopyFileInfo);
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

//CopyFilesInfo::CopyFilesInfo(CopyFilesInfo& rCopyFileInfo)
//{
//	CopyFilesInfo(rCopyFileInfo.existingFileName, rCopyFileInfo.newFileName, rCopyFileInfo.bFailIfExists);
//}

//CopyFilesInfo::CopyFilesInfo(void* pointer)
//{
//	*this = new CopyFilesInfo((TCHAR*)pointer, (TCHAR*)((int)pointer + MAX_PATH));
//}

CopyFilesInfo::~CopyFilesInfo()
{
	std::cout << _T("Деструктор структруы") << std::endl;
}

int main(int argc, char** argv)
{
	int min;
	int max;
	int timeLive;

	if (argc < 4)
	{
		wprintf(TEXT("Не правильное число агрументов. Идём лесом.\n"));
		system("pause");
		min = 2;
		max = 10;
		timeLive = 1000;
	}
	else
	{
		min = atoi(argv[1]);
		max = atoi(argv[2]);
		timeLive = atoi(argv[3]);
	}

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
		//system("cls");
		std::cout<<"1-List\n";
		std::cout<<"2-Status\n";
		std::cout<<"3-Exit\n";
		std::cout<<"4-Copy\n";
		std::cout<<"5-Size\n";
		std::cout<<"Selection: ";
		std::cin>> input;
		system("cls");
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
							copy(nextFfd.cFileName, pnameInsideDir, pce);
						}
						else
						{
							wprintf(TEXT(" Начинаем копирование\n"));
							TCHAR* pSource = &nextFfd.cFileName[0];
							CopyFilesInfo *cfi = new CopyFilesInfo(pSource, pnameInsideDir);
							PTP_WORK_CALLBACK workCallBack = (PTP_WORK_CALLBACK)copyFile;
							PTP_WORK work = CreateThreadpoolWork(workCallBack, (PVOID)(cfi), pce);
							SubmitThreadpoolWork(work);
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
			PTP_WORK_CALLBACK workCallBack = (PTP_WORK_CALLBACK)copyFile;
			PTP_WORK work = CreateThreadpoolWork(workCallBack, (void*)(cfi), pce);
			SubmitThreadpoolWork(work);
		}
	}
}

DWORD WINAPI copyFile(void* context)
{
	CopyFilesInfo* cfi = (CopyFilesInfo*)context;
	wprintf(TEXT("Непосредственное копирование из %s в %s\n", cfi->existingFileName,  cfi->newFileName));
	int result = CopyFile(cfi->existingFileName, cfi->newFileName, cfi->bFailIfExists);
	delete cfi;
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
	_tscanf_s(L"%s",sourceName, MAX_PATH/*_countof(sourceName)*/);

	wprintf(TEXT("Введите файл/папку - место назначения "));
	TCHAR destinationName[MAX_PATH];
	_tscanf_s(L"%s",destinationName, _countof(destinationName));
	wprintf(TEXT("подготовка копирования %s в %s\n", sourceName,  destinationName));
	copy(sourceName, destinationName, pce);
}

void size()
{
	std::cout << "5-Size\n";
}