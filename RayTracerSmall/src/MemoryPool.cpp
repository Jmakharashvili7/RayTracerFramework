#include "MemoryPool.h"

// constructor with default values
MemoryPool::MemoryPool()
{
	m_firstChunk = nullptr;
	m_lastChunk = nullptr;
	m_cursorChunk = nullptr;

	m_totalMemoryPoolSize = 0;
	m_usedMemoryPoolSize = 0;
	m_freeMemoryPoolSize = 0;

	m_memoryChunkSize = DEFAULT_MEMORY_CHUNK_SIZE;
	m_memoryChunkCount = 0;
	m_objectCount = 0;

	m_setMemoryData = false;
	m_minAllocateSize = DEFAULT_MEMORY_SIZE_TO_ALLOCATE;

	// allocate the initial memory from the os
	AllocateMemory(DEFAULT_MEMORY_POOL_SIZE);
}

// custom values constructor
MemoryPool::MemoryPool(const size_t& memoryPoolSize, const size_t& memoryChunkSize, const size_t& minAllocateSize, bool setMemoryData)
{
	m_firstChunk = nullptr;
	m_lastChunk = nullptr;
	m_cursorChunk = nullptr;

	m_totalMemoryPoolSize = 0;
	m_usedMemoryPoolSize = 0;
	m_freeMemoryPoolSize = 0;

	m_memoryChunkSize = memoryChunkSize;
	m_memoryChunkCount = 0;
	m_objectCount = 0;

	m_setMemoryData = setMemoryData;
	m_minAllocateSize = minAllocateSize;

	// allocate the initial memory from the os
	AllocateMemory(memoryPoolSize);
}



void* MemoryPool::GetMemory(const std::size_t& memorySize)
{
	std::size_t sBestMemBlockSize = CalculateBestMemorySize(memorySize);

	MemoryChunk* ptrChunk = nullptr;
	while (!ptrChunk)
	{
		// Is a Chunks available to hold the requested amount of Memory ?
		ptrChunk = FindChunkSuitableToHoldMemory(sBestMemBlockSize);
		if(!ptrChunk)
		{
			// No chunk can be found
			// => Memory-Pool is to small. We have to request 
			//    more Memory from the Operating-System....
			sBestMemBlockSize = MaxValue(sBestMemBlockSize, CalculateBestMemorySize(m_minAllocateSize));
			AllocateMemory(sBestMemBlockSize);
			std::cout << m_totalMemoryPoolSize << std::endl;
		}
	}

	// Finally, a suitable Chunk was found.
	// Adjust the Values of the internal "TotalSize"/"UsedSize" Members and 
	// the Values of the MemoryChunk itself.
	m_usedMemoryPoolSize += sBestMemBlockSize;
	m_freeMemoryPoolSize -= sBestMemBlockSize;
	m_objectCount++;
	SetMemoryChunkValue(ptrChunk, sBestMemBlockSize);

	// eventually, return the Pointer to the User
	return ((void*)ptrChunk->data);

}

void MemoryPool::FreeMemory(void* memoryBlock)
{
	MemoryChunk* ptrChunk = FindChunk(memoryBlock);
	if (ptrChunk)
	{
		FreeChunks(ptrChunk);
	}
	else
	{
		assert(false && "Error: Pointer not in memory pool!");
	}
	assert((m_objectCount>0) && ("Error: Requesting to delete more memory than allocated!"));
	m_objectCount--;
}

void MemoryPool::PrintInfo()
{
	std::cout << "Memory Pool info: " << std::endl;
	std::cout << "Total Size: " << m_totalMemoryPoolSize << std::endl;
	std::cout << "Total Used Memory: " << m_usedMemoryPoolSize << std::endl;
	std::cout << "Total Free Memory: " << m_freeMemoryPoolSize << std::endl;
	std::cout << "Total Objects: " << m_objectCount << std::endl;
}

bool MemoryPool::AllocateMemory(const size_t& memorySize)
{
	unsigned int neededChunks = CalculateNeededChunks(memorySize);
	size_t bestMemBlockSize = CalculateBestMemorySize(memorySize);

	// allocate memory from the os
	Byte* newMemBlock = (Byte*) malloc(bestMemBlockSize);
	MemoryChunk* newChunks = (MemoryChunk*) malloc((neededChunks * sizeof(MemoryChunk)));

	m_totalMemoryPoolSize += bestMemBlockSize;
	m_freeMemoryPoolSize += bestMemBlockSize;
	m_memoryChunkCount += neededChunks;

	return LinkChunksToData(newChunks, neededChunks, newMemBlock);
}

bool MemoryPool::LinkChunksToData(MemoryChunk* newChunks, unsigned int neededChunks, Byte* newMemBlock)
{
	MemoryChunk* newChunk = nullptr;
	unsigned int memOffSet = 0;
	bool allocationChunkAssigned = false;
	for (unsigned int i = 0; i < neededChunks; i++)
	{
		if (!m_firstChunk)
		{
			m_firstChunk = InitMemoryChunk(&(newChunks[0]));
			m_lastChunk = m_firstChunk;
			m_cursorChunk = m_firstChunk;
		}
		else
		{
			newChunk = InitMemoryChunk(&(newChunks[i]));
			m_lastChunk->next = newChunk;
			m_lastChunk = newChunk;
		}

		memOffSet = (i * ((unsigned int)m_memoryChunkSize));
		m_lastChunk->data = &(newMemBlock[memOffSet]);

		if (!allocationChunkAssigned)
		{
			m_lastChunk->isAllocatedChunk = true;
			allocationChunkAssigned = true;
		}
	}

	return RecalcChunkMemorySize(m_firstChunk, m_memoryChunkCount);
}

