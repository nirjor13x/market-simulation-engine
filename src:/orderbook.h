#pragma once
#include <deque>
#include <functional>
#include <map>
#include "order.h"
#include <optional>
#include <unordered_map>
#include <vector>

class orderbook {
    public:
        void addOrder(Order order);
        bool cancelOrder(uint64_t id);

        std::optional<int64_t> bestBid() const;
        std::optional<int64_t> bestAsk() const;
        std::optional<int64_t> spread() const;
        std::optional<double> midPrice() const;

        void print() const;
        void printTrades() const;
        void printSummary() const;
    private:

        struct Location {
            Side side;
            int64_t price;
        };
        // price -> queue of orders at that price (oldest at the front)
        // bids: highest price first. asks: lowest price first
        std::map<int64_t, std::deque<Order>, std::greater<int64_t>> bids_;
        std::map<int64_t, std::deque<Order>> asks_;
        std::unordered_map<uint64_t, Location> index_;
        std::vector<Trade> trades_; // every trade that has happened
    
};