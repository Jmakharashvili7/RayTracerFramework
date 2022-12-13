	#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include <iostream>
#include <fstream>

typedef unsigned char Byte;

typedef struct MemoryChunk
{
	Byte* data;
	size_t dataSize;
	size_t usedSize;
	bool isAllocatedChunk; // check if the chunk is pointing to something
	MemoryChunk* next; // Pointer to the next memoryhunk is
};

#define DEFAULT_MEMORY_POOL_SIZE 1024                                 // Initial MemoryPool size (in Bytes)
#define DEFAULT_MEMORY_CHUNK_SIZE 128                                 // Default MemoryChunkSize (in Bytes)
#define DEFAULT_MEMORY_SIZE_TO_ALLOCATE DEFAULT_MEMORY_CHUNK_SIZE * 2 // Default Memory Size To Allocate (in Bytes)

class MemoryPool
{
public:
	MemoryPool();
	MemoryPool(const size_t& memoryPoolSize, 
			   const size_t& memoryChunkSize,
			   const size_t& minAllocateSize,
			   bool  setMemoryData);

	void* GetMemory(const std::size_t& memorySize);
	void  FreeMemory(void* memoryBlock);
	void  PrintInfo();
private:
	bool AllocateMemory(const size_t& memorySize);
	bool LinkChunksToData(MemoryChunk* newChunks, unsigned int chunkCount, Byte* newMemBlock);
	bool RecalcChunkMemorySize(MemoryChunk* ptrChunk, unsigned int chunkCount);
	void FreeChunks(MemoryChunk* ptrChunk);
	size_t CalculateBestMemorySize(const size_t& memorySize);
	size_t MaxValue(const std::size_t& lValue, const std::size_t& rValue) const;
	unsigned int CalculateNeededChunks(const size_t& memorySize);
	MemoryChunk* InitMemoryChunk(MemoryChunk* memoryChunk);
	MemoryChunk* FindChunk(void* memoryBlock);
	MemoryChunk* FindChunkSuitableToHoldMemory(const size_t& memorySize);
	MemoryChunk* SkipChunks(MemoryChunk* startChunk, unsigned int chunksToSkip);
	void SetMemoryChunkValue(MemoryChunk* ptrChunk, const size_t& memBlockSize);
private:
	// Memory chunks
	MemoryChunk* m_firstChunk;
	MemoryChunk* m_lastChunk;
	MemoryChunk* m_cursorChunk;

	// Pool size
	size_t m_totalMemoryPoolSize;
	size_t m_usedMemoryPoolSize;
	size_t m_freeMemoryPoolSize;
	size_t m_minAllocateSize;
	size_t m_memoryChunkSize;

	unsigned int m_memoryChunkCount;
	unsigned int m_objectCount;

	bool m_setMemoryData;
};

