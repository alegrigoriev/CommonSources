#include "stdafx.h"
#include "WorkerThreadPool.h"

WorkerThreadPool::WorkerThreadPool(int NumberOfThreads)
	: m_NumberOfThreads(0),
	m_StopEvent(NULL), m_JobsAvailableSemaphore(NULL)
{

}

WorkerThreadPool::~WorkerThreadPool()
{
}
