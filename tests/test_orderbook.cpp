#include <iostream>
#include "../src/orderbook.h"

static int checks = 0;
static int failures = 0;

// If the condition is false, print which line failed
#define CHECK(cond)                                                  \
    do {                                                             \
        ++checks;                                                    \
        if (!(cond)) {                                               \
            ++failures;                                              \
            std::cout << "  FAIL (line " << __LINE__ << "): " #cond "\n"; \
        }                                                            \
    } while (0)

void test_no_cross_rests_in_book() {
    orderbook book;
    book.addOrder({1, Side::Sell, 10100, 10});
    book.addOrder({2, Side::Buy, 9900, 10});
    CHECK(book.trades().size() == 0);
    CHECK(*book.bestBid() == 9900);
    CHECK(*book.bestAsk() == 10100);
    CHECK(*book.spread() == 200);
}

void test_full_match_empties_book() {
    orderbook book;
    book.addOrder({1, Side::Sell, 10020, 15});
    book.addOrder({2, Side::Buy, 10020, 15});
    CHECK(book.trades().size() == 1);
    CHECK(book.trades()[0].quantity == 15);
    CHECK(book.trades()[0].price == 10020);
    CHECK(!book.bestAsk().has_value());
    CHECK(!book.bestBid().has_value());
}

void test_partial_fill_leaves_remainder() {
    orderbook book;
    book.addOrder({1, Side::Sell, 10020, 30});
    book.addOrder({2, Side::Buy, 10020, 10});
    CHECK(book.trades().size() == 1);
    CHECK(book.trades()[0].quantity == 10);
    CHECK(book.quantityAt(Side::Sell, 10020) == 20);
}

void test_trade_happens_at_resting_price() {
    orderbook book;
    book.addOrder({1, Side::Sell, 10020, 10});
    book.addOrder({2, Side::Buy, 10050, 10});   // willing to pay more
    CHECK(book.trades().size() == 1);
    CHECK(book.trades()[0].price == 10020);     // but pays the resting price
}

void test_price_time_priority() {
    orderbook book;
    book.addOrder({1, Side::Sell, 10020, 10});  // arrived first
    book.addOrder({2, Side::Sell, 10020, 10});  // arrived second
    book.addOrder({3, Side::Buy, 10020, 10});
    CHECK(book.trades().size() == 1);
    CHECK(book.trades()[0].sellId == 1);        // oldest order fills first
    book.addOrder({4, Side::Buy, 10020, 10});
    CHECK(book.trades().size() == 2);
    CHECK(book.trades()[1].sellId == 2);
}

void test_buy_walks_multiple_levels() {
    orderbook book;
    book.addOrder({1, Side::Sell, 10020, 10});
    book.addOrder({2, Side::Sell, 10050, 10});
    book.addOrder({3, Side::Buy, 10050, 15});
    CHECK(book.trades().size() == 2);
    CHECK(book.trades()[0].price == 10020);
    CHECK(book.trades()[0].quantity == 10);
    CHECK(book.trades()[1].price == 10050);
    CHECK(book.trades()[1].quantity == 5);
    CHECK(book.quantityAt(Side::Sell, 10050) == 5);
}

void test_sell_matches_best_bid_first() {
    orderbook book;
    book.addOrder({1, Side::Buy, 9900, 10});
    book.addOrder({2, Side::Buy, 9980, 10});    // better price
    book.addOrder({3, Side::Sell, 9900, 10});
    CHECK(book.trades().size() == 1);
    CHECK(book.trades()[0].buyId == 2);
    CHECK(book.trades()[0].price == 9980);
}

void test_cancel() {
    orderbook book;
    book.addOrder({1, Side::Buy, 9900, 10});
    CHECK(book.cancelOrder(1) == true);
    CHECK(!book.bestBid().has_value());
    CHECK(book.cancelOrder(1) == false);        // already gone
    CHECK(book.cancelOrder(999) == false);      // never existed
}

void test_cannot_cancel_filled_order() {
    orderbook book;
    book.addOrder({1, Side::Sell, 10020, 10});
    book.addOrder({2, Side::Buy, 10020, 10});
    CHECK(book.cancelOrder(1) == false);
}

void test_market_order_leftover_is_discarded() {
    orderbook book;
    book.addOrder({1, Side::Sell, 10020, 5});
    book.addOrder({2, Side::Buy, 0, 20, OrderType::Market});
    CHECK(book.trades().size() == 1);
    CHECK(book.trades()[0].quantity == 5);
    CHECK(!book.bestBid().has_value());         // leftover 15 must not rest
}

void test_spread_and_mid() {
    orderbook book;
    book.addOrder({1, Side::Buy, 9980, 10});
    book.addOrder({2, Side::Sell, 10020, 10});
    CHECK(*book.spread() == 40);
    CHECK(*book.midPrice() == 10000.0);
}

void test_empty_book_queries() {
    orderbook book;
    CHECK(!book.bestBid().has_value());
    CHECK(!book.bestAsk().has_value());
    CHECK(!book.spread().has_value());
    CHECK(!book.midPrice().has_value());
}

#define RUN(test)                                   \
    do {                                            \
        int before = failures;                      \
        test();                                     \
        std::cout << (failures == before ? "PASS  " : "FAIL  ") << #test << "\n"; \
    } while (0)

int main() {
    RUN(test_no_cross_rests_in_book);
    RUN(test_full_match_empties_book);
    RUN(test_partial_fill_leaves_remainder);
    RUN(test_trade_happens_at_resting_price);
    RUN(test_price_time_priority);
    RUN(test_buy_walks_multiple_levels);
    RUN(test_sell_matches_best_bid_first);
    RUN(test_cancel);
    RUN(test_cannot_cancel_filled_order);
    RUN(test_market_order_leftover_is_discarded);
    RUN(test_spread_and_mid);
    RUN(test_empty_book_queries);

    std::cout << "\n" << (checks - failures) << "/" << checks << " checks passed\n";
    return failures == 0 ? 0 : 1;
}