// Origin 
//#pragma once
//#include <cassert>
//#include <vector>
//#include <iostream>
//#include <numeric>
//
//#include "vector_thl.h"
//#include <xmemory>
//template<typename T,typename Allocator = std::allocator<T>>
//class ObjectPool
//{
//public:
//    ObjectPool() = default;
//    explicit  ObjectPool(const Allocator& allocator);
//    virtual ~ObjectPool();
//
//    // Allow move construction and move assignment
//    ObjectPool(ObjectPool&& src) noexcept = default;
//    ObjectPool& operator=(ObjectPool&& rhs) noexcept = default;
//
//    // Prevent copy construction and copy assignment
//    ObjectPool(const ObjectPool& src) = delete;
//    ObjectPool& operator=(const ObjectPool&& rhs) = delete;
//
//    // Reserves and return an object from the pool. Arguments can be
//    // provided which are perfectly forwarded to a constructor of T
//    template<typename ...Args>
//    std::shared_ptr<T> acquireObject(Args...args);
//
//private:
//    // Contains chunks of memory in witch instance of T will be created.
//    // For each chunk, the pointer to its first object is stored
//    std::vector<T*> m_pool;
//
//    // Contains pointer to all free instances of T that
//    // are available in the pool
//    std::vector<T*> m_freeObjects;
//    // The number of T instanced that should fit in the first allocated chunk
//    static const size_t ms_initialChunkSize{ 5 };
//    // The number of T instanced that should fit in a newly allocated chunk
//    // This value is doubled after each newly created chunk
//    size_t m_newChunkSize{ ms_initialChunkSize };
//    // Creates a new block of uninitialized memory, big enough to hold
//    // m_newChunkSize instances of T
//    void addChunk();
//    // The allocator to use for allocating and deallocating chunks
//    Allocator m_allocator;
//};
//
//template <typename T, typename Allocator>
//ObjectPool<T, Allocator>::ObjectPool(const Allocator& allocator)
//    :m_allocator{allocator}
//{
//
//}
//
//template <typename T, typename Allocator>
//void ObjectPool<T, Allocator>::addChunk()
//{
//    std::cout << "Allocating new chunk...\n";
//
//    // Allocate a new chunk of uninitialized memory big enough to hold
//    // m_newChunkSize instances of T, and add the chunk to the pool.
//    auto* firstNewObject{ m_allocator.allocate(m_newChunkSize) };
//    m_pool.push_back(firstNewObject);
//
//    // Create pointers to each individual object in the new chunk
//    // and store them in the list of free object.
//    size_t oldFreeObjectsSize{ m_freeObjects.size() };
//    m_freeObjects.resize(oldFreeObjectsSize + m_newChunkSize);
//    std::iota(std::begin(m_freeObjects) + oldFreeObjectsSize, std::end(m_freeObjects), firstNewObject);
//
//    // Double the chunk size for next time
//    m_newChunkSize *= 2;
//    std::cout << "Allocating done...\n";
//
//}
//
//template <typename T, typename Allocator>
//template<typename ...Args>
//std::shared_ptr<T> ObjectPool<T,Allocator>::acquireObject(Args... args)
//{
//    // If there are no free object, allocate a new chunk.
//    if (m_freeObjects.empty())
//    {
//        addChunk();
//    }
//
//    std::vector<int> a;
//    // Get a free object.
//    T* object{ m_freeObjects.back() };
//
//    // Initialize, i.e. construct, an instance of T in an
//    // uninitialized block of memory using placement new, and
//    // perfectly forward any provided argument to the constructor
//    new(object) T{ std::forward<Args>(args)...};
//
//    // Remove the object from the list of free objects
//    m_freeObjects.pop_back();
//
//    // Wrap the initialized object and return it
//    return std::shared_ptr<T> {object, [this](T* object)
//        {
//            // Destroy object
//            std::destroy_at(object);
//            // Put the object back in the list of free object
//            m_freeObjects.push_back(object);
//        }};
//}
//
//template <typename T, typename Allocator>
//ObjectPool<T, Allocator>::~ObjectPool()
//{
//    // Note: this implementation assumes that all object handed out by this
//    // pool have been returned to the pool before the pool is destroyed
//    // The following statement asserts if that is not the case
//    assert(m_freeObjects.size() == ms_initialChunkSize * (std::pow(2, m_pool.size()) - 1));
//
//    // Deallocated all allocated memory
//    size_t chunkSize{ ms_initialChunkSize };
//    for (auto* chunk : m_pool)
//    //for (size_t i {0};i<m_pool.size();++i)
//    {
//        //auto* chunk = m_pool.at(i);
//        m_allocator.deallocate(chunk, chunkSize);
//        chunkSize *= 2;
//    }
//    m_pool.clear();
//}
//

