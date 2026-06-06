#include <atomic>
#include <bit>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "limitorderbook.h"
#include "messagetypes.h"
#include "orderpool.h"
#include "ringbuffer.h"

// --- Global Thread Sync Flags ---
std::atomic<bool> g_EngineRunning{true};
std::atomic<uint16_t> g_TargetLocate{0}; // don't have enough ram, so only track apple.

inline uint16_t Swap16(uint16_t val)
{
    if constexpr (std::endian::native == std::endian::little)
    {
        return (val << 8) | (val >> 8);
    }

    return val;
}

inline uint32_t Swap32(uint32_t val)
{
    if constexpr (std::endian::native == std::endian::little)
    {
        return __builtin_bswap32(val);
    }

    return val;
}

inline uint64_t Swap64(uint64_t val)
{
    if constexpr (std::endian::native == std::endian::little)
    {
        return __builtin_bswap64(val);
    }

    return val;
}

inline bool ShouldProcessTrade(uint16_t currentLocate, uint16_t targetLocate)
{
#ifdef LOW_LATENCY_MODE
    return (targetLocate != 0 && currentLocate == targetLocate);
#else
    return true;
#endif
}

// Producer
void NetworkProducer(SPSCRingBuffer& ringBuffer, const std::string& fileName)
{
    std::ifstream fileStream(fileName, std::ios::binary);
    if (!fileStream.is_open())
    {
        std::cerr << "CRITICAL: Failed to open NASDAQ file." << std::endl;
        g_EngineRunning.store(false, std::memory_order_release);
        return;
    }

    std::vector<std::string> tickerArray(65536, "");
    uint32_t messagesProcessed = 0;

    while (messagesProcessed < 20000000)
    {
        uint16_t messageLength;
        if (!fileStream.read(reinterpret_cast<char*>(&messageLength), sizeof(messageLength)))
        {
            break;
        }

        messageLength = Swap16(messageLength);
        char messageType;
        fileStream.read(&messageType, sizeof(messageType));

        InternalEvent ev;
        ev.m_MessageType = messageType;

        if (messageType == 'R')
        {
            StockDirectoryMessage msg;
            fileStream.read(reinterpret_cast<char*>(&msg) + 1, messageLength - 1);
            uint16_t locate = Swap16(msg.m_StockLocate);
            tickerArray[locate] = std::string(msg.m_Stock, 8);
            if (tickerArray[locate] == "AAPL    ")
            {
                g_TargetLocate.store(locate, std::memory_order_release);
            }
        }
        else if (messageType == 'S')
        {
            fileStream.seekg(messageLength - 1, std::ios::cur);
        }
        else if (messageType == 'A')
        {
            AddOrderMessage msg;
            fileStream.read(reinterpret_cast<char*>(&msg) + 1, messageLength - 1);
            uint16_t locate = Swap16(msg.m_StockLocate);

            if (ShouldProcessTrade(locate, g_TargetLocate.load(std::memory_order_relaxed)))
            {
                ev.m_StockLocate = locate;
                ev.m_OrderRefNum = Swap64(msg.m_OrderReferenceNumber);
                ev.m_Side = msg.m_BuySellIndicator;
                ev.m_Shares = Swap32(msg.m_Shares);
                ev.m_Price = Swap32(msg.m_Price);

                while (!ringBuffer.Push(ev))
                {
                    std::this_thread::yield();
                }
            }
        }
        else if (messageType == 'E')
        {
            OrderExecutedMessage msg;
            fileStream.read(reinterpret_cast<char*>(&msg) + 1, messageLength - 1);
            uint16_t locate = Swap16(msg.m_StockLocate);

            if (ShouldProcessTrade(locate, g_TargetLocate.load(std::memory_order_relaxed)))
            {
                ev.m_OrderRefNum = Swap64(msg.m_OrderReferenceNumber);
                ev.m_Shares = Swap32(msg.m_ExecutedShares);
                while (!ringBuffer.Push(ev))
                {
                    std::this_thread::yield();
                }
            }
        }
        else if (messageType == 'X')
        {
            OrderCanceledMessage msg;
            fileStream.read(reinterpret_cast<char*>(&msg) + 1, messageLength - 1);
            uint16_t locate = Swap16(msg.m_StockLocate);

            if (ShouldProcessTrade(locate, g_TargetLocate.load(std::memory_order_relaxed)))
            {
                ev.m_OrderRefNum = Swap64(msg.m_OrderReferenceNumber);
                ev.m_Shares = Swap32(msg.m_CanceledShares);
                while (!ringBuffer.Push(ev))
                {
                    std::this_thread::yield();
                }
            }
        }
        else if (messageType == 'D')
        {
            OrderDeletedMessage msg;
            fileStream.read(reinterpret_cast<char*>(&msg) + 1, messageLength - 1);
            uint16_t locate = Swap16(msg.m_StockLocate);

            if (ShouldProcessTrade(locate, g_TargetLocate.load(std::memory_order_relaxed)))
            {
                ev.m_OrderRefNum = Swap64(msg.m_OrderReferenceNumber);
                while (!ringBuffer.Push(ev))
                {
                    std::this_thread::yield();
                }
            }
        }
        else
        {
            fileStream.seekg(messageLength - 1, std::ios::cur);
        }

        messagesProcessed++;

        // Send a Control Message to the Logic Thread to trigger a screen update
        if (messagesProcessed % 1000000 == 0)
        {
            InternalEvent hb;
            hb.m_MessageType = 'H';
            hb.m_Shares = messagesProcessed / 1000000;
            while (!ringBuffer.Push(hb))
            {
                std::this_thread::yield();
            }
        }
    }

    std::cout << "--- File Fully Processed. Signaling Shutdown ---" << std::endl;
    g_EngineRunning.store(false, std::memory_order_release);
}

