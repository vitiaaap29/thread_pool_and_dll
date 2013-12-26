#pragma once
#include "stdafx.h"

typedef struct SizeFileInfo
{
	WIN32_FIND_DATA ffd;
	CRITICAL_SECTION* incSizeCriticalSection;
	float* sum;
	SizeFileInfo(WIN32_FIND_DATA ffd, float* sum, CRITICAL_SECTION* cs);
	~SizeFileInfo(){}
} SizeFileInfo;

inline SizeFileInfo::SizeFileInfo(WIN32_FIND_DATA ffd, float* sum, CRITICAL_SECTION* cs)
{
	this->sum = sum;
	this->ffd = ffd;
	incSizeCriticalSection = cs;
	InitializeCriticalSectionAndSpinCount(incSizeCriticalSection, 4000);
}