// Improved
#pragma once
#include <cassert>
#include <vector>
#include <iostream>
#include <numeric>
#include <mutex>
#include <memory>
#include <atomic>
#include <cmath>
#include "../Logger/LogManager.h"

template<typename T, typename Allocator = std::allocator<T>>
class ObjectPool
{
public:
    ObjectPool() : m_mutex(std::make_shared<std::mutex>()), m_outstandingObjects(0) {}
    explicit ObjectPool(const Allocator& allocator);
    virtual ~ObjectPool();

    // Allow move construction and move assignment
    ObjectPool(ObjectPool&& src) noexcept;
    ObjectPool& operator=(ObjectPool&& rhs) noexcept;

    // Prevent copy construction and copy assignment
    ObjectPool(const ObjectPool& src) = delete;
    ObjectPool& operator=(const ObjectPool& rhs) = delete;

    // Reserves and return an object from the pool. Arguments can be
    // provided which are perfectly forwarded to a constructor of T
    template<typename ...Args>
    std::shared_ptr<T> acquireObject(Args...args);

    // Get the current count of outstanding objects
    size_t getOutstandingObjectCount() const {
        return m_outstandingObjects.load();
    }

private:
    // Contains chunks of memory in which instance of T will be created.
    // For each chunk, the pointer to its first object is stored
    std::vector<T*> m_pool;

    // Contains pointer to all free instances of T that 
    // are available in the pool
    std::vector<T*> m_freeObjects;

    // The number of T instances that should fit in the first allocated chunk
    static const size_t ms_initialChunkSize{ 5 };

    // The number of T instances that should fit in a newly allocated chunk
    // This value is doubled after each newly created chunk
    size_t m_newChunkSize{ ms_initialChunkSize };

    // Creates a new block of uninitialized memory, big enough to hold
    // m_newChunkSize instances of T
    void addChunk();

    // The allocator to use for allocating and deallocating chunks
    Allocator m_allocator;

    // Shared mutex to ensure the deleter can work safely even if the pool is moved
    std::shared_ptr<std::mutex> m_mutex;

    // Track the current count of allocated but not returned objects
    std::atomic<size_t> m_outstandingObjects;
};

template <typename T, typename Allocator>
ObjectPool<T, Allocator>::ObjectPool(const Allocator& allocator)
    : m_allocator{ allocator }, m_mutex(std::make_shared<std::mutex>()), m_outstandingObjects(0)
{
}

template <typename T, typename Allocator>
ObjectPool<T, Allocator>::ObjectPool(ObjectPool&& src) noexcept
    : m_outstandingObjects(0)
{
    std::lock_guard<std::mutex> lock(*src.m_mutex);
    m_pool = std::move(src.m_pool);
    m_freeObjects = std::move(src.m_freeObjects);
    m_newChunkSize = src.m_newChunkSize;
    m_allocator = std::move(src.m_allocator);
    m_mutex = src.m_mutex; // Share the mutex
    m_outstandingObjects.store(src.m_outstandingObjects.load());
    src.m_outstandingObjects.store(0); // Reset the source object's count
}

