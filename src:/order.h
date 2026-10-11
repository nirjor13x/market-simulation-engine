#pragma once
#include <cstdint>

enum class Side { Buy, Sell };

struct Order {
    uint64_t id; // unique order id
    Side side; // Buy or Sell
    int64_t price; // in cents: 10020 means $100.20
    uint32_t quantity; // number of shares
};

// A record of one completed trade
struct Trade {
    uint64_t buyId;
    uint64_t sellId;
    int64_t price; // always the price of the order that was already resting
    uint32_t quantitiy;
};