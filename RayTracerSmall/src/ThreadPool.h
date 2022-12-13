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
	static void AddTask(Function_Type f)
	{
		m_threads.push_back(std::thread(f));
	}
	static void WaitAllThreads();
private:
	ThreadPool() {}
	~ThreadPool() {}
#ifdef _WIN32
	static std::vector<std::thread> m_threads;
#else
	static std::vector<pid_t> m_threads;
#endif
};