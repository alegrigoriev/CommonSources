#pragma once
#include "KListEntry.h"

class WorkerThreadPoolJob;
class WorkerThreadPool
{
	WorkerThreadPool(int NumberOfThreads = 0); // default: number of CPUs (including hyperthreads), up to 64
	~WorkerThreadPool();
	void ShutDown();	// shut down threads

	bool AddJob(WorkerThreadPoolJob * job);

	int m_NumberOfThreads;
	HANDLE m_Thread[64];
	HANDLE m_StopEvent;		// manual reset; is set in ShutDown to stop all threads
	HANDLE m_JobsAvailableSemaphore;

	CSimpleCriticalSection m_cs;
	UINT WorkerThreadProc();
	ListHead<WorkerThreadPoolJob> m_QueuedJobs;
	ListHead<WorkerThreadPoolJob> m_RetiredJobs;
};

typedef bool WorkItemProc(void * Context1, void * Context2);

class WorkerThreadPoolJob : ListItem<WorkerThreadPoolJob>
{
	WorkerThreadPoolJob();
	~WorkerThreadPoolJob();
	bool WaitForCompletion(DWORD timeout = INFINITE);

	void AddWorkitem(class WorkerItem * Item);

	HANDLE CompletionEvent;
	WorkerThreadPool m_Pool;
	ListHead<WorkerItem> m_Items;
	ListHead<WorkerItem> m_DoneItems;
};

class WorkerItem : ListItem<WorkerItem>
{
public:
	WorkerThreadPoolJob * m_Job;
	WorkItemProc * Proc;
	void * Context1;
	void * Context2;
};
