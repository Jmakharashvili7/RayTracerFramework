#include "ThreadPool.h"

std::vector<std::thread> ThreadPool::m_threads;

void ThreadPool::WaitAllThreads()
{
	for (auto& th : m_threads)
	{
		th.join();
	}
	
	m_threads.clear();
}
