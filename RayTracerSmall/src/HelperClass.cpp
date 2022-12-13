#include "HelperClass.h"
#include <inttypes.h>
#include <string>

#ifdef _DEBUG

void* operator new (size_t size)
{
	size_t nRequestedBytes = size + sizeof(Header) + sizeof(Footer);
	std::cout << "Default Tracker: Added " << nRequestedBytes << " bytes.\n";
	char* pMem = (char*)malloc(nRequestedBytes);
	MemoryTracker::AddBytesTracker(nRequestedBytes);

	Header* pHeader = (Header*)pMem;
	pHeader->Init();
	MemoryTracker::Insert(pHeader);

	pHeader->memoryType = MemoryType::DEFAULT;
	pHeader->size = size;
	pHeader->checkValue = 0xDEADC0DE;

	void* pFooterAdd = pMem + sizeof(Header) + size;
	Footer* pFooter = (Footer*)pFooterAdd;
	pFooter->checkValue = 0xDEADBEEF;

	void* pStartMemBlock = pMem + sizeof(Header);
	return pStartMemBlock;
}

void* operator new(size_t size, MemoryType tracker)
{
	size_t nRequestedBytes = size + sizeof(Header) + sizeof(Footer);
	char* pMem = (char*)malloc(nRequestedBytes);
	std::cout << enumString[(int)tracker] << " Tracker: Added " << nRequestedBytes << " bytes.\n";

	if (tracker != MemoryType::TRACKER)
		MemoryTracker::AddBytesTracker(nRequestedBytes, tracker);

	Header* pHeader = (Header*)pMem;
	pHeader->Init();
	MemoryTracker::Insert(pHeader);
	pHeader->memoryType = tracker;
	pHeader->size = size;
	pHeader->checkValue = 0xDEADC0DE;

	void* pFooterAdd = pMem + sizeof(Header) + size;
	Footer* pFooter = (Footer*)pFooterAdd;
	pFooter->checkValue = 0xDEADBEEF;

	void* pStartMemBlock = pMem + sizeof(Header);
	return pStartMemBlock;;
}

void operator delete(void* pMem)
{
	Header* pHeader = (Header*)((char*)pMem - sizeof(Header));
	size_t memory = pHeader->size + sizeof(Header) + sizeof(Footer);
	std::cout << "Removed " << memory << " bytes.\n";
	MemoryTracker::RemoveBytesTracker(memory, pHeader->memoryType);
	MemoryTracker::Remove(pHeader);

	free(pHeader);
}

///
/// Tracker Class
///


Tracker::Tracker(MemoryType trackerType)
{
	Initialize(trackerType);
}

void* Tracker::operator new(size_t size)
{
	return ::operator new(size, MemoryType::TRACKER);
}

Tracker* MemoryTracker::GetTracker(MemoryType trackerType)
{
	if (trackerType == MemoryType::DEFAULT)
		return GetDefaultTracker();

	// if the tracker already exists, return it
	for (Tracker* tracker : m_trackers)
	{
		if (tracker->GetType() == trackerType)
			return tracker;
	}

	// if no tracker is found create one
	Tracker* tracker = new Tracker(trackerType);
	m_trackers.push_back(tracker);
	return tracker;
}

///
/// MemoryTracker
///

std::vector<Tracker*> MemoryTracker::m_trackers;
Tracker* MemoryTracker::m_defaultTracker = nullptr;
Header* MemoryTracker::m_pFirstHeader = nullptr;

Tracker* MemoryTracker::GetDefaultTracker()
{
	if (m_defaultTracker)
		return m_defaultTracker;
	
	// if there is no default tracker make one
	m_defaultTracker = new Tracker();
	return m_defaultTracker;
}

void MemoryTracker::AddBytesTracker(size_t bytes, MemoryType trackerType)
{
	GetTracker(trackerType)->AddBytesAllocated(bytes);
}

void MemoryTracker::RemoveBytesTracker(size_t bytes, MemoryType trackerType)
{
	GetTracker(trackerType)->RemovesBytesAllocated(bytes);
}

int MemoryTracker::GetTotalMemory()
{
	if (m_trackers.size() > 0)
	{
		size_t m_total = 0;
		for (Tracker* tracker : m_trackers)
		{
			m_total += tracker->GetAllocatedMemory();
		}

		// check if we have a default tracker
		if (m_defaultTracker)
		{
			m_total += m_defaultTracker->GetAllocatedMemory();
			m_total += (m_trackers.size() + 1) * sizeof(Tracker);
		}
		else
		{
			m_total += m_trackers.size() * sizeof(Tracker);
		}

		return m_total;
	}
	else
	{
		size_t m_total = 0;
		if (m_defaultTracker)
		{
			m_total += m_defaultTracker->GetAllocatedMemory();
			m_total += sizeof(Tracker);
		}
		
		return m_total;
	}
}

void MemoryTracker::Insert(Header* newHeader)
{
	if (m_pFirstHeader == nullptr) {
		m_pFirstHeader = newHeader;
	}
	else {
		Header* currHeader = m_pFirstHeader;
		while (true)
		{
			// find the last header
			Header* nextHeader = currHeader->pNext;
			if (nextHeader == nullptr)
				break; // last header so break out of the while loop
			else
				currHeader = nextHeader;
		}
		currHeader->pNext = newHeader;
		newHeader->pPrev = currHeader;
	}
}

void MemoryTracker::Remove(Header* header)
{
	header->pPrev->pNext = header->pNext;

	if (header->pNext != nullptr)
		header->pNext->pPrev = header->pPrev;
}

void MemoryTracker::WalkTheHeap()
{
	if (!m_pFirstHeader) {
		std::cout << "Heap is empty" << std::endl;
		return;
	}

	// set the first header
	Header* currHeader = m_pFirstHeader;
	while (true) {
		printf("0x%" PRIXPTR "\n", (uintptr_t)currHeader);
		Footer* pFooter = (Footer*)((char*)currHeader + currHeader->size + sizeof(Header));

		// check if the header was overwritten 
		if (currHeader->checkValue != 0xDEADC0DE)
			printf("The header was overwritten\n");

		// check if the footer was overwritten
		if (pFooter->checkValue != 0xDEADBEEF)
			printf("The footer was overwritten\n");

		// check if the we reached the end
		currHeader = currHeader->pNext;
		if (currHeader == nullptr) {
			return;
		}
	}
}

void MemoryTracker::DeleteTrackers()
{
	for (Tracker* tracker : m_trackers)
		delete tracker;

	// delete default tracker
	if (m_defaultTracker) delete m_defaultTracker;

	m_trackers.clear();
}

#endif