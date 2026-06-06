#include "orderpool.h"

OrderPool::OrderPool(size_t initialCapacity)
{
    m_Pool.reserve(initialCapacity);
    m_FreeSlots.reserve(initialCapacity / 10);
    m_ReferenceMap.reserve(initialCapacity);
    std::cout << "Order Pool Initialized: Pre-allocated capacity for " << initialCapacity << " orders.\n";
}

void OrderPool::AddOrder(uint64_t refNum, uint16_t stockLocate, char side, uint32_t shares, uint32_t price)
{
    size_t index;
    if (!m_FreeSlots.empty())
    {
        index = m_FreeSlots.back();
        m_FreeSlots.pop_back();
        m_Pool[index] = {refNum, stockLocate, shares, price, side, true};
    }
    else
    {
        index = m_Pool.size();
        m_Pool.push_back({refNum, stockLocate, shares, price, side, true});
    }
    
    m_ReferenceMap[refNum] = index;
}

Order* OrderPool::GetOrder(uint64_t refNum)
{
    auto it = m_ReferenceMap.find(refNum);
    if (it != m_ReferenceMap.end())
    {
        return &m_Pool[it->second];
    }
    
    return nullptr;
}

void OrderPool::ReduceShares(uint64_t refNum, uint32_t removedShares)
{
    auto it = m_ReferenceMap.find(refNum);
    if (it != m_ReferenceMap.end())
    {
        m_Pool[it->second].m_Shares -= removedShares;
    }
}

void OrderPool::DeleteOrder(uint64_t refNum)
{
    auto it = m_ReferenceMap.find(refNum);
    if (it != m_ReferenceMap.end())
    {
        size_t index = it->second;
        m_Pool[index].m_IsActive = false;
        m_FreeSlots.push_back(index);
        m_ReferenceMap.erase(it);
    }
}

size_t OrderPool::GetActiveOrderCount() const { return m_ReferenceMap.size(); }