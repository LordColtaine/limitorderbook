#pragma once
#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <vector>

struct Order
{
    uint64_t m_OrderRefNumber;
    uint16_t m_StockLocate;
    uint32_t m_Shares;
    uint32_t m_Price;
    char m_Side;
    bool m_IsActive;
};

class OrderPool
{
public:
    OrderPool(size_t initialCapacity = 1000000);
    void AddOrder(uint64_t refNum, uint16_t stockLocate, char side, uint32_t shares, uint32_t price);
    Order* GetOrder(uint64_t refNum);
    void ReduceShares(uint64_t refNum, uint32_t removedShares);
    void DeleteOrder(uint64_t refNum);
    size_t GetActiveOrderCount() const;

private:
    std::vector<Order> m_Pool;
    std::vector<size_t> m_FreeSlots;
    std::unordered_map<uint64_t, size_t> m_ReferenceMap;
};