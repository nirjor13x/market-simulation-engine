#pragma once
#include <deque>
#include <functional>
#include <map>
#include "order.h"

class orderbook {
    public:
        void addOrder(Order order);
        void print() const;
        void printTrades() const;
    private:
        // price -> queue of orders at that price (oldest at the front)
        // bids: highest price first. asks: lowest price first
        std::map<int64_t, std::deque<Order>, std::greater<int64_t>> bids_;
        std::map<int64_t, std::deque<Order>> asks_;
        std::vector<Trade> trades_; // every trade that has happened
    
};