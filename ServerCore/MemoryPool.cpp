#include "pch.h"
#include "MemoryPool.h"

MemoryPool::MemoryPool(int32 _allocSize) : m_allocSize(_allocSize)
{
	::InitializeSListHead(&m_header);
}

MemoryPool::~MemoryPool()
{
	while (MemoryHeader* memory = static_cast<MemoryHeader*>(
		::InterlockedPopEntrySList(&m_header)))
	{
		// null�� �ƴϸ�
		::_aligned_free(memory);
	}

	/*while (m_queue.empty() == false)
	{
		MemoryHeader* header = m_queue.front();
		m_queue.pop();
		::free(header);
	}*/
}

void MemoryPool::Push(MemoryHeader* _ptr)
{
	//WRITE_LOCK;
	_ptr->allocSize = 0;

	// Pool�� �޸� �ݳ�
	//m_queue.push(_ptr);
	::InterlockedPushEntrySList(&m_header, 
		static_cast<PSLIST_ENTRY>(_ptr));

	m_useCount.fetch_sub(1);
	m_reservedCount.fetch_add(1);
}

MemoryHeader* MemoryPool::Pop()
{
	MemoryHeader* memory = static_cast<MemoryHeader*>(
		::InterlockedPopEntrySList(&m_header));

	/*{
		WRITE_LOCK;

		//Pool�� ������ �ִ���?
		if (m_queue.empty() == false)
		{
			//������ �ϳ� �����´�.
			header = m_queue.front();
			m_queue.pop();
		}
	}*/

	// ������ ���� �Ҵ�
	if (memory == nullptr)
	{
		memory = reinterpret_cast<MemoryHeader*>(
			::_aligned_malloc(m_allocSize, SLIST_ALIGNMENT));
	}
	else
	{
		ASSERT_CRASH(memory->allocSize == 0);
		m_reservedCount.fetch_sub(1);
	}

	m_useCount.fetch_add(1);

	return memory;
}