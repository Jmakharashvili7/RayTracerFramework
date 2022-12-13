#pragma once
#include <vector>
#include <functional>
#ifdef _WIN32
#include <thread>
#else
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#endif

class ThreadPool
{
public:
	template<typename Function_Type>
	static void AddJob(Function_Type f)
	{
#ifdef _WIN32
		m_threads.push_back(std::thread(f));
#else
        pid_t id = vfork();
        if (id < 0)
            printf("Fork error: not created as if <0!");
        else if (id == 0)
        {
            m_forkIDs.push_back(id);
            f();
            _exit(0);
        }
        else {

        }
#endif
	}
	static void WaitAllJobs();
private:
	ThreadPool() {}
	~ThreadPool() {}
#ifdef _WIN32
	static std::vector<std::thread> m_threads;
#else
	static std::vector<pid_t> m_forkIDs;
#endif
};