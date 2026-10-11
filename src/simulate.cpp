#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <unordered_set>
#include <vector>
#include "orderbook.h"

// Where traders anchor their prices: the mid price if both sides of the book
// exist, otherwise the last trade price.
static int64_t referencePrice(const orderbook& book, int64_t lastPrice) {
    if (auto mid = book.midPrice()) return static_cast<int64_t>(*mid);
    return lastPrice;
}

// ---------------------------------------------------------------------
// Random trader: noise. Each step it cancels an old order, sends a market
// order, or places a limit order within 20 cents of the reference price.
// ---------------------------------------------------------------------
class RandomTrader {
public:
    void step(orderbook& book, uint64_t& nextId, int64_t ref, std::mt19937& rng) {
        std::uniform_real_distribution<double> coin(0.0, 1.0);
        double action = coin(rng);

        // 20%: cancel a random order we placed earlier
        if (action < 0.20 && !live_.empty()) {
            size_t i = std::uniform_int_distribution<size_t>(0, live_.size() - 1)(rng);
            book.cancelOrder(live_[i]);   // returns false if it already traded, that's fine
            live_[i] = live_.back();
            live_.pop_back();
            return;
        }

        Side side = (coin(rng) < 0.5) ? Side::Buy : Side::Sell;
        uint32_t qty = std::uniform_int_distribution<uint32_t>(1, 20)(rng);
        uint64_t id = nextId++;

        if (action < 0.35) {
            // about 15%: market order
            book.addOrder({id, side, 0, qty, OrderType::Market});
        } else {
            // the rest: limit order near the reference price
            int64_t price = ref + std::uniform_int_distribution<int>(-20, 20)(rng);
            if (price < 1) price = 1;
            book.addOrder({id, side, price, qty});
            live_.push_back(id);
        }
    }

private:
    std::vector<uint64_t> live_;   // ids of limit orders that may still be resting
};

// ---------------------------------------------------------------------
// Market maker: keeps a bid and an ask around the reference price and earns
// the spread when both sides trade. If it piles up inventory, it shifts both
// quotes to encourage trades that bring the inventory back toward zero.
// ---------------------------------------------------------------------
class MarketMaker {
public:
    int64_t cash = 0;        // cents
    int64_t inventory = 0;   // shares (negative means short)

    void step(orderbook& book, uint64_t& nextId, int64_t ref) {
        // Pull the old quotes, then post fresh ones
        for (uint64_t id : quotes_) book.cancelOrder(id);
        quotes_.clear();

        // Long inventory -> quote lower (want to sell). Short -> quote higher.
        int64_t skew = std::clamp<int64_t>(-inventory / 10, -15, 15);
        int64_t bid = ref - kHalfSpread + skew;
        int64_t ask = ref + kHalfSpread + skew;
        if (bid < 1) return;

        uint64_t bidId = nextId++;
        uint64_t askId = nextId++;
        mine_.insert(bidId);
        mine_.insert(askId);

        book.addOrder({bidId, Side::Buy, bid, kQuoteSize});
        quotes_.push_back(bidId);
        book.addOrder({askId, Side::Sell, ask, kQuoteSize});
        quotes_.push_back(askId);
    }

    // Called for every trade in the market. Updates cash and inventory if we were in it.
    void onTrade(const Trade& t) {
        int64_t value = t.price * static_cast<int64_t>(t.quantity);
        if (mine_.count(t.buyId)) {
            cash -= value;
            inventory += t.quantity;
        }
        if (mine_.count(t.sellId)) {
            cash += value;
            inventory -= t.quantity;
        }
    }

private:
    static constexpr int64_t kHalfSpread = 10;   // quote 10 cents each side of the reference
    static constexpr uint32_t kQuoteSize = 10;
    std::vector<uint64_t> quotes_;               // our currently live quotes
    std::unordered_set<uint64_t> mine_;          // every order id we have ever used
};

// ---------------------------------------------------------------------
// The simulation
// ---------------------------------------------------------------------
int main(int argc, char** argv) {
    int steps = 20000;
    if (argc > 1) steps = std::atoi(argv[1]);

    orderbook book;
    RandomTrader trader;
    MarketMaker mm;
    std::mt19937 rng(42);          // fixed seed: same run every time
    uint64_t nextId = 1;
    int64_t lastPrice = 10000;     // start at $100.00

    std::ofstream tradesCsv("trades.csv");
    std::ofstream stepsCsv("steps.csv");
    tradesCsv << "step,buy_id,sell_id,price,quantity\n";
    stepsCsv << "step,best_bid,best_ask,spread,mid,mm_inventory,mm_cash,mm_pnl\n";
    stepsCsv << std::fixed << std::setprecision(1);

    size_t processed = 0;
    int64_t minPrice = std::numeric_limits<int64_t>::max();
    int64_t maxPrice = std::numeric_limits<int64_t>::min();

    // Handle every trade that happened since we last looked
    auto handleNewTrades = [&](int step) {
        const std::vector<Trade>& all = book.trades();
        for (; processed < all.size(); ++processed) {
            const Trade& t = all[processed];
            mm.onTrade(t);
            lastPrice = t.price;
            minPrice = std::min(minPrice, t.price);
            maxPrice = std::max(maxPrice, t.price);
            tradesCsv << step << ',' << t.buyId << ',' << t.sellId << ','
                      << t.price << ',' << t.quantity << '\n';
        }
    };

    for (int step = 1; step <= steps; ++step) {
        trader.step(book, nextId, referencePrice(book, lastPrice), rng);
        handleNewTrades(step);

        mm.step(book, nextId, referencePrice(book, lastPrice));
        handleNewTrades(step);

        // Record the state of the market after this step
        auto bid = book.bestBid();
        auto ask = book.bestAsk();
        auto mid = book.midPrice();
        double mark = mid ? *mid : static_cast<double>(lastPrice);
        double pnl = static_cast<double>(mm.cash) + static_cast<double>(mm.inventory) * mark;

        stepsCsv << step << ',';
        if (bid) stepsCsv << *bid;
        stepsCsv << ',';
        if (ask) stepsCsv << *ask;
        stepsCsv << ',';
        if (bid && ask) stepsCsv << (*ask - *bid);
        stepsCsv << ',';
        if (mid) stepsCsv << *mid;
        stepsCsv << ',' << mm.inventory << ',' << mm.cash << ',' << pnl << '\n';
    }

    double finalPnl = static_cast<double>(mm.cash) + static_cast<double>(mm.inventory) * static_cast<double>(lastPrice);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Steps run:        " << steps << "\n";
    std::cout << "Trades:           " << book.trades().size() << "\n";
    if (!book.trades().empty()) {
        std::cout << "Price range:      $" << minPrice / 100.0 << " to $" << maxPrice / 100.0 << "\n";
    }
    std::cout << "Last price:       $" << lastPrice / 100.0 << "\n";
    std::cout << "Market maker:     inventory " << mm.inventory << " shares, cash $"
              << mm.cash / 100.0 << ", P&L $" << finalPnl / 100.0 << "\n";
    std::cout << "Wrote trades.csv and steps.csv\n";
    return 0;
}