template <typename T, typename Allocator>
ObjectPool<T, Allocator>& ObjectPool<T, Allocator>::operator=(ObjectPool&& rhs) noexcept
{
    if (this != &rhs) {
        // Use std::lock to lock both mutexes simultaneously, avoiding deadlock
        std::lock(*m_mutex, *rhs.m_mutex);
        std::lock_guard<std::mutex> lockThis(*m_mutex, std::adopt_lock);
        std::lock_guard<std::mutex> lockRhs(*rhs.m_mutex, std::adopt_lock);

        // Release all memory in the current pool
        size_t chunkSize{ ms_initialChunkSize };
        for (auto* chunk : m_pool) {
            m_allocator.deallocate(chunk, chunkSize);
            chunkSize *= 2;
        }

        m_pool = std::move(rhs.m_pool);
        m_freeObjects = std::move(rhs.m_freeObjects);
        m_newChunkSize = rhs.m_newChunkSize;
        m_allocator = std::move(rhs.m_allocator);
        m_outstandingObjects.store(rhs.m_outstandingObjects.load());
        rhs.m_outstandingObjects.store(0); // Reset the source object's count
    }
    return *this;
}

template <typename T, typename Allocator>
void ObjectPool<T, Allocator>::addChunk()
{
    // Note: This method assumes the mutex is already acquired
    std::cout << "Allocating new chunk...\n";

    // Allocate a new chunk of uninitialized memory big enough to hold
    // m_newChunkSize instances of T, and add the chunk to the pool.
    auto* firstNewObject{ m_allocator.allocate(m_newChunkSize) };
    m_pool.push_back(firstNewObject);

    // Create pointers to each individual object in the new chunk
    // and store them in the list of free objects.
    size_t oldFreeObjectsSize{ m_freeObjects.size() };
    m_freeObjects.resize(oldFreeObjectsSize + m_newChunkSize);
    std::iota(std::begin(m_freeObjects) + oldFreeObjectsSize, std::end(m_freeObjects), firstNewObject);

    // Double the chunk size for next time
    m_newChunkSize *= 2;
    std::cout << "Allocating done...\n";
}

template <typename T, typename Allocator>
template<typename ...Args>
std::shared_ptr<T> ObjectPool<T, Allocator>::acquireObject(Args... args)
{
    T* object = nullptr;

    // Use mutex to protect the object acquisition process
    {
        std::lock_guard<std::mutex> lock(*m_mutex);

        // If there are no free objects, allocate a new chunk.
        if (m_freeObjects.empty())
        {
            addChunk();
        }

        // Get a free object.
        object = m_freeObjects.back();

        // Remove the object from the list of free objects
        m_freeObjects.pop_back();

        // Increment the outstanding objects count
        m_outstandingObjects++;
    }

    // Construct the object outside the lock to reduce lock holding time
    // Initialize, i.e. construct, an instance of T in an
    // uninitialized block of memory using placement new, and
    // perfectly forward any provided argument to the constructor
    new(object) T{ std::forward<Args>(args)... };

    // Capture the shared mutex and object pointer
    auto sharedMutex = m_mutex;
    auto poolPtr = this;
    auto deleter = [sharedMutex, poolPtr, object](T* ptr) {
        // Destroy object
        std::destroy_at(ptr);

        // Use shared mutex to protect the object return process
        std::lock_guard<std::mutex> lock(*sharedMutex);

        // Put the object back in the list of free objects
        poolPtr->m_freeObjects.push_back(object);

        // Decrement the outstanding objects count
        poolPtr->m_outstandingObjects--;
        };

    // Wrap the initialized object and return it
    return std::shared_ptr<T>(object, deleter);
}

template <typename T, typename Allocator>
ObjectPool<T, Allocator>::~ObjectPool()
{
    std::lock_guard<std::mutex> lock(*m_mutex);

    // Check if there are any outstanding objects
    size_t outstanding = m_outstandingObjects.load();
    if (outstanding > 0) {
        const std::string message = "Warning: ObjectPool destroyed with " +
            std::to_string(outstanding) + 
            " outstanding objects that have not been returned to the pool.\n";
        std::cerr << message;
        LogWarning(message);
    }

    // Calculate how many objects should be in the pool (if all objects were returned)
    size_t expectedFreeObjects = ms_initialChunkSize * (std::pow(2, m_pool.size()) - 1) - outstanding;

    // Verify that the number of free objects matches the expected count
    assert(m_freeObjects.size() == expectedFreeObjects);

    // Deallocate all allocated memory
    size_t chunkSize{ ms_initialChunkSize };
    for (auto* chunk : m_pool)
    {
        m_allocator.deallocate(chunk, chunkSize);
        chunkSize *= 2;
    }
    m_pool.clear();
}