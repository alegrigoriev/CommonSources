#include "stdafx.h"
#include "WorkerThreadPool.h"

WorkerThreadPool::WorkerThreadPool()
	: m_NumberOfThreads(0),
	m_StopEvent(NULL), m_JobsAvailableSemaphore(NULL), m_WorkAvailableEvent(NULL), m_JobsMasterLock(1)
{
	m_StopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_JobsAvailableSemaphore = CreateSemaphore(NULL, 0, 0x10000000, NULL);
	m_WorkAvailableEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	ZeroMemory(&m_Thread, sizeof m_Thread);
}

int WorkerThreadPool::Start(int NumberOfThreads, int Priority)
{
	if (m_NumberOfThreads != 0)
	{
		return m_NumberOfThreads;
	}
	ResetEvent(m_StopEvent);
	DWORD_PTR ProcessAffinity, SystemAffinity;

	if (NumberOfThreads <= 0)
	{
		NumberOfThreads = (sizeof ProcessAffinity) * 8;	// maximum
	}
	if (NumberOfThreads > MAXIMUM_WAIT_OBJECTS)
	{
		NumberOfThreads = MAXIMUM_WAIT_OBJECTS;
	}
	GetProcessAffinityMask(GetCurrentProcess(), &ProcessAffinity, &SystemAffinity);

	for (; m_NumberOfThreads < NumberOfThreads && ProcessAffinity != 0; m_NumberOfThreads++)
	{
		unsigned id;
		DWORD_PTR CurrentAffinity = (0 - ProcessAffinity) & ProcessAffinity;
		ProcessAffinity -= CurrentAffinity;

		m_Thread[m_NumberOfThreads] = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, this, CREATE_SUSPENDED, &id);
		if (NULL == m_Thread[m_NumberOfThreads])
		{
			break;
		}
		if (NumberOfThreads > 2)
		{
			SetThreadAffinityMask(m_Thread[m_NumberOfThreads], CurrentAffinity);
		}
		if (Priority != THREAD_PRIORITY_NORMAL)
		{
			SetThreadPriority(m_Thread[m_NumberOfThreads], Priority);
		}
		ResumeThread(m_Thread[m_NumberOfThreads]);
	}
	return m_NumberOfThreads;
}

WorkerThreadPool::~WorkerThreadPool()
{
	ShutDown();
	CloseHandle(m_StopEvent);
	CloseHandle(m_JobsAvailableSemaphore);
	CloseHandle(m_WorkAvailableEvent);

}

void WorkerThreadPool::ShutDown()	// shut down threads
{
	//
	SetEvent(m_StopEvent);
	if (m_NumberOfThreads != 0)
	{
		WaitForMultipleObjects(m_NumberOfThreads, m_Thread, TRUE, INFINITE);
		for (int i = 0; i < m_NumberOfThreads; i++)
		{
			CloseHandle(m_Thread[i]);
			m_Thread[i] = NULL;
		}
		m_NumberOfThreads = 0;
	}
}

bool WorkerThreadPool::AddJob(WorkerThreadPoolJob * job)
{
	//
	{
		job->m_Pool = this;
		// cannot add an already queued job
		ASSERT(job->IsAlone());
		ASSERT(!job->m_Items.IsEmpty());	// must have work
		CSimpleCriticalSectionLock lock(m_cs);
		m_QueuedJobs.InsertTail(job);
	}
	ReleaseSemaphore(m_JobsAvailableSemaphore, 1, NULL);
	return true;
}

unsigned __stdcall WorkerThreadPool::WorkerThread(void *ctx)
{
	WorkerThreadPool * p = (WorkerThreadPool*)ctx;
	return p->_WorkerThread();
}

// only one WorkerThreadPoolJob is executed at any given time, but its workitems can be executed in parallel on multiple threads
unsigned WorkerThreadPool::_WorkerThread()
{
	// only one thread waits for JobsAvailable, all others wait for m_WorkAvailableEvent
	HANDLE events[2] = { m_StopEvent};
	if (InterlockedExchange(&m_JobsMasterLock, 0))
	{
		// this thread will own the job and will give workitems from it
		events[1] = m_JobsAvailableSemaphore;
	}
	else
	{
		events[1] = m_WorkAvailableEvent;
	}
	while (WaitForMultipleObjects(2, events, FALSE, INFINITE) == WAIT_OBJECT_0 + 1)
	{
		WorkerThreadPoolJob * job = m_QueuedJobs.First();
		if (0 == job->m_WorkitemsPending)
		{
			// No workitems in this job. This thread will run the first workitem of next job
			ASSERT(events[1] == m_JobsAvailableSemaphore);
			{
				CSimpleCriticalSectionLock lock(m_cs);
				job->RemoveFromList();
			}
			SetEvent(job->m_CompletionEvent);
			// the jobs are not deleted by the thread
			continue;
		}
		// we own the job
		WorkerThreadPoolItem *item = job->m_Items.RemoveHead();
		if (!item->DeleteOnRetire)
		{
			job->m_DoneItems.InsertTail(item);
		}
		// if there is more work items, signal it:
		if (!job->m_Items.IsEmpty())
		{
			// next thread will pick more job
			SetEvent(m_WorkAvailableEvent);
		}

		item->Proc(item);	// TODO: handle return bool
		if (item->DeleteOnRetire)
		{
			delete item;
		}

		if (0 == InterlockedDecrement(&job->m_WorkitemsPending))
		{
			// we completed the last workitem. This thread will run the first workitem of next job
			ASSERT(job->m_Items.IsEmpty());

			events[1] = m_JobsAvailableSemaphore;
			{
				CSimpleCriticalSectionLock lock(m_cs);
				job->RemoveFromList();
			}
			SetEvent(job->m_CompletionEvent);
			// the jobs are not deleted by the thread
		}
		else
		{
			// will continue waiting for more work
			events[1] = m_WorkAvailableEvent;
		}
	}
	return 0;
}

WorkerThreadPoolJob::WorkerThreadPoolJob()
	: m_CompletionEvent(NULL)
	, m_Pool(NULL)
	, m_WorkitemsPending(0)
{
	m_CompletionEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

WorkerThreadPoolJob::~WorkerThreadPoolJob()
{
	// cannot delete still queued job
	ASSERT(IsAlone());
	while (!m_DoneItems.IsEmpty())
	{
		m_DoneItems.RemoveHead();
	}
	CloseHandle(m_CompletionEvent);
}

bool WorkerThreadPoolJob::WaitForCompletion(DWORD timeout)
{
	return WaitForSingleObject(m_CompletionEvent, timeout) == WAIT_OBJECT_0;
}

void WorkerThreadPoolJob::AddWorkitem(WorkerThreadPoolItem * Item)
{
	// cannot add to already queued job
	ASSERT(IsAlone());
	ASSERT(Item->IsAlone());
	ASSERT(Item->Proc != NULL);
	m_WorkitemsPending++;
	Item->m_Job = this;
	m_Items.InsertTail(Item);
}
