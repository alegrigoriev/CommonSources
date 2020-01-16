// Profiler.c
#define PROFILER_DLL_EXPORT
#ifdef DO_PROFILING
#include "profiler.h"
#include <windows.h>
PROFILER_EXPORT PROFILER_THREAD_DATA VCYCLE_VXD Vxd;

static PROFILER_THREAD_DATA Profiler * pPrFirst = NULL;
static PROFILER_THREAD_DATA Profiler * pPrLast = NULL;
PROFILER_EXPORT PROFILER_THREAD_DATA __int64 Profiler__uExcludeBegin = 0;

void VCYCLE_VXD__Init(VCYCLE_VXD * pVxd)
{
	pVxd->hVxd = CreateFile("\\\\.\\VCYCLE.VXD", 0, 0, NULL, 0,
							FILE_FLAG_DELETE_ON_CLOSE, NULL);
	if (pVxd->hVxd != INVALID_HANDLE_VALUE)
	{
		volatile __int64 * pTmp = & (pVxd->uCounterOff);
		DeviceIoControl(pVxd->hVxd, 2, & pTmp,
						sizeof pTmp, NULL, 0, NULL, NULL);
	}
	pVxd->uCounterOff = Profiler__ReadTSC();
}

void VCYCLE_VXD__Close(VCYCLE_VXD * pVxd)
{
	void * pTmp = NULL;
	DeviceIoControl(pVxd->hVxd, 2, & pTmp,
					sizeof pTmp, NULL, 0, NULL, NULL);
	CloseHandle(pVxd->hVxd);
}

void Profiler__Init(Profiler * pPr, const char * szName)
{
	if (Vxd.hVxd == NULL)
	{
		VCYCLE_VXD__Init(&Vxd);
	}
	pPr->m_szName = szName;
	pPr->uCount = 0;
	pPr->pNext = NULL;
	pPr->uTotal = 0;
	if (pPrLast != NULL)
	{
		pPrLast->pNext = pPr;
	}
	else
	{
		pPrFirst = pPr;
	}
	pPrLast = pPr;
}


void Profiler__PrintStatistics(Profiler * pPr, FILE * pFile)
{
	unsigned uMeanClocks = (unsigned)(pPr->uTotal / pPr->uCount);
	double dPercent = (double)(pPr->uTotal) * 100.
					/ (double) (Profiler__ReadTSC() - Vxd.uCounterOff);
	fprintf(pFile, "%-30.30s |%7u|%11.0f (%4.1f%%)|%9u|\n",
			pPr->m_szName, pPr->uCount, (double)(pPr->uTotal), dPercent, uMeanClocks);
}

void Profiler__PrintAllStatistics(FILE * pFile, int nFlags)
{
	Profiler * pProf = pPrFirst;
	// sort the list
	if (nFlags & (PRINTSTATS_SORT_NAME | PRINTSTATS_SORT_SHARE
			| PRINTSTATS_SORT_TIME))
	{
		Profiler * pTmp;
		pProf = NULL;
		while (pPrFirst != NULL)
		{
			Profiler * pPrev = NULL;
			pTmp = pProf;
			while(pTmp != NULL)
			{
				int nOrder = 0;
				if (nFlags & PRINTSTATS_SORT_NAME)
				{
					nOrder = 0 < strcmp(pTmp->m_szName, pPrFirst->m_szName);
				}
				else if (nFlags & PRINTSTATS_SORT_SHARE)
				{
					nOrder = pTmp->uTotal < pPrFirst->uTotal;
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
				pPrev->pNext = pPrFirst;
				pPrFirst = pPrFirst->pNext;
				pPrev->pNext->pNext = pTmp;
			}
			else
			{
				// add first item
				pProf = pPrFirst;
				pPrFirst = pPrFirst->pNext;
				pProf->pNext = pTmp;
			}
		}
	}
	if (FALSE == Profiler__IsVirtualized())
	{
		fprintf(pFile, "Warning: VCYCLE.VXD not installed, "
				"results ARE inaccurate!\n");
	}
	fprintf(pFile, "%-30.30s |%-7.7s|%-19.19s|%-9.9s|\n",
			"Function:", "Called #:",
			"Total clocks:", "Per call:");
	while (pProf != NULL)
	{
		Profiler__PrintStatistics(pProf, pFile);
		pProf = pProf->pNext;
	}
}
#endif
