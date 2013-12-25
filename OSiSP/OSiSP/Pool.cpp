#include "stdafx.h"
#include "Pool.h"

using namespace std;

Pool::Pool(int minCountThreads, int maxCounThreads, int maxTimeLife):
	minCountThreads(minCountThreads), 
	maxCountThreads(maxCounThreads),
	maxTimeLife(maxTimeLife)
{
	handlersAndTime = new map<DWORD, DWORD>();
	killThreads = new map<DWORD, bool>();
	queueWorks = new vector<WorkItem>();
	semaphoreFreeThread = CreateSemaphore(NULL, 0, MAX_COUNT_WORKS, NULL);
	InitializeCriticalSectionAndSpinCount(&worksCriticalSection, COUNT_SPIN_CRITICAL_SECTION);
	InitializeCriticalSectionAndSpinCount(&timeLifeCriticalSection, COUNT_SPIN_CRITICAL_SECTION);
	InitializeCriticalSectionAndSpinCount(&killCriticalSection, COUNT_SPIN_CRITICAL_SECTION);

	handlers = new vector<HANDLE>();
	for (int i = 0; i < minCountThreads; i++)
	{
		HANDLE currentHandler = addOrdinaryThread(false);
		if (currentHandler == NULL)
		{
			wprintf(TEXT("Ярая ошибка не можем создать поток в констукторе\r\n"));
		}
	}

	vector<HANDLE>::iterator it = handlers->begin();
	for (; it != handlers->end(); it++)
	{
		ResumeThread(*it);
	}

	LARGE_INTEGER li;
	killThreadTimer = CreateWaitableTimer(NULL, FALSE, NULL);

	int nTimerUnitsPerSecond = 10000000;
	li.QuadPart = -1 * nTimerUnitsPerSecond;
	int periodMiliseconds = maxTimeLife * 40;
	SetWaitableTimer(killThreadTimer, &li, periodMiliseconds, NULL, NULL, FALSE);

	signKillKiller = 0;
	CreateThread(NULL, 0, this->killer, this, 0, NULL); 
}

Pool::~Pool(void)
{
	vector<HANDLE>::iterator it = handlers->begin();
	for (; it != handlers->end(); it++)
	{
		CloseHandle(*it);
	}
	delete handlers;
	delete handlersAndTime;
	delete queueWorks;
	CloseHandle(semaphoreFreeThread);
	DeleteCriticalSection(&worksCriticalSection);
	DeleteCriticalSection(&timeLifeCriticalSection);
	DeleteCriticalSection(&killCriticalSection);
}

void Pool::addWorkToQueue(WorkItem work)
{
	bool needAddThread = handlersAndTime->size() < queueWorks->size() && handlersAndTime->size() < maxCountThreads;
	if (needAddThread)
	{
		HANDLE newThread = addOrdinaryThread(true);
		if (newThread != NULL)
		{
			ResumeThread(newThread);
		}
	}
	queueWorks->push_back(work);
	LONG previosValue;
	ReleaseSemaphore(semaphoreFreeThread, 1, &previosValue); 
}

void Pool::status()
{
	map<DWORD, DWORD>::iterator itFist = handlersAndTime->begin();
	map<DWORD, DWORD>::iterator itLast = handlersAndTime->end();
	EnterCriticalSection(&this->killCriticalSection);
	map<DWORD, DWORD> *current = new map<DWORD, DWORD>(itFist, itLast);
	LeaveCriticalSection(&this->killCriticalSection);

	map<DWORD, DWORD>::iterator it = current->begin();
	TCHAR* state;
	int countWork = 0;
	int countWait = 0;
	for (; it != current->end(); it++)
	{
		if ((*it).second == SIGN_THREAD_NOW_WORK)
		{
			state = L" работает   ";
			countWork++;
		}
		else
		{
			state = L" ждёт задачу";
			countWait++;
		}
		wprintf(L"id потока = %d  |  состояние = %s\n", (*it).first, state);
	}
	wprintf(L"%d потоков работает, %d ожидает\n", countWork, countWait);
}


