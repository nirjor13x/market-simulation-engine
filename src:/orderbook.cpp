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

void orderbook::addOrder(const Order& order){
    if (order.side == Side::Buy){
        bids_[order.price].push_back(order);
    }else{
        asks_[order.price].push_back(order);
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