#include "stdafx.h"
#include "Pool.h"

using namespace std;

Pool::Pool(int minCountThreads, int maxCounThreads, int maxTimeLife):
	minCountThreads(minCountThreads), 
	maxCountThreads(maxCounThreads),
	maxTimeLife(maxTimeLife)
{
	handlersAndTime = new map<HANDLE, DWORD>();
	killThreads = new map<HANDLE, bool>();
	queueWorks = new vector<WorkItem>();
	semaphoreFreeThread = CreateSemaphore(NULL, 0, MAX_COUNT_WORKS, NULL);
	InitializeCriticalSectionAndSpinCount(&worksCriticalSection, COUNT_SPIN_CRITICAL_SECTION);
	InitializeCriticalSectionAndSpinCount(&timeLifeCriticalSection, COUNT_SPIN_CRITICAL_SECTION);
	InitializeCriticalSectionAndSpinCount(&killCriticalSection, COUNT_SPIN_CRITICAL_SECTION);

	for (int i = 0; i < minCountThreads; i++)
	{
		HANDLE isCreate = addOrdinaryThread(false);
		if (isCreate == NULL)
		{
			wprintf(TEXT("���� ������ �� ����� ������� ����� � �����������\r\n"));
		}
	}

	//��������� ����� ������
	//PTHREAD_START_ROUTINE threadKiller = (PTHREAD_START_ROUTINE)this->killer;
	CreateThread(NULL, 0, this->killer, this, 0, NULL); 
}

Pool::~Pool(void)
{
	map<HANDLE, DWORD>::iterator it = handlersAndTime->begin();
	for (; it != handlersAndTime->end(); it++)
	{
		CloseHandle((*it).first);
	}
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
		addOrdinaryThread(true);
	}
	queueWorks->push_back(work);
	LONG previosValue;
	ReleaseSemaphore(semaphoreFreeThread, 1, &previosValue); 
}


HANDLE Pool::addOrdinaryThread(bool multiThreadEnviroment)
{
	HANDLE temp = CreateThread(NULL, 0, threadFunction, this, 0, NULL);
	if (multiThreadEnviroment)
	{
		EnterCriticalSection(&timeLifeCriticalSection);
		handlersAndTime->insert(std::pair<HANDLE, DWORD>(temp, 0));
		LeaveCriticalSection(&timeLifeCriticalSection);

		EnterCriticalSection(&killCriticalSection);
		killThreads->insert(std::pair<HANDLE,bool>(temp,false));
		LeaveCriticalSection(&killCriticalSection);
	}
	else
	{
		handlersAndTime->insert(std::pair<HANDLE, DWORD>(temp, 0));
		killThreads->insert(std::pair<HANDLE,bool>(temp,false));
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
	map<HANDLE, DWORD>::iterator it = pool->handlersAndTime->begin();
	map<HANDLE, bool>::iterator itKill = pool->killThreads->begin();
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
	return 0;
}

DWORD WINAPI Pool::threadFunction(void* context)
{
	HANDLE currentThreadHandle = getCurrentThreadHandle();
	//�������� ���� �����, ��� ����� ��� ��� �� ������� ���
	printf("Thread: %d\n", currentThreadHandle);
	fflush(stdin);
	Pool *pool = (Pool*)context;

	while(true)
	{
		//������� �� ������, ���� ��� ������� killer
		EnterCriticalSection(&pool->killCriticalSection);
		if (pool->killThreads->at(currentThreadHandle) == true)
		{
			LeaveCriticalSection(&pool->killCriticalSection);
			break;
		}
		LeaveCriticalSection(&pool->killCriticalSection);

		//��� ���� ��� ����� ������
		WaitForSingleObject(pool->semaphoreFreeThread, INFINITE);

		//��������, ��� ����� ����� ��������
		EnterCriticalSection(&pool->timeLifeCriticalSection);
		pool->handlersAndTime->at(currentThreadHandle) = SIGN_THREAD_NOW_WORK;
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
		pool->handlersAndTime->at(currentThreadHandle) = currentTime;
		LeaveCriticalSection(&pool->timeLifeCriticalSection);
	}

	return 0;
}