HANDLE Pool::addOrdinaryThread(bool multiThreadEnviroment)
{
	HANDLE temp = CreateThread(NULL, 0, threadFunction, this, CREATE_SUSPENDED, NULL);
	if (temp != NULL)
	{
		DWORD threadId = GetThreadId(temp);
		if (multiThreadEnviroment)
		{
			EnterCriticalSection(&timeLifeCriticalSection);
			handlersAndTime->insert(std::pair<DWORD, DWORD>(threadId, 0));
			LeaveCriticalSection(&timeLifeCriticalSection);

			EnterCriticalSection(&killCriticalSection);
			killThreads->insert(std::pair<DWORD,bool>(threadId,false));
			LeaveCriticalSection(&killCriticalSection);
		}
		else
		{
			handlersAndTime->insert(std::pair<DWORD, DWORD>(threadId, 0));
			killThreads->insert(std::pair<DWORD,bool>(threadId,false));
		}
		handlers->push_back(temp);
	}

	return temp;
}

void Pool::killAll()
{
	map<DWORD, bool> *temp = new map<DWORD, bool>();
	map<DWORD, bool>::iterator it = killThreads->begin();
	for (; it != killThreads->end(); it++)
	{
		DWORD currentId = (*it).first;
		temp->insert(std::pair<DWORD,bool>(currentId, true));
	}
	delete killThreads;
	EnterCriticalSection(&killCriticalSection);
	killThreads = temp;	
	LeaveCriticalSection(&killCriticalSection);
	InterlockedExchangeAdd(&signKillKiller, 1);
	Sleep(maxTimeLife);
	if (queueWorks->size() != 0)
	{
		wprintf(TEXT("Осталось %d необработанных запросов\n"), queueWorks->size());
	}
}

DWORD WINAPI Pool::killer(PVOID context)
{
	int timeWithoutWork = 0;
	Pool *pool = (Pool*)context;
	while(InterlockedExchangeAdd(&pool->signKillKiller, 0) == 0)
	{
		//printf("Killer %d wait, me said.. i want kill=)\n", GetCurrentThreadId());
		WaitForSingleObject(pool->killThreadTimer, INFINITE);
		map<DWORD, DWORD>::iterator it = pool->handlersAndTime->begin();
		map<DWORD, bool>::iterator itKill = pool->killThreads->begin();
		for (;it != pool->handlersAndTime->end(); it++, itKill++)
		{
			if( (*it).second != SIGN_THREAD_NOW_WORK)
			{
				timeWithoutWork = GetTickCount() - (*it).second;
				if (timeWithoutWork >= pool->maxTimeLife)
				{
					EnterCriticalSection(&pool->killCriticalSection);
					(*itKill).second = true;
					LeaveCriticalSection(&pool->killCriticalSection);
				}
			}
		}
	}
	return 0;
}

DWORD WINAPI Pool::threadFunction(void* context)
{
	DWORD currentThreadId = GetCurrentThreadId();
	//работаем пока хотим, что поток жил или не убиваем его
	printf("Thread: %d\n", currentThreadId);
	fflush(stdin);
	Pool *pool = (Pool*)context;

	while(true)
	{
		//выходим из потока, если ему повелел killer
		EnterCriticalSection(&pool->killCriticalSection);
		if (pool->killThreads->at(currentThreadId) == true)
		{
			LeaveCriticalSection(&pool->killCriticalSection);
			break;
		}
		LeaveCriticalSection(&pool->killCriticalSection);
		printf("Work thread: %d wait on semaphore\n", currentThreadId);
		//ждём пока нет новой задачи
		WaitForSingleObject(pool->semaphoreFreeThread, INFINITE);
		printf("Work thread: %d now was work\n", currentThreadId);
		//отмечаем, что поток начнёт работать
		EnterCriticalSection(&pool->timeLifeCriticalSection);
		pool->handlersAndTime->at(currentThreadId) = SIGN_THREAD_NOW_WORK;
		LeaveCriticalSection(&pool->timeLifeCriticalSection);

		//атомарно получаем работу и удаляем её из очереди
		//чтобы несколько потоко не выполняли одно и тоже
		EnterCriticalSection(&pool->worksCriticalSection);
		WorkItem work = pool->queueWorks->back();
		pool->queueWorks->pop_back();
		LeaveCriticalSection(&pool->worksCriticalSection);

		//работаем
		work.function(work.context);

		//время когда поток перестал работать
		EnterCriticalSection(&pool->timeLifeCriticalSection);
		DWORD currentTime = GetTickCount();
		pool->handlersAndTime->at(currentThreadId) = currentTime;
		LeaveCriticalSection(&pool->timeLifeCriticalSection);
	}

	return 0;
}
