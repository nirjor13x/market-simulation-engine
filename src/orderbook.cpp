#include "orderbook.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <string>
#include <iomanip>
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
    const bool isMarket = (order.type == OrderType::Market);

    if (isMarket){
        order.price = (order.side == Side::Buy)
                        ? std::numeric_limits<int64_t>::max()
                        : std::numeric_limits<int64_t>::min();
    }
    if (order.side == Side::Buy){
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
                
                if (resting.quantity == 0) {
                    index_.erase(resting.id);
                    level.pop_front();
                }
            }
            if (level.empty()) asks_.erase(best); // no orders left at this price
        }
        if (!isMarket && order.quantity > 0){
            bids_[order.price].push_back(order);
            index_[order.id] = {Side::Buy, order.price};
        }
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

                if (resting.quantity == 0) {
                    index_.erase(resting.id);
                    level.pop_front();
                }
            }
            if (level.empty()) bids_.erase(best);
        }
        if (!isMarket && order.quantity > 0){
            asks_[order.price].push_back(order);
            index_[order.id] = {Side::Sell, order.price};
        }
    }
}

bool orderbook::cancelOrder(uint64_t id){
    auto found = index_.find(id);
    if (found == index_.end()) return false;

    Location loc = found -> second;

    auto removeFrom = [&](auto& book){
        auto levelIt = book.find(loc.price);
        if(levelIt == book.end()) return;
        auto& level = levelIt -> second;
        for(auto it = level.begin(); it != level.end(); ++it){
            if(it->id == id){
                level.erase(it);
                break;
            }
        }
        if(level.empty()) book.erase(levelIt);
    };

    if(loc.side == Side::Buy) removeFrom(bids_);
    else removeFrom(asks_);

    index_.erase(found);
    return true;
}
std::optional<int64_t> orderbook::bestBid() const{
    if(bids_.empty()) return std::nullopt;
    return bids_.begin() -> first;
}

std::optional<int64_t> orderbook::bestAsk() const{
    if(asks_.empty()) return std::nullopt;
    return asks_.begin() -> first;
}

std::optional<int64_t> orderbook::spread() const{
    auto bid = bestBid();
    auto ask = bestAsk();
    if (!bid || !ask) return std::nullopt;
    return *ask - *bid;
}

std::optional<double> orderbook::midPrice() const{
    auto bid = bestBid();
    auto ask = bestAsk();
    if (!bid || !ask) return std::nullopt;
    return (*bid + *ask) / 2.0;
}

void orderbook::printSummary() const {
    auto bid = bestBid();
    auto ask = bestAsk();
    auto sp = spread();
    auto mid = midPrice();

    std::cout << "Best bid: " << (bid ? formatPrice(*bid) : "none") << "\n";
    std::cout << "Best ask: " << (ask ? formatPrice(*ask) : "none") << "\n";
    std::cout << "Spread:   " << (sp ? formatPrice(*sp) : "none") << "\n";
    if (mid){
        std::cout << "Mid:     $" << std::fixed << std::setprecision(3)
                  << (*mid/100.0) << "\n";
    }else{
        std::cout << "Mid:     none\n";
    }
}

uint32_t orderbook::quantityAt(Side side, int64_t price) const {
    if (side==Side::Buy){
        auto it = bids_.find(price);
        return it == bids_.end() ? 0 : totalQuantity(it->second);
    }
    auto it = asks_.find(price);
    return it == asks_.end() ? 0 : totalQuantity(it->second);
}

void orderbook::print() const {
    cout << "----- ASKS (sells) -----\n";
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
        << " | " << t.quantity << " @ " << formatPrice(t.price) << "\n";
    }
}