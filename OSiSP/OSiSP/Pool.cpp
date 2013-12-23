#include "stdafx.h"
#include "Pool.h"

using namespace std;

Pool::Pool(int minCountThreads, int maxCounThreads, int maxTimeLife):
	minCountThreads(minCountThreads), 
	maxCountThreads(maxCounThreads),
	maxTimeLife(maxTimeLife)
{
	handlers = new map<HANDLE, DWORD>();
	killThreads = new map<HANDLE, bool>();
	queueWorks = new vector<WorkItem>();
	semaphoreFreeThread = CreateSemaphore(NULL, 0, MAX_COUNT_WORKS, NULL);
	InitializeCriticalSectionAndSpinCount(&criticalSectionForWorks, COUNT_SPIN_CRITICAL_SECTION);
	InitializeCriticalSectionAndSpinCount(&criticalSectionForTimeLife, COUNT_SPIN_CRITICAL_SECTION);
	InitializeCriticalSectionAndSpinCount(&criticalSectionForKill, COUNT_SPIN_CRITICAL_SECTION);

	PTHREAD_START_ROUTINE threadWrappedFunction = this->threadFunction;
	for (int i = 0; i < minCountThreads; i++)
	{
		HANDLE temp = CreateThread(NULL, 0, threadWrappedFunction, NULL, 0, NULL);
		if (temp != NULL)
		{
			handlers->at(temp) = 0;
			killThreads->at(temp) = false;
		}
		else
		{
			//ярая ошибка
		}
	}

	//запускаем поток убийцу
	PTHREAD_START_ROUTINE threadKiller = this->killer;
	CreateThread(NULL, 0, threadKiller, NULL, 0, NULL); 

	/*map<HANDLE, bool>::iterator it = killThreads->begin();
	for (; it != killThreads->end(); it++)
	{
		ResumeThread((*it).first);
	}*/
}

Pool::~Pool(void)
{
	map<HANDLE, DWORD>::iterator it = handlers->begin();
	for (; it != handlers->end(); it++)
	{
		CloseHandle((*it).first);
	}
	delete handlers;
	delete queueWorks;
	CloseHandle(semaphoreFreeThread);
	DeleteCriticalSection(&criticalSectionForWorks);
	DeleteCriticalSection(&criticalSectionForTimeLife);
	DeleteCriticalSection(&criticalSectionForKill);
}

void Pool::addWorkToQueue(WorkItem work)
{
	queueWorks->push_back(work);
	LONG previosValue;
	ReleaseSemaphore(semaphoreFreeThread, 1, &previosValue); 
}



DWORD WINAPI Pool::threadFunction(void* context)
{
	HANDLE currentThreadHandle = getCurrentThreadHandle();
	//работаем пока хотим, что поток жил или не убиваем его
	while(true)
	{
		//выходим из потока, если ему повелел killer
		EnterCriticalSection(&criticalSectionForKill);
		if (killThreads->at(currentThreadHandle) == true)
		{
			LeaveCriticalSection(&criticalSectionForKill);
			break;
		}
		LeaveCriticalSection(&criticalSectionForKill);

		//ждём пока нет новой задачи
		WaitForSingleObject(semaphoreFreeThread, INFINITE);

		//отмечаем, что поток начнёт работать
		EnterCriticalSection(&criticalSectionForTimeLife);
		handlers->at(currentThreadHandle) = 0;
		LeaveCriticalSection(&criticalSectionForTimeLife);

		//атомарно получаем работу и удаляем её из очереди
		//чтобы несколько потоко не выполняли одно и тоже
		EnterCriticalSection(&criticalSectionForWorks);
		WorkItem work = queueWorks->back();
		queueWorks->pop_back();
		LeaveCriticalSection(&criticalSectionForWorks);

		//работаем
		work.function(work.context);

		//время когда поток перестал работать
		EnterCriticalSection(&criticalSectionForTimeLife);
		DWORD currentTime = GetTickCount();
		handlers->at(currentThreadHandle) = currentTime;
		LeaveCriticalSection(&criticalSectionForTimeLife);
	}

	return 0;
}

DWORD WINAPI Pool::killer(PVOID context)
{
	int timeWithoutWork = 0;
	map<HANDLE, DWORD>::iterator it = handlers->begin();
	map<HANDLE, bool>::iterator itKill = killThreads->begin();
	for (;it != handlers->end(); it++, itKill++)
	{
		if( (*it).second != 0)
		{
			timeWithoutWork = GetTickCount() - (*it).second;
			if (timeWithoutWork >= maxTimeLife)
			{
				EnterCriticalSection(&criticalSectionForKill);
				(*itKill).second = true;
				LeaveCriticalSection(&criticalSectionForKill);
			}
		}
	}
	return 0;
}

HANDLE Pool::getCurrentThreadHandle()
{
	HANDLE result;
	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(),
		GetCurrentProcess(), &result, 0,
		FALSE, DUPLICATE_SAME_ACCESS);
	return result;
}

void Pool::killAllThread()
{

}