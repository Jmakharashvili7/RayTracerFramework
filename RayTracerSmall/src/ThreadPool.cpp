#include "ThreadPool.h"

#ifdef _WIN32
std::vector<std::thread> ThreadPool::m_threads;
#else
std::vector<pid_t> ThreadPool::m_forkIDs;
#endif

void ThreadPool::WaitAllJobs()
{
#ifdef _WIN32
	for (auto& th : m_threads)
	{
		th.join();
	}
	
	m_threads.clear();
#else
    int status;
    for (auto& id : m_forkIDs)
    {
        if (wait(&status) == -1) {

        }
        else if(WIFEXITED(status)) {

        }
    }
    m_forkIDs.clear();
#endif
}
