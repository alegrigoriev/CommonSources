// profiler.h
// author: Alexander Grigoriev
// written: Jun 07, 1996
// Profiler class
// it works together with VCYCLE.VXD, described in
// MSDN CD "Extend Your Application with Dynamically
// Loaded VxDs Under Windows 95" article.
#ifndef __PROFILER_H__
#define __PROFILER_H__

#ifdef DO_PROFILING
#ifdef PROFILER_DLL_EXPORT
#define PROFILER_EXPORT __declspec(dllexport)
#else
#define PROFILER_EXPORT
#endif

#ifdef PROFILE_MT
#define PROFILER_THREAD_DATA __declspec(thread)
#else
#define PROFILER_THREAD_DATA
#endif

// PrintAllStatistics flags:
#define PRINTSTATS_SORT_NAME 1  // sort the output by name
#define PRINTSTATS_SORT_SHARE 2 // sort the output by share
#define PRINTSTATS_SORT_TIME 4  // sort the output by time per pass
#define PRINTSTATS_SORT_REVERSE 8  // reverse sort order

// C++ build
#ifdef __cplusplus

#ifndef __AFX_H__
#define _AFX_NO_BSTR_SUPPORT
#include <afx.h>			// MFC core and standard components
#endif

class Profiler
{
private:
	class VCYCLE_VXD
	{
		friend class Profiler;
		HANDLE hVxd;
		volatile __int64 uCounterOff;
		VCYCLE_VXD();
		void Init();
		void Shutdown();
	public:
		// I had to make the destructor public because
		// of strange MSVC behaviour: it did call a destructor
		// for the class member on the global scope,
		// not in class scope
		~VCYCLE_VXD();
	};
	friend class VCYCLE_VXD;
	PROFILER_EXPORT PROFILER_THREAD_DATA static VCYCLE_VXD Vxd;
	PROFILER_EXPORT PROFILER_THREAD_DATA static __int64 uExcludeBegin;
	PROFILER_EXPORT PROFILER_THREAD_DATA static Profiler * pFirst;
	PROFILER_EXPORT PROFILER_THREAD_DATA static Profiler * pLast;
	static BOOL IsVirtualized()
	{
		return Vxd.hVxd != INVALID_HANDLE_VALUE;
	}
	Profiler * pNext;
	//BOOL bChained;
	const char * m_szName;
	__int64 uCounterBegin;
	__int64 uTotal;
	unsigned int uCount;
	BOOL bChained;

public:
	Profiler(const char * szName, BOOL bDoChain = TRUE);
	~Profiler();
	static void InitVcycleVxd()
	{
		Vxd.Init();
	}
	static void ShutdownVcycleVxd()
	{
		Vxd.Shutdown();
	}
	static inline __int64 Profiler::ReadTSC();

	void PrintStatistics(FILE * pFile = stdout);
	static void PrintAllStatistics(FILE * pFile = stdout,int flags = 0);

	unsigned int GetCount()
	{
		return uCount;
	}
	__int64 GetAccumulatedClocks()
	{
		return uTotal;
	}
	void Start();
	void End();
	void Clear()
	{
		uTotal = 0;
		uCount = 0;
	}
	static void StartExclude();
	static void EndExclude();
};

class ProfilerRun
{
	Profiler & Prof;
public:
	ProfilerRun(Profiler & pProf)
		: Prof(pProf)
	{
		Prof.Start();
	}
	~ProfilerRun()
	{
		Prof.End();
	}
};

class ProfilerExclude
{
public:
	ProfilerExclude()
	{
		Profiler::StartExclude();
	}
	~ProfilerExclude()
	{
		Profiler::EndExclude();
	}
};

inline void Profiler::Start()
{
	__int64 uCounterBeginOff;
	uCount++;
	do {
		uCounterBeginOff = Vxd.uCounterOff;
		uCounterBegin = ReadTSC() - uCounterBeginOff;
		// we shall guarantee that no thread switch occured
		// between reading Vxd.uCounterOff and ReadTSC();
	} while (uCounterBeginOff - Vxd.uCounterOff);
}

inline void Profiler::End()
{
	__int64 uCounterEndOff, uCounterEnd;
	do {
		uCounterEndOff = Vxd.uCounterOff;
		uCounterEnd = ReadTSC() - uCounterEndOff;
		// we shall guarantee that no thread switch occured
		// between reading Vxd.uCounterOff and ReadTSC();
	} while (uCounterEndOff - Vxd.uCounterOff);
	uTotal += uCounterEnd - uCounterBegin;
}

#pragma warning(disable: 4035)
inline __int64 Profiler::ReadTSC()
{
	__asm {
		__emit 0x0f;
		__emit 0x31;
	}

}
#pragma warning(default: 4035)

#define PROFILER_INIT() Profiler::InitVcycleVxd()
#define PROFILER_SHUTDOWN() Profiler::ShutdownVcycleVxd()
#define PROFILER(FuncName) \
	PROFILER_THREAD_DATA static Profiler Prof__##FuncName(#FuncName); ProfilerRun ProfRun__##FuncName(Prof__##FuncName);
