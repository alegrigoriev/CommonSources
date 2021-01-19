// Copyright Alexander Grigoriev, 1997-2002, All Rights Reserved
#pragma once

class CSimpleCriticalSection
{
	CRITICAL_SECTION m_cs;
public:
	CSimpleCriticalSection() noexcept
	{
		InitializeCriticalSection( & m_cs);
	}
	~CSimpleCriticalSection()
	{
		DeleteCriticalSection( & m_cs);
	}
	void Lock() volatile noexcept
	{
		EnterCriticalSection(const_cast<CRITICAL_SECTION *>( & m_cs));
	}
	void Unlock() volatile noexcept
	{
		LeaveCriticalSection(const_cast<CRITICAL_SECTION *>( & m_cs));
	}
private:
	CSimpleCriticalSection& operator =(const CSimpleCriticalSection&) = delete;
	CSimpleCriticalSection(const CSimpleCriticalSection&) = delete;
};
class CSimpleCriticalSectionLock
{
	CSimpleCriticalSection volatile & m_cs;
public:
	CSimpleCriticalSectionLock(CSimpleCriticalSection volatile& cs) noexcept
		: m_cs(cs)
	{
		cs.Lock();
	}
	~CSimpleCriticalSectionLock()
	{
		m_cs.Unlock();
	}
private:
	CSimpleCriticalSectionLock& operator =(const CSimpleCriticalSectionLock&) = delete;
	CSimpleCriticalSectionLock(const CSimpleCriticalSectionLock&) = delete;
};