bool MemoryPool::RecalcChunkMemorySize(MemoryChunk* chunk, unsigned int chunkCount)
{
	unsigned int memOffset = 0;
	for (unsigned int i = 0; i < chunkCount; i++)
	{
		if (chunk)
		{
			memOffset = (i * ((unsigned int)m_memoryChunkSize));
			chunk->dataSize = (((unsigned int)m_totalMemoryPoolSize) - memOffset);
			chunk = chunk->next;
		}
		else
		{
			assert(false && "Error: Chunk == nullptr");
			return false;
		}
	}
	return true;
}

void MemoryPool::FreeChunks(MemoryChunk* ptrChunk)
{
	MemoryChunk* ptrCurrentChunk = ptrChunk;
	unsigned int chunkCount = CalculateNeededChunks(ptrCurrentChunk->usedSize);

	for (unsigned int i = 0; i < chunkCount; i++)
	{
		if (ptrCurrentChunk)
		{
			ptrCurrentChunk->usedSize = 0;

			m_usedMemoryPoolSize -= m_memoryChunkSize;
			m_freeMemoryPoolSize += m_memoryChunkSize;
			ptrCurrentChunk = ptrCurrentChunk->next;
		}
	}
}

size_t MemoryPool::CalculateBestMemorySize(const size_t& memorySize)
{
	unsigned int neededChunks = CalculateNeededChunks(memorySize);
	return size_t(neededChunks * m_memoryChunkSize);
}

unsigned int MemoryPool::CalculateNeededChunks(const size_t& memorySize)
{
	float f = (float)(((float)memorySize) / ((float)m_memoryChunkSize));
	return ((unsigned int)ceil(f));
}

MemoryChunk* MemoryPool::InitMemoryChunk(MemoryChunk* memoryChunk)
{
	if (memoryChunk)
	{
		memoryChunk->data = nullptr;
		memoryChunk->dataSize = 0;
		memoryChunk->isAllocatedChunk = false;
		memoryChunk->next = nullptr;
		memoryChunk->usedSize = 0;
	}
	return memoryChunk;
}

MemoryChunk* MemoryPool::FindChunk(void* memoryBlock)
{
	MemoryChunk* tempChunk = m_firstChunk;
	while (tempChunk)
	{
		if (tempChunk->data == ((Byte*)memoryBlock))
			break;

		tempChunk = tempChunk->next;
	}
	return tempChunk;
}

MemoryChunk* MemoryPool::FindChunkSuitableToHoldMemory(const size_t& memorySize)
{
	// Find a Chunk to hold *at least* "sMemorySize" Bytes.
    unsigned int uiChunksToSkip = 0;
    bool bContinueSearch = true;
    MemoryChunk* ptrChunk = m_cursorChunk; // Start search at Cursor-Pos.
    for (unsigned int i = 0; i < m_memoryChunkCount; i++)
    {
		if (ptrChunk)
        {
  			if (ptrChunk == m_lastChunk) // End of List reached : Start over from the beginning
			{
				ptrChunk = m_firstChunk;
			}
	
			if (ptrChunk->dataSize >= memorySize)
			{
				if (ptrChunk->usedSize == 0)
			    {
	  				m_cursorChunk = ptrChunk;
			        return ptrChunk;
			    }
			}

			uiChunksToSkip = CalculateNeededChunks(ptrChunk->usedSize);
  			if (uiChunksToSkip == 0) uiChunksToSkip = 1;
			ptrChunk = SkipChunks(ptrChunk, uiChunksToSkip);
  		}
		else
  		{
			bContinueSearch = false;
		}
    }
    return nullptr;
}

// return higher value
size_t MemoryPool::MaxValue(const std::size_t& lValue, const std::size_t& rValue) const
{
	if (lValue > rValue)
	{
		return lValue;
	}
	return rValue;
}

MemoryChunk* MemoryPool::SkipChunks(MemoryChunk* startChunk, unsigned int chunksToSkip)
{
	MemoryChunk* ptrCurrentChunk = startChunk;
	for (unsigned int i = 0; i < chunksToSkip; i++) {
		if (ptrCurrentChunk)
		{
			ptrCurrentChunk = ptrCurrentChunk->next;
		}
		else
		{
			assert(false && "Error: Chunk == nullptr not expected!");
			break;
		}
	}

	return ptrCurrentChunk;
}

void MemoryPool::SetMemoryChunkValue(MemoryChunk* ptrChunk, const size_t& memBlockSize)
{
	if (ptrChunk && ptrChunk != m_lastChunk)
	{
		ptrChunk->usedSize = memBlockSize;
	}
	else
	{
		assert(false && "Error: Invalid nullptr passed");
	}
}
