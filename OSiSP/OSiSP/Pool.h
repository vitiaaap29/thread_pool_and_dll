#pragma once
class Pool
{
private:
	int minCountThreads;
	int maxCounThreads;
	int maxTimeLife;
	std::vector<HANDLE> *handlers;
public:
	Pool(int minCountThreads, int maxCounThreads, int maxTimeLife);
	~Pool(void);
};

