// Copyright Alexander Grigoriev, 1997-2002, All Rights Reserved
#pragma once

class CSimpleCriticalSection
{
	CRITICAL_SECTION m_cs;
public:
	CSimpleCriticalSection()
	{
		InitializeCriticalSection( & m_cs);
	}
	~CSimpleCriticalSection()
	{
		DeleteCriticalSection( & m_cs);
	}
	void Lock() volatile
	{
		EnterCriticalSection(const_cast<CRITICAL_SECTION *>( & m_cs));
	}
	void Unlock() volatile
	{
		LeaveCriticalSection(const_cast<CRITICAL_SECTION *>( & m_cs));
	}
};
class CSimpleCriticalSectionLock
{
	CSimpleCriticalSection volatile & m_cs;
public:
	CSimpleCriticalSectionLock(CSimpleCriticalSection volatile & cs)
		: m_cs(cs)
	{
		cs.Lock();
	}
	~CSimpleCriticalSectionLock()
	{
		m_cs.Unlock();
	}
private:
	CSimpleCriticalSectionLock & operator =(const CSimpleCriticalSectionLock&);
	CSimpleCriticalSectionLock(const CSimpleCriticalSectionLock&);
};
