// Profiler.cpp
#ifndef __AFX_H__
#define _AFX_NO_BSTR_SUPPORT
#include <afx.h>			// MFC core and standard components
#endif
#define PROFILER_DLL_EXPORT
#ifdef DO_PROFILING
#include "profiler.h"
Profiler::VCYCLE_VXD::VCYCLE_VXD()
{
	Init();
}

void Profiler::VCYCLE_VXD::Init()
{
	hVxd = CreateFile("\\\\.\\VCYCLE.VXD", 0, 0, NULL, 0,
					FILE_FLAG_DELETE_ON_CLOSE, NULL);
	if (hVxd != INVALID_HANDLE_VALUE)
	{
		__int64 * pTmp = const_cast<__int64 *>(& uCounterOff);
		DeviceIoControl(hVxd, 2, & pTmp,
						sizeof pTmp, NULL, 0, NULL, NULL);
		//int iCode = GetLastError();
	}
	uCounterOff = Profiler::ReadTSC();
}

Profiler::VCYCLE_VXD::~VCYCLE_VXD()
{
	Shutdown();
}

void Profiler::VCYCLE_VXD::Shutdown()
{
	void * pTmp = NULL;
	if (hVxd)
	{
		DeviceIoControl(hVxd, 2, & pTmp,
						sizeof pTmp, NULL, 0, NULL, NULL);
		CloseHandle(hVxd);
		hVxd = NULL;
	}
}

PROFILER_THREAD_DATA Profiler::VCYCLE_VXD Profiler::Vxd;
PROFILER_THREAD_DATA Profiler * Profiler::pFirst = NULL;
PROFILER_THREAD_DATA Profiler * Profiler::pLast = NULL;
PROFILER_THREAD_DATA __int64 Profiler::uExcludeBegin = 0;

Profiler::Profiler(const char * szName, BOOL bDoChain)
	:m_szName(szName), uCount(0), pNext(NULL), bChained(bDoChain),
	uTotal(0)
{
	if (bDoChain)
	{
		if (pLast != NULL)
		{
			pLast->pNext = this;
		}
		else
		{
			pFirst = this;
		}
		pLast = this;
	}
}

Profiler::~Profiler()
{
	if (bChained)
	{
		Profiler * pPrev = NULL;
		Profiler * pCurr = pFirst;
		while (pCurr)
		{
			if (pCurr == this)
			{
				if (pPrev != NULL)
				{
					pPrev->pNext = pCurr->pNext;
				}
				else
				{
					pFirst = pCurr->pNext;
				}
				if (this == pLast)
				{
					pLast = pPrev;
				}
				break;
			}
			pPrev = pCurr;
			pCurr = pCurr->pNext;
		}
		bChained = FALSE;
	}
}

void Profiler::StartExclude()
{
	__int64 uExcludeBeginOff;
	do {
		uExcludeBeginOff = Vxd.uCounterOff;
		uExcludeBegin = ReadTSC() - uExcludeBeginOff;
		// we shall guarantee that no thread switch occured
		// between reading Vxd.uCounterOff and ReadTSC();
	} while (uExcludeBeginOff - Vxd.uCounterOff);
}

void Profiler::EndExclude()
{
	__int64 uExcludeEndOff, uExcludeEnd;
	do {
		uExcludeEndOff = Vxd.uCounterOff;
		uExcludeEnd = ReadTSC() - uExcludeEndOff;
		// we shall guarantee that no thread switch occured
		// between reading Vxd.uCounterOff and ReadTSC();
	} while (uExcludeEndOff - Vxd.uCounterOff);
	Vxd.uCounterOff += uExcludeEnd - uExcludeBegin;
}

void Profiler::PrintStatistics(FILE * pFile)
{
	unsigned uMeanClocks = unsigned(uTotal / uCount);
	double dPercent = double(uTotal) * 100. / double (ReadTSC() - Vxd.uCounterOff);
	fprintf(pFile, "%-30.30s |%7u|%11.0f (%4.1f%%)|%9u|\n",
			m_szName, uCount, double(uTotal), dPercent, uMeanClocks);
}

void Profiler::PrintAllStatistics(FILE * pFile, int nFlags)
{
	if (FALSE == IsVirtualized())
	{
		fprintf(pFile, "Warning: VCYCLE.VXD not installed, "
				"results ARE inaccurate!\n");
	}
	Profiler * pProf = pFirst;
	// sort the list
	if (nFlags & (PRINTSTATS_SORT_NAME | PRINTSTATS_SORT_SHARE
			| PRINTSTATS_SORT_TIME))
	{
		Profiler * pTmp;
		pProf = NULL;
		while (pFirst != NULL)
		{
			Profiler * pPrev = NULL;
			pTmp = pProf;
			while(pTmp != NULL)
			{
				int nOrder = 0;
				if (nFlags & PRINTSTATS_SORT_NAME)
				{
					nOrder = 0 < strcmp(pTmp->m_szName, pFirst->m_szName);
				}
				else if (nFlags & PRINTSTATS_SORT_SHARE)
				{
					nOrder = pTmp->uTotal < pFirst->uTotal;
				}
				else if (nFlags & PRINTSTATS_SORT_TIME)
				{
					nOrder = (pTmp->uTotal / pTmp->uCount)
							< (pProf->uTotal / pProf->uCount);
				}
				if (nFlags & PRINTSTATS_SORT_REVERSE) nOrder ^= 1;
				if (nOrder) break;
				pPrev = pTmp;
				pTmp = pTmp->pNext;
			}
			if (pPrev != NULL)
			{
				pPrev->pNext = pFirst;
				pFirst = pFirst->pNext;
				pPrev->pNext->pNext = pTmp;
			}
			else
			{
				// add first item
				pProf = pFirst;
				pFirst = pFirst->pNext;
				pProf->pNext = pTmp;
			}
		}
	}
	fprintf(pFile, "%-30.30s |%-7.7s|%-19.19s|%-9.9s|\n",
			"Function:", "Called #:",
			"Total clocks:", "Per call:");
	while (pProf != NULL)
	{
		pProf->PrintStatistics(pFile);
		pProf = pProf->pNext;
	}
}
#endif
