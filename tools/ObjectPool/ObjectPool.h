#pragma once
#include <cassert>
#include <vector>
#include <iostream>
#include <numeric>

#include "vector_thl.h"
#include <xmemory>
template<typename T,typename Allocator = std::allocator<T>>
class ObjectPool
{
public:
	ObjectPool() = default;
	explicit  ObjectPool(const Allocator& allocator);
	virtual ~ObjectPool();

	// Allow move construction and move assignment
	ObjectPool(ObjectPool&& src) noexcept = default;
	ObjectPool& operator=(ObjectPool&& rhs) noexcept = default;

	// Prevent copy construction and copy assignment
	ObjectPool(const ObjectPool& src) = delete;
	ObjectPool& operator=(const ObjectPool&& rhs) = delete;

	// Reserves and return an object from the pool. Arguments can be
	// provided which are perfectly forwarded to a constructor of T
	template<typename ...Args>
	std::shared_ptr<T> acquireObject(Args...args);

private:
	// Contains chunks of memory in witch instance of T will be created.
	// For each chunk, the pointer to its first object is stored
	std::vector<T*> m_pool;

	// Contains pointer to all free instances of T that 
	// are available in the pool
	std::vector<T*> m_freeObjects;
	// The number of T instanced that should fit in the first allocated chunk
	static const size_t ms_initialChunkSize{ 5 };
	// The number of T instanced that should fit in a newly allocated chunk
	// This value is doubled after each newly created chunk
	size_t m_newChunkSize{ ms_initialChunkSize };
	// Creates a new block of uninitialized memory, big enough to hold
	// m_newChunkSize instances of T
	void addChunk();
	// The allocator to use for allocating and deallocating chunks
	Allocator m_allocator;
};

template <typename T, typename Allocator>
ObjectPool<T, Allocator>::ObjectPool(const Allocator& allocator)
	:m_allocator{allocator}
{
	
}

template <typename T, typename Allocator>
void ObjectPool<T, Allocator>::addChunk()
{
	std::cout << "Allocating new chunk...\n";

	// Allocate a new chunk of uninitialized memory big enough to hold
	// m_newChunkSize instances of T, and add the chunk to the pool.
	auto* firstNewObject{ m_allocator.allocate(m_newChunkSize) };
	m_pool.push_back(firstNewObject);

	// Create pointers to each individual object in the new chunk
	// and store them in the list of free object.
	size_t oldFreeObjectsSize{ m_freeObjects.size() };
	m_freeObjects.resize(oldFreeObjectsSize + m_newChunkSize);
	std::iota(std::begin(m_freeObjects) + oldFreeObjectsSize, std::end(m_freeObjects), firstNewObject);

	// Double the chunk size for next time
	m_newChunkSize *= 2;
	std::cout << "Allocating done...\n";

}

template <typename T, typename Allocator>
template<typename ...Args>
std::shared_ptr<T> ObjectPool<T,Allocator>::acquireObject(Args... args)
{
	// If there are no free object, allocate a new chunk.
	if (m_freeObjects.empty())
	{
		addChunk();
	}

	std::vector<int> a;
	// Get a free object.
	T* object{ m_freeObjects.back() };

	// Initialize, i.e. construct, an instance of T in an
	// uninitialized block of memory using placement new, and
	// perfectly forward any provided argument to the constructor
	new(object) T{ std::forward<Args>(args)...};

	// Remove the object from the list of free objects
	m_freeObjects.pop_back();

	// Wrap the initialized object and return it
	return std::shared_ptr<T> {object, [this](T* object)
		{
			// Destroy object
			std::destroy_at(object);
			// Put the object back in the list of free object
			m_freeObjects.push_back(object);
		}};
}

template <typename T, typename Allocator>
ObjectPool<T, Allocator>::~ObjectPool()
{
	// Note: this implementation assumes that all object handed out by this
	// pool have been returned to the pool before the pool is destroyed
	// The following statement asserts if that is not the case
	assert(m_freeObjects.size() == ms_initialChunkSize * (std::pow(2, m_pool.size()) - 1));

	// Deallocated all allocated memory
	size_t chunkSize{ ms_initialChunkSize };
	for (auto* chunk : m_pool)
	//for (size_t i {0};i<m_pool.size();++i)
	{
		//auto* chunk = m_pool.at(i);
		m_allocator.deallocate(chunk, chunkSize);
		chunkSize *= 2;
	}
	m_pool.clear();
}



