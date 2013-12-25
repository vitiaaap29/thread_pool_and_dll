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
			wprintf(TEXT("���� ������ �� ����� ������� ����� � �����������\r\n"));
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
	int periodMiliseconds = maxTimeLife / 10;
	SetWaitableTimer(killThreadTimer, &li, periodMiliseconds, NULL, NULL, FALSE);

	killKiller = false;
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

HANDLE Pool::getCurrentThreadHandle()
{
	HANDLE result;
	HANDLE prom = GetCurrentProcess();
	HANDLE thre = GetCurrentThread();
	DuplicateHandle(prom, thre,
		prom, &result, 0,
		FALSE, DUPLICATE_SAME_ACCESS);

	DWORD id = GetCurrentThreadId();
	result = OpenThread(PROCESS_ALL_ACCESS, false, GetCurrentThreadId());
	return result;
}

DWORD WINAPI Pool::killer(PVOID context)
{
	int timeWithoutWork = 0;
	Pool *pool = (Pool*)context;
	while(*pool->killKiller == false)
	{
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
	//�������� ���� �����, ��� ����� ��� ��� �� ������� ���
	printf("Thread: %d\n", currentThreadId);
	fflush(stdin);
	Pool *pool = (Pool*)context;

	while(true)
	{
		//������� �� ������, ���� ��� ������� killer
		EnterCriticalSection(&pool->killCriticalSection);
		if (pool->killThreads->at(currentThreadId) == true)
		{
			LeaveCriticalSection(&pool->killCriticalSection);
			break;
		}
		LeaveCriticalSection(&pool->killCriticalSection);

		//��� ���� ��� ����� ������
		WaitForSingleObject(pool->semaphoreFreeThread, INFINITE);

		//��������, ��� ����� ����� ��������
		EnterCriticalSection(&pool->timeLifeCriticalSection);
		pool->handlersAndTime->at(currentThreadId) = SIGN_THREAD_NOW_WORK;
		LeaveCriticalSection(&pool->timeLifeCriticalSection);

		//�������� �������� ������ � ������� � �� �������
		//����� ��������� ������ �� ��������� ���� � ����
		EnterCriticalSection(&pool->worksCriticalSection);
		WorkItem work = pool->queueWorks->back();
		pool->queueWorks->pop_back();
		LeaveCriticalSection(&pool->worksCriticalSection);

		//��������
		work.function(work.context);

		//����� ����� ����� �������� ��������
		EnterCriticalSection(&pool->timeLifeCriticalSection);
		DWORD currentTime = GetTickCount();
		pool->handlersAndTime->at(currentThreadId) = currentTime;
		LeaveCriticalSection(&pool->timeLifeCriticalSection);
	}

	return 0;
}
