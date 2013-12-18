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

int loadAllLibrary(std::vector<std::wstring*> dllName);
void status();
void exit();
void copy();
void size();

int listFileData(TCHAR* dirName);

int main()
{
	setlocale( LC_ALL,"Russian" );
	std::vector<std::wstring> dllNames;
	dllNames.push_back(L"list.dll");

	int input;
	std::cout<<"1-List\n";
	std::cout<<"2-Status\n";
	std::cout<<"3-Exit\n";
	std::cout<<"4-Copy\n";
	std::cout<<"5-Size\n";
	std::cout<<"Selection: ";
	std::cin>> input;

	TCHAR dirName[MAX_PATH];

	switch ( input ) {

	case 1:
		std::cout<<"Vvedite polni put' k direktorii:";
		wscanf_s(L"%s",dirName, _countof(dirName));
		listFileData(dirName);
		break;
	case 2:
		status();
		break;
	case 3:
		exit();
		break;
	case 4:
		copy();
		break;
	case 5:
		size();
		break;
	default:
		std::cout<<"Error, bad input, quitting\n";
		break;
	}

	system("pause");
	return 0;

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

int loadAllLibrary(std::vector<std::wstring*> dllName)
{
	std::vector<void*> adress;
	adress.push_back(list); //list
	//
	//
	//
	//
	int countLibrary = dllName.size();

	for (int i = 0; i < countLibrary; i++)
	{
		HMODULE hDll;
		// Указатель на функцию
		//void (*adress) (void);

		LPCWSTR name = (LPCWSTR)dllName[i];
		// Загружаем динамически подключаемую библиотеку
		hDll = LoadLibrary(name);
		if(!hDll)
		{
			std::cout << _T("Динамическая библиотека не загружена") << std::endl;
			return GetLastError();
		}
		// Настраиваем адрес функции
		adress[i] = (void (*)(void))GetProcAddress(hDll, "HelloWorld");
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

void exit()
{
	std::cout << "3-Exit\n";
}

void copy()
{
	std::cout << "4-Copy\n";
}

void size()
{
	std::cout << "5-Size\n";
}