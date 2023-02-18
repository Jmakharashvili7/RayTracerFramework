#pragma once

#include <stdlib.h>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <vector>
#include <iostream>
#include <cassert>
#include "ThreadPool.h"
#include "Sphere.h"
#include "Vector3.h"
#include "chrono"
#include "MemoryPool.h"
#include "JsonSphere.h"
#include "HelperClass.h"
#ifdef _WIN32
#include <algorithm>
#include <sstream>
#include <string.h>
#endif

#define uint unsigned int

//[comment]
// This variable controls the maximum recursion depth
//[/comment]
#define MAX_RAY_DEPTH 5
//[comment]
// Defined values to easily adjust threading for testing.
// SS_THREAD_COUNT = number of threads to use for smooth scaling function
// REND_THREAD_COUNT = number of threads to run trace function on in render
// FRAME_COUNT = number of frames to render
// SS_USE_THREAD = use threads for smooth scaling if defined
// REND_USE_THREADS = use threads for render if defined
//[/comment]
#ifdef _WIN32
#define SS_THREAD_COUNT std::thread::hardware_concurrency()
#define REND_THREAD_COUNT std::thread::hardware_concurrency()
#else
#define SS_THREAD_COUNT 10
#define REND_THREAD_COUNT 10
#endif

#define SPHERE_COUNT 4
#define FRAME_COUNT 120
#define SS_USE_THREADS
#define REND_USE_THREADSS


#if defined __linux__ || defined __APPLE__
// "Compiled for Linux
#else
// Windows doesn't define these values by default, Linux does
#define M_PI 3.141592653589793
#define INFINITY 1e8
#endif