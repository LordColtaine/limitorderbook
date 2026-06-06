#include "limitorderbook.h"

ArrayOrderBook::ArrayOrderBook()
    : m_Bids(MAX_PRICE_TICKS, 0), m_Asks(MAX_PRICE_TICKS, 0), m_BestBid(0), m_BestAsk(MAX_PRICE_TICKS)
{
}

void ArrayOrderBook::AddVolume(char side, uint32_t price, uint32_t shares)
{
    if (price >= MAX_PRICE_TICKS)
    {
        return;
    }
    
    if (side == 'B')
    {
        m_Bids[price] += shares;
        if (price > m_BestBid)
        {
            m_BestBid = price;
        }
    }
    else
    {
        m_Asks[price] += shares;
        if (price < m_BestAsk)
        {
            m_BestAsk = price;
        }
    }
}

void ArrayOrderBook::RemoveVolume(char side, uint32_t price, uint32_t shares)
{
    if (price >= MAX_PRICE_TICKS)
    {
        return;
    }

    if (side == 'B')
    {
        m_Bids[price] -= shares;
        if (price == m_BestBid && m_Bids[price] == 0)
        {
            while (m_BestBid > 0 && m_Bids[m_BestBid] == 0)
            {
                --m_BestBid;
            }
        }
    }
    else
    {
        m_Asks[price] -= shares;
        if (price == m_BestAsk && m_Asks[price] == 0)
        {
            while (m_BestAsk < MAX_PRICE_TICKS - 1 && m_Asks[m_BestAsk] == 0)
            {
                ++m_BestAsk;
            }
        }
    }
}

double ArrayOrderBook::CalculateImbalance() const
{
    if (m_BestBid == 0 || m_BestAsk >= MAX_PRICE_TICKS)
    {
        return 0.0;
    }

    const double bidVol = m_Bids[m_BestBid];
    const double askVol = m_Asks[m_BestAsk];
    
    const double sum = bidVol + askVol;
    if (sum == 0)
    {
        return 0.0;
    }

    return (bidVol - askVol) / sum;
}

void ArrayOrderBook::PrintTopOfBook(const std::string& ticker) const
{
    const double bestBidPrint = (m_BestBid > 0) ? (m_BestBid / 10000.0) : 0.0;
    const double bestAskPrint = (m_BestAsk < MAX_PRICE_TICKS) ? (m_BestAsk / 10000.0) : 0.0;
    std::cout << "[HFT MODE] [" << ticker << "] Bid: $" << bestBidPrint << " <--> Ask: $" << bestAskPrint << std::endl;
}

void MapOrderBook::AddVolume(char side, uint32_t price, uint32_t shares)
{
    if (side == 'B')
    {
        m_Bids[price] += shares;
    }
    else
    {
        m_Asks[price] += shares;
    }
}

void MapOrderBook::RemoveVolume(char side, uint32_t price, uint32_t shares)
{
    if (side == 'B')
    {
        m_Bids[price] -= shares;
        if (m_Bids[price] == 0)
        {
            m_Bids.erase(price);
        }
    }
    else
    {
        m_Asks[price] -= shares;
        if (m_Asks[price] == 0)
        {
            m_Asks.erase(price);
        }
    }
}

double MapOrderBook::CalculateImbalance() const
{
    if (m_Bids.empty() || m_Asks.empty())
    {
        return 0.0;
    }

    const double bidVol = m_Bids.begin()->second;
    const double askVol = m_Asks.begin()->second;
    
    const double sum = bidVol + askVol;
    if (sum == 0)
    {
        return 0.0;
    }

    return (bidVol - askVol) / sum;
}

void MapOrderBook::PrintTopOfBook(const std::string& ticker) const
{
    const double bestBid = m_Bids.empty() ? 0.0 : (m_Bids.begin()->first / 10000.0);
    const double bestAsk = m_Asks.empty() ? 0.0 : (m_Asks.begin()->first / 10000.0);
    std::cout << "[DS MODE] [" << ticker << "] Bid: $" << bestBid << " <--> Ask: $" << bestAsk << std::endl;
}