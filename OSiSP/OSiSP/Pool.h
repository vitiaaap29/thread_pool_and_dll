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
	void killAllThread();
private:
	const int MAX_COUNT_WORKS = 0xFFFFFFF;
	const int COUNT_SPIN_CRITICAL_SECTION = 4000;
	int minCountThreads;
	int maxCountThreads;
	int maxTimeLife;

	CRITICAL_SECTION criticalSectionForKill;
	std::map<HANDLE, bool>* killThreads;

	CRITICAL_SECTION criticalSectionForTimeLife;
	std::map<HANDLE, DWORD> *handlers;

	std::vector<WorkItem> *queueWorks;
	CRITICAL_SECTION criticalSectionForWorks;
	HANDLE semaphoreFreeThread;

	DWORD WINAPI threadFunction(void* context);

	DWORD WINAPI killer(PVOID context);

	HANDLE getCurrentThreadHandle();
};

