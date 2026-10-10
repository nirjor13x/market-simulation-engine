#pragma once
#include <cstdint>

enum class Side { Buy, Sell };

struct Order {
    uint64_t id; // unique order id
    Side side; // Buy or Sell
    int64_t price; // in cents: 10020 means $100.20
    uint32_t quantity; // number of shares
};