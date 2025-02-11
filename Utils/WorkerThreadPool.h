#pragma once
#include "KListEntry.h"

class WorkerThreadPoolJob;
class WorkerThreadPool
{
public:
	WorkerThreadPool();
	~WorkerThreadPool();
	// default: number of CPUs (including hyperthreads), up to 64
	// returns number of created threads
	int Start(int NumberOfThreads = 0, int Priority = THREAD_PRIORITY_BELOW_NORMAL);
	void ShutDown();	// shut down threads

	bool AddJob(WorkerThreadPoolJob * job);

private:
	int m_NumberOfThreads;
	HANDLE m_Thread[MAXIMUM_WAIT_OBJECTS];
	HANDLE m_StopEvent;		// manual reset; is set in ShutDown to stop all threads
	HANDLE m_WorkAvailableEvent;
	HANDLE m_JobsAvailableSemaphore;

	CSimpleCriticalSection m_cs;
	unsigned _WorkerThread();
	ListHead<WorkerThreadPoolJob> m_QueuedJobs;
	//ListHead<WorkerThreadPoolJob> m_RetiredJobs;
	static unsigned __stdcall WorkerThread(void *);
	LONG m_JobsMasterLock;	// whichever thread gets it, will wait for

private:
	WorkerThreadPool(const WorkerThreadPool&) = delete;
	WorkerThreadPool(WorkerThreadPool&&) = delete;
	WorkerThreadPool& operator=(const WorkerThreadPool&) = delete;
	WorkerThreadPool& operator=(WorkerThreadPool&&) = delete;
};

class WorkerThreadPoolItem;

// job consists of multiple workitems. Workitems can be executed in parallel. Jobs are executed in serial.
class WorkerThreadPoolJob;
typedef bool WorkerItemProc(WorkerThreadPoolItem * workitem);
class WorkerThreadPoolItem : public ListItem<WorkerThreadPoolItem>
{
public:
	virtual ~WorkerThreadPoolItem() {}
	WorkerThreadPoolItem()
		: DeleteOnRetire(false)
		, m_Job(NULL)
		, Proc(NULL)
		, Context1(NULL)
		, Context2(NULL)
	{
	}
	bool DeleteOnRetire;
	WorkerThreadPoolJob * m_Job;
	WorkerItemProc * Proc;
	void * Context1;
	void * Context2;
private:
	WorkerThreadPoolItem(const WorkerThreadPoolItem&) = delete;
	WorkerThreadPoolItem(WorkerThreadPoolItem&&) = delete;
	WorkerThreadPoolItem& operator=(const WorkerThreadPoolItem&) = delete;
	WorkerThreadPoolItem& operator=(WorkerThreadPoolItem&&) = delete;
};

class WorkerThreadPoolJob : public ListItem<WorkerThreadPoolJob>
{
public:
	WorkerThreadPoolJob();
	~WorkerThreadPoolJob();
	bool WaitForCompletion(DWORD timeout = INFINITE);

	void AddWorkitem(class WorkerThreadPoolItem * Item);

	HANDLE m_CompletionEvent;
	LONG m_WorkitemsPending;
	WorkerThreadPool * m_Pool;
	ListHead<WorkerThreadPoolItem> m_Items;
	ListHead<WorkerThreadPoolItem> m_DoneItems;

private:
	WorkerThreadPoolJob(const WorkerThreadPoolJob&) = delete;
	WorkerThreadPoolJob(WorkerThreadPoolJob&&) = delete;
	WorkerThreadPoolJob& operator=(const WorkerThreadPoolJob&) = delete;
	WorkerThreadPoolJob& operator=(WorkerThreadPoolJob&&) = delete;
};

