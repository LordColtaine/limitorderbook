#pragma once
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <vector>

struct InternalEvent
{
    char m_MessageType; // 'A', 'E', 'X', 'D'
    uint16_t m_StockLocate;
    uint64_t m_OrderRefNum;
    char m_Side; // 'B' or 'S'
    uint32_t m_Shares;
    uint32_t m_Price;
};

// A Lock-Free SPSC Circular Queue
class SPSCRingBuffer
{
private:
    std::vector<InternalEvent> m_Buffer;
    std::atomic<size_t> m_Head;
    std::atomic<size_t> m_Tail;
    size_t m_Mask;

public:
    // Capacity must be a power of 2 for the ultra-fast bitwise mask to work
    SPSCRingBuffer(size_t powerOfTwoSize);

    bool Push(const InternalEvent& item);
    bool Pop(InternalEvent& item);
};