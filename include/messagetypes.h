#include <cstdint>

// --- ITCH 5.0 Structs ---
#pragma pack(push, 1)
struct SystemEventMessage
{
    char m_MessageType;
    uint16_t m_StockLocate;
    uint16_t m_TrackingNumber;
    uint8_t m_Timestamp[6];
    char m_EventCode;
};

struct StockDirectoryMessage
{
    char m_MessageType;
    uint16_t m_StockLocate;
    uint16_t m_TrackingNumber;
    uint8_t m_Timestamp[6];
    char m_Stock[8];
    char m_MarketCategory;
    char m_FinancialStatusIndicator;
    uint32_t m_RoundLotSize;
    char m_RoundLotsOnly;
    char m_IssueClassification;
    char m_IssueSubType[2];
    char m_Authenticity;
    char m_ShortSaleThresholdIndicator;
    char m_IPOFlag;
    char m_LULDReferencePriceTier;
    char m_ETPFlag;
    uint32_t m_ETPLeverageFactor;
    char m_InverseIndicator;
};

struct AddOrderMessage
{
    char m_MessageType;
    uint16_t m_StockLocate;
    uint16_t m_TrackingNumber;
    uint8_t m_Timestamp[6];
    uint64_t m_OrderReferenceNumber;
    char m_BuySellIndicator;
    uint32_t m_Shares;
    char m_Stock[8];
    uint32_t m_Price;
};

struct OrderExecutedMessage
{
    char m_MessageType;
    uint16_t m_StockLocate;
    uint16_t m_TrackingNumber;
    uint8_t m_Timestamp[6];
    uint64_t m_OrderReferenceNumber;
    uint32_t m_ExecutedShares;
    uint64_t m_MatchNumber;
};

struct OrderCanceledMessage
{
    char m_MessageType;
    uint16_t m_StockLocate;
    uint16_t m_TrackingNumber;
    uint8_t m_Timestamp[6];
    uint64_t m_OrderReferenceNumber;
    uint32_t m_CanceledShares;
};

struct OrderDeletedMessage
{
    char m_MessageType;
    uint16_t m_StockLocate;
    uint16_t m_TrackingNumber;
    uint8_t m_Timestamp[6];
    uint64_t m_OrderReferenceNumber;
};
#pragma pack(pop)