#define PROFILER_EXCLUDE() \
	ProfilerExclude ProfExc();
#define PROFILER_START_EXCLUDE() Profiler::StartExclude()
#define PROFILER_END_EXCLUDE() Profiler::EndExclude()
#define PROFILER_PRINT_ALL(pFile,flags) Profiler::PrintAllStatistics(pFile,flags)

#else
// C build
#include <stdio.h>
typedef struct Profiler
{
	struct Profiler * pNext;

	const char * m_szName;
	__int64 uCounterBegin;
	__int64 uTotal;
	unsigned int uCount;

} Profiler;

typedef struct VCYCLE_VXD
{
	void * hVxd;
	__int64 volatile uCounterOff;
} VCYCLE_VXD;

extern PROFILER_EXPORT PROFILER_THREAD_DATA VCYCLE_VXD Vxd;
extern PROFILER_EXPORT PROFILER_THREAD_DATA __int64 Profiler__uExcludeBegin;

#pragma warning(disable: 4035)
__inline __int64 Profiler__ReadTSC()
{
	__asm {
		__emit 0x0f;
		__emit 0x31;
	}
}
#pragma warning(default: 4035)

__inline void Profiler__Clear(Profiler * pPr)
{
	pPr->uTotal = 0;
	pPr->uCount = 0;
}

__inline void Profiler__StartExclude()
{
	__int64 uExcludeBeginOff;
	do {
		uExcludeBeginOff = Vxd.uCounterOff;
		Profiler__uExcludeBegin = Profiler__ReadTSC() - uExcludeBeginOff;
		// we shall guarantee that no thread switch occured
		// between reading Vxd.uCounterOff and ReadTSC();
	} while (uExcludeBeginOff != Vxd.uCounterOff);
}

void Profiler__Init(Profiler * pPr, const char * szName);

void Profiler__PrintStatistics(Profiler * pPr, FILE * pFile);
void Profiler__PrintAllStatistics(FILE * pFile,int nFlags);

__inline int Profiler__IsVirtualized()
{
	return Vxd.hVxd != (void*)(unsigned int) -1;
}

__inline void Profiler__Start(Profiler * pPr)
{
	__int64 uCounterBeginOff;
	pPr->uCount++;
	do {
		uCounterBeginOff = Vxd.uCounterOff;
		pPr->uCounterBegin = Profiler__ReadTSC()
							- uCounterBeginOff;
		// we shall guarantee that no thread switch occured
		// between reading Vxd.uCounterOff and ReadTSC();
	} while (uCounterBeginOff != Vxd.uCounterOff);
}

__inline void Profiler__End(Profiler * pPr)
{
	__int64 uCounterEndOff, uCounterEnd;
	do {
		uCounterEndOff = Vxd.uCounterOff;
		uCounterEnd = Profiler__ReadTSC() - uCounterEndOff;
		// we shall guarantee that no thread switch occured
		// between reading Vxd.uCounterOff and ReadTSC();
	} while (uCounterEndOff != Vxd.uCounterOff);
	pPr->uTotal += uCounterEnd - pPr->uCounterBegin;
}

__inline void Profiler__EndExclude()
{
	__int64 uExcludeEndOff, uExcludeEnd;
	do {
		uExcludeEndOff = Vxd.uCounterOff;
		uExcludeEnd = Profiler__ReadTSC() - uExcludeEndOff;
		// we shall guarantee that no thread switch occured
		// between reading Vxd.uCounterOff and ReadTSC();
	} while (uExcludeEndOff != Vxd.uCounterOff);
	Vxd.uCounterOff += uExcludeEnd - Profiler__uExcludeBegin;
}

#define PROFILER_BEGIN(FuncName) \
		PROFILER_THREAD_DATA static Profiler Prof__##FuncName; \
			{ static int flag = 0; \
			if (!flag) flag ++, Profiler__Init(& Prof__##FuncName, #FuncName); } \
		Profiler__Start(& Prof__##FuncName);

#define PROFILER_END(FuncName)  \
		Profiler__End(& Prof__##FuncName);

#define PROFILER_START_EXCLUDE() Profiler__StartExclude()
#define PROFILER_END_EXCLUDE() Profiler__EndExclude()
#define PROFILER_PRINT_ALL(pFile,flags) Profiler__PrintAllStatistics(pFile,flags)

#endif

#else   // #ifdef DO_PROFILING
#define PROFILER_INIT()
#define PROFILER_SHUTDOWN()
#define PROFILER(FuncName)
#define PROFILER_EXCLUDE()
#define PROFILER_START_EXCLUDE()
#define PROFILER_END_EXCLUDE()
#define PROFILER_PRINT_ALL(pFile,flags)
#define PROFILER_BEGIN(FuncName)
#define PROFILER_END(FuncName)
#endif

#endif // #ifndef __PROFILER_H__
