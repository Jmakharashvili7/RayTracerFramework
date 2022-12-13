#pragma once

#include <iostream>
#include <mutex>
#include <vector>
#include "MemoryPool.h"

#ifdef _DEBUG

enum class MemoryType
{
    DEFAULT,
    GRAPHICS,
    SOUND,
    TRACKER
};

static const char* enumString[] = { "Default", "Graphics", "Sound", "Tracker" };

void* operator new(size_t size);
void* operator new(size_t size, MemoryType type);
void operator delete (void* pMem);

struct Header
{
    void Init() { pNext = nullptr; pPrev = nullptr; size = 0; checkValue = 0; memoryType = MemoryType::DEFAULT; }
    int size; // stores the size of the main allocated section
    int checkValue; 
    MemoryType memoryType; // type of memory
    Header* pNext;
    Header* pPrev;
};

struct Footer
{
    int reserved; // we can use this later
    int checkValue;
};

class Tracker
{
public:
    Tracker(MemoryType trackerType);
    Tracker() { Initialize(); }
    ~Tracker() {}

    inline void         Initialize(MemoryType trackerType = MemoryType::DEFAULT) { m_allocatedMemory = 0; m_type = trackerType; }
    inline void         AddBytesAllocated(size_t addedBytes) { m_allocatedMemory += addedBytes; }
    inline void         RemovesBytesAllocated(size_t removedBytes) { if (removedBytes > m_allocatedMemory) m_allocatedMemory = 0; 
                                                                  else m_allocatedMemory -= removedBytes; }
    inline int          GetAllocatedMemory() { return m_allocatedMemory; }
    inline MemoryType   GetType() { return m_type; }
    void* operator new(size_t size);
private:
    MemoryType m_type;
    size_t m_allocatedMemory;
};

class MemoryTracker
{
public:
    static void*    GetMemory(const size_t& memSize);
    static void     FreeMemory(void* memBlock, const size_t& memBlockSize);
    static void     PrintInfo();
    static void     AddBytesTracker(size_t bytes, MemoryType memoryType = MemoryType::DEFAULT);
    static void     RemoveBytesTracker(size_t bytes, MemoryType memoryType = MemoryType::DEFAULT);
    static void     DeleteTrackers();
    static int      GetTotalMemory();
    static void     Insert(Header* newHeader);
    static void     Remove(Header* header);
    static void     WalkTheHeap();
    inline static Header*  GetFirstHeader() { return m_pFirstHeader; }
    inline static void     SetFirstHeader(Header* firstHeader) { m_pFirstHeader = firstHeader; m_pFirstHeader->pNext = nullptr; m_pFirstHeader->pPrev = nullptr; }
private: // functions
    MemoryTracker() {}
    ~MemoryTracker() {}

    static Tracker* GetTracker(MemoryType memoryType);
    static Tracker* GetDefaultTracker();
private: // variables
    static std::vector<Tracker*> m_trackers;
    static Tracker* m_defaultTracker;
    static Header* m_pFirstHeader;
};

#endif