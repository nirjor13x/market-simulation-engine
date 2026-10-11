#include "orderbook.h"
#include <iostream>
#include <string>
using namespace std;

static string formatPrice(int64_t cents){
    string c = to_string(cents%100);
    if (c.size()<2) c = "0" + c;
    return "$" + to_string(cents/100) + "." + c;
}

static uint32_t totalQuantity(const deque<Order>& level){
    uint32_t total = 0;
    for (const Order& o : level) total += o.quantity;
    return total;
}

void orderbook::addOrder(Order order){
    if (order.side == Side::Buy){
        // Keep trading while we still want shares and a cheap-enough seller exists
        while (order.quantity > 0 && !asks_.empty()){
            auto best = asks_.begin();
            if (best->first > order.price) break;
            std::deque<Order>& level = best->second;
            
            while (order.quantity > 0 && !level.empty()){
                Order& resting = level.front();
                uint32_t qty = std::min(order.quantity, resting.quantity);

                trades_.push_back({order.id, resting.id, resting.price, qty});
                order.quantity -= qty;
                resting.quantity -= qty;
                
                if (resting.quantity == 0) level.pop_front(); // fully filled, remove
            }
            if (level.empty()) asks_.erase(best); // no orders left at this price
        }
        // Anything not filled waits in the book
        if (order.quantity > 0) bids_[order.price].push_back(order);
    }else{
        // Same logic, mirrored: a seller matches against the highest bids
        while (order.quantity > 0 && !bids_.empty()){
            auto best = bids_.begin();
            if (best->first < order.price) break;
            std::deque<Order>& level = best->second;

            while (order.quantity > 0 && !level.empty()){
                Order& resting = level.front();
                uint32_t qty = std::min(order.quantity, resting.quantity);

                trades_.push_back({resting.id, order.id, resting.price, qty});
                order.quantity -= qty;
                resting.quantity -= qty;

                if (resting.quantity == 0) level.pop_front();
            }
            if (level.empty()) bids_.erase(best);
        }
        if (order.quantity > 0) asks_[order.price].push_back(order);
    }
}

void orderbook::print() const {
    cout << "----- ASKS (sells) -----\n";
    // asks_ is lowest-first, so walk it backwards to show the best ask nearest the middle
    for (auto it = asks_.rbegin(); it != asks_.rend(); ++it){
        cout << formatPrice(it->first) << "  x " << totalQuantity(it->second) << "\n";
    }
    cout << "----- BIDS (buys) -----\n";
    for(auto it = bids_.begin(); it != bids_.end(); ++it){
        cout << formatPrice(it->first) << "  x " << totalQuantity(it->second) << "\n";
    }
}

void orderbook::printTrades() const {
    std::cout << "----- TRADES -----\n";
    for(const Trade& t : trades_){
        std::cout << "buy #" << t.buyId << " x sell #" << t.sellId
        << " | " << t.quantitiy << " @ " << formatPrice(t.price) << "\n";
    }
}