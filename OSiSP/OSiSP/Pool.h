#pragma once
class Pool
{
public:
	typedef void (*WorkFunction)(void*);

	struct WorkItem
	{
		void* context;
		WorkFunction function;
		WorkItem(WorkFunction  executableTask, void* context):
			function(executableTask), context(context){};
	};

	Pool(int minCountThreads, int maxCounThreads, int maxTimeLife);
	~Pool(void);

	void addWorkToQueue(WorkItem work);
	void killAll();
	void status();
private:
	static const int MAX_COUNT_WORKS = 0xFFFFFFF;
	static const int COUNT_SPIN_CRITICAL_SECTION = 4000;
	static const DWORD SIGN_THREAD_NOW_WORK = 0;

	int minCountThreads;
	int maxCountThreads;
	int maxTimeLife;

	HANDLE killThreadTimer;
	DWORD signKillKiller;

	std::vector<HANDLE> *handlers;

	CRITICAL_SECTION killCriticalSection;
	std::map<DWORD, bool>* killThreads;

	CRITICAL_SECTION timeLifeCriticalSection;
	std::map<DWORD, DWORD> *handlersAndTime;

	std::vector<WorkItem> *queueWorks;
	CRITICAL_SECTION worksCriticalSection;
	HANDLE semaphoreFreeThread;

	HANDLE addOrdinaryThread(bool multiThreadEnviroment);

	static DWORD WINAPI killer(PVOID context);
	
	static DWORD WINAPI threadFunction(void* context);
};

