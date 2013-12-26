// OSiSP.cpp: îïðåäåëÿåò òî÷êó âõîäà äëÿ êîíñîëüíîãî ïðèëîæåíèÿ.
//
#include "stdafx.h"
#include <iostream>
#include <locale.h>
#include <string.h>
#include "copyFileInfo.h"
#include "Pool.h"

Pool* pool;

/*Óêàçàòåëè íà çàãðóæàåìûå ôóíêöèè*/
void (*list) (std::wstring dirName);

int loadAllLibrary(std::vector<std::wstring> dllName, std::vector<std::wstring> functionNames);
void status();
void exit();
void getCopyInputData();
void size();

int listFileData(TCHAR* dirName);
void copyFile(void* context);

int main(int argc, char** argv)
{
	int min;
	int max;
	int timeLive;

	if (argc < 4)
	{
		wprintf(TEXT("Íå ïðàâèëüíîå ÷èñëî àãðóìåíòîâ. Èä¸ì ëåñîì.\n"));
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
		wprintf(TEXT("Àðãóìåíòû èìåþò íåäîïóñòèìîå çíà÷åíèå. Èä¸ì ëåñîì.\n"));
		system("pause");
		return -1;
	}

	pool = new Pool(min, max, timeLive);

	setlocale( LC_ALL,"Russian" );
	std::vector<std::wstring> dllNames;
	dllNames.push_back(L"list.dll");

	std::vector<std::wstring> functionNames;
	functionNames.push_back(L"listFileData");

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
		fflush(stdout);
		fflush(stdin);
		std::cin>> input;
		//system("cls");
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
				exit();
				break;
			case 4:
				getCopyInputData();
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
		// Óêàçàòåëü íà ôóíêöèþ
		//void (*adress) (void);

		LPCWSTR name = const_cast<LPCWSTR>(&dllNames[i][0]);
		// Çàãðóæàåì äèíàìè÷åñêè ïîäêëþ÷àåìóþ áèáëèîòåêó
		hDll = LoadLibrary(name);
		if(!hDll)
		{
			std::cout << _T("Äèíàìè÷åñêàÿ áèáëèîòåêà íå çàãðóæåíà") << std::endl;
			return GetLastError();
		}
		// Íàñòðàèâàåì àäðåñ ôóíêöèè
		adress[i] = (void (*)(void))GetProcAddress(hDll, (LPCSTR)(&functionNames.at(i)[0]));
		if(!adress[i])
		{
			std::cout << _T("Îøèáêà ïîëó÷åíèÿ àäðåñà ôóíêöèè") << std::endl;
			return GetLastError();
		}
		// Âûçûâàåì ôóíêöèþ èç áèáëèîòåêè
		//adress();

		// Îòêëþ÷àåì áèáëèîòåêó
		if(!FreeLibrary(hDll))
		{
			std::cout << _T("Îøèáêà âûãðóçêè áèáëèîòåêè èç ïàìÿòè") << std::endl;
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
	pool->killAll();
}

void getCopyInputData()
{
	std::cout << "4-Copy\n";
	wprintf(TEXT("Ââåäèòå ôàéë/ïàïêó äëÿ êîïèðîâàíèÿ "));
	TCHAR sourceName[MAX_PATH];
	_tscanf_s(L"%s",sourceName, MAX_PATH/*_countof(sourceName)*/);

	wprintf(TEXT("Ââåäèòå ôàéë/ïàïêó - ìåñòî íàçíà÷åíèÿ "));
	TCHAR destinationName[MAX_PATH];
	_tscanf_s(L"%s",destinationName, _countof(destinationName));
	wprintf(L"ïîäãîòîâêà êîïèðîâàíèÿ %s â %s\n", sourceName,  destinationName);
	dispatchCopyByPool(sourceName, destinationName, copyFile, pool);
}

void size()
{
	std::cout << "5-Size\n";
	wprintf(TEXT("Введите имя файла/директории "));
	TCHAR name[MAX_PATH];
	_tscanf_s(L"%s",name, MAX_PATH);
	float* size = 0;
	dispatchSizeByPool(name, size, sizeFile, pool);
}