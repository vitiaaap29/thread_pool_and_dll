#pragma once
#include "stdafx.h"

typedef struct SizeFileInfo
{
	WIN32_FIND_DATA ffd;
	CRITICAL_SECTION* incSizeCriticalSection;
	float* sum;
	SizeFileInfo(WIN32_FIND_DATA ffd, float* sum)
	{
		this->ffd = ffd;
		InitializeCriticalSectionAndSpinCount(incSizeCriticalSection, 4000);
	}
	~SizeFileInfo(){}
} SizeFileInfo;