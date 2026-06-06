#include "ringbuffer.h"

SPSCRingBuffer::SPSCRingBuffer(size_t powerOfTwoSize)
    : m_Buffer(powerOfTwoSize), m_Head(0), m_Tail(0), m_Mask(powerOfTwoSize - 1)
{
}

bool SPSCRingBuffer::Push(const InternalEvent& item)
{
    const size_t currentTail = m_Tail.load(std::memory_order_relaxed);
    const size_t nextTail = (currentTail + 1) & m_Mask;

    // full
    if (nextTail == m_Head.load(std::memory_order_acquire))
    {
        return false;
    }

    m_Buffer[currentTail] = item;
    m_Tail.store(nextTail, std::memory_order_release);
    return true;
}

bool SPSCRingBuffer::Pop(InternalEvent& item)
{
    const size_t currentHead = m_Head.load(std::memory_order_relaxed);

    // Empty
    if (currentHead == m_Tail.load(std::memory_order_acquire))
    {
        return false;
    }

    item = m_Buffer[currentHead];
    m_Head.store((currentHead + 1) & m_Mask, std::memory_order_release);
    return true;
}