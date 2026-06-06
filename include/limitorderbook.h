#pragma once
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <vector>

// Support prices up to $500.00
constexpr uint32_t MAX_PRICE_TICKS = 5000000;

// ==========================================
//  Data-oriented
// ==========================================
class ArrayOrderBook
{
public:
    ArrayOrderBook();

    void AddVolume(char side, uint32_t price, uint32_t shares);
    void RemoveVolume(char side, uint32_t price, uint32_t shares);

    double CalculateImbalance() const;
    void PrintTopOfBook(const std::string& ticker) const;

private:
    std::vector<uint32_t> m_Bids;
    std::vector<uint32_t> m_Asks;
    uint32_t m_BestBid;
    uint32_t m_BestAsk;
};

// ==========================================
// Map
// ==========================================
class MapOrderBook
{
public:
    void AddVolume(char side, uint32_t price, uint32_t shares);
    void RemoveVolume(char side, uint32_t price, uint32_t shares);

    double CalculateImbalance() const;
    void PrintTopOfBook(const std::string& ticker) const;

private:
    std::map<uint32_t, uint64_t, std::greater<uint32_t>> m_Bids;
    std::map<uint32_t, uint64_t> m_Asks;
};

// ==========================================
// Toggle
// ==========================================
#define LOW_LATENCY_MODE

#ifdef LOW_LATENCY_MODE
using LimitOrderBook = ArrayOrderBook;
#else
using LimitOrderBook = MapOrderBook;
#endif