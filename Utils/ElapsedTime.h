#pragma once
#include <mmsystem.h>

namespace ElapsedTime
{

template<typename TimeTraits>
class CElapsedTimeT : TimeTraits
{
	using TimeTraits::GetTime;
	using TimeTraits::ToMs;
	using TimeTraits::ToTenthMs;
public:

	CElapsedTimeT() noexcept
		: m_StartTime(GetTime())
	{
	}

	~CElapsedTimeT()
	{
	}
	typename TimeTraits::TimeDifferenceType ElapsedTime() noexcept
	{
		return GetTime() - m_StartTime;
	}

	DWORD ElapsedTimeMs() noexcept
	{
		return ToMs(GetTime() - m_StartTime);
	}
	DWORD ElapsedTimeTenthMs() noexcept
	{
		return ToTenthMs(GetTime() - m_StartTime);
	}
	void Reset() noexcept
	{
		m_StartTime = GetTime();
	}
private:
	typename TimeTraits::TimeType m_StartTime;
};

struct CElapsedTimeDummy
{
	DWORD ElapsedTime() noexcept
	{
		return 0;
	}

	DWORD ElapsedTimeMs() noexcept
	{
		return 0;
	}
	DWORD ElapsedTimeTenthMs() noexcept
	{
		return 0;
	}
};

class Win32TimeTraits
{
public:
	typedef DWORD TimeType;
	typedef DWORD TimeDifferenceType;
	static DWORD ToMs(TimeDifferenceType diff) noexcept
	{
		return diff;
	}
	static DWORD ToTenthMs(TimeDifferenceType diff) noexcept
	{
		return diff * 10;
	}
	static TimeType GetTime() noexcept
	{
		return GetTickCount();
	}
};

class MmTimeTraits
{
public:
	typedef DWORD TimeType;
	typedef DWORD TimeDifferenceType;
	static DWORD ToMs(TimeDifferenceType diff) noexcept
	{
		return diff;
	}
	static DWORD ToTenthMs(TimeDifferenceType diff) noexcept
	{
		return diff * 10;
	}
	static TimeType GetTime() noexcept
	{
		return timeGetTime();
	}
};

class PerfCounterTimeTraits
{
public:
	typedef ULONGLONG TimeType;
	typedef LONGLONG TimeDifferenceType;
	static DWORD ToMs(TimeDifferenceType diff) noexcept
	{
		LARGE_INTEGER freq;
		if (QueryPerformanceFrequency( & freq))
		{
			ASSERT(freq.QuadPart >= 1000U);
			return DWORD(diff / ULONG(freq.QuadPart / 1000U));
		}
		return 0;
	}
	static DWORD ToTenthMs(TimeDifferenceType diff) noexcept
	{
		LARGE_INTEGER freq;
		if (QueryPerformanceFrequency( & freq))
		{
			ASSERT(freq.QuadPart >= 10000U);
			return DWORD(diff / ULONG(freq.QuadPart / 10000U));
		}
		return 0;
	}
	static TimeType GetTime() noexcept
	{
		LARGE_INTEGER counter;
		if (QueryPerformanceCounter( & counter))
		{
			return counter.QuadPart;
		}
		return 0;
	}
};
}

typedef ElapsedTime::CElapsedTimeT<ElapsedTime::Win32TimeTraits> ElapsedTimeStamp;
typedef ElapsedTime::CElapsedTimeT<ElapsedTime::MmTimeTraits> ElapsedTimeStampMm;
typedef ElapsedTime::CElapsedTimeT<ElapsedTime::PerfCounterTimeTraits> ElapsedTimeStampPerf;

#ifdef _DEBUG
typedef ElapsedTimeStampPerf DebugTimeStamp;
#else
typedef ElapsedTime::CElapsedTimeDummy DebugTimeStamp;
#endif
