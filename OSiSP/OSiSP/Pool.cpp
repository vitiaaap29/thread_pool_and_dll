#include "stdafx.h"
#include "Pool.h"

using namespace std;

Pool::Pool(int minCountThreads, int maxCounThreads, int maxTimeLife):
	minCountThreads(minCountThreads), 
	maxCounThreads(maxCounThreads),
	maxTimeLife(maxTimeLife)
{
	handlers = new vector<HANDLE>(maxCounThreads);
}


Pool::~Pool(void)
{
	for (int i = 0; i < handlers->size(); i++)
	{
		CloseHandle(handlers->at(i));
	}
	delete handlers;
}