// consumer
void LogicConsumer(SPSCRingBuffer& ringBuffer)
{
    OrderPool orderPool(5000000);
    std::unordered_map<uint16_t, LimitOrderBook> marketBooks;

    InternalEvent ev;
    while (g_EngineRunning.load(std::memory_order_acquire) || ringBuffer.Pop(ev))
    {
        if (!ringBuffer.Pop(ev))
        {
            continue;
        }

        if (ev.m_MessageType == 'A')
        {
            orderPool.AddOrder(ev.m_OrderRefNum, ev.m_StockLocate, ev.m_Side, ev.m_Shares, ev.m_Price);
            marketBooks[ev.m_StockLocate].AddVolume(ev.m_Side, ev.m_Price, ev.m_Shares);
        }
        else if (ev.m_MessageType == 'E')
        {
            Order* activeOrder = orderPool.GetOrder(ev.m_OrderRefNum);
            if (activeOrder)
            {
                marketBooks[activeOrder->m_StockLocate].RemoveVolume(activeOrder->m_Side, activeOrder->m_Price,
                                                                     ev.m_Shares);
                orderPool.ReduceShares(ev.m_OrderRefNum, ev.m_Shares);
            }
        }
        else if (ev.m_MessageType == 'X')
        {
            Order* activeOrder = orderPool.GetOrder(ev.m_OrderRefNum);
            if (activeOrder)
            {
                marketBooks[activeOrder->m_StockLocate].RemoveVolume(activeOrder->m_Side, activeOrder->m_Price,
                                                                     ev.m_Shares);
                orderPool.ReduceShares(ev.m_OrderRefNum, ev.m_Shares);
            }
        }
        else if (ev.m_MessageType == 'D')
        {
            Order* activeOrder = orderPool.GetOrder(ev.m_OrderRefNum);
            if (activeOrder)
            {
                marketBooks[activeOrder->m_StockLocate].RemoveVolume(activeOrder->m_Side, activeOrder->m_Price,
                                                                     activeOrder->m_Shares);
                orderPool.DeleteOrder(ev.m_OrderRefNum);
            }
        }
        else if (ev.m_MessageType == 'H')
        {
            std::cout << "\n--- Processed " << ev.m_Shares
                      << " Million Network Messages --- | Active Orders in RAM: " << orderPool.GetActiveOrderCount()
                      << std::endl;

            uint16_t loc = g_TargetLocate.load(std::memory_order_relaxed);
            if (loc != 0)
            {
                marketBooks[loc].PrintTopOfBook("AAPL");
                double imbalance = marketBooks[loc].CalculateImbalance();
                std::cout << "Order Book Imbalance: " << imbalance << std::endl;
            }
        }
    }
}

int main()
{
    std::string fileName = "../data/01302020.NASDAQ_ITCH50";
    SPSCRingBuffer ringBuffer(1048576);

    std::cout << "--- Engine Starting: Multithreaded Mode ---" << std::endl;
    std::thread producerThread(NetworkProducer, std::ref(ringBuffer), fileName);
    std::thread consumerThread(LogicConsumer, std::ref(ringBuffer));

    producerThread.join();
    consumerThread.join();

    std::cout << "\n--- Engine Successfully Shut Down ---" << std::endl;
    return 0;
}