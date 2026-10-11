#include "orderbook.h"

int main(){
    orderbook book;

    book.addOrder({1, Side::Sell, 10100, 30});
    book.addOrder({2, Side::Sell, 10050, 20});
    book.addOrder({3, Side::Sell, 10020, 15});
    book.addOrder({4, Side::Sell, 10020, 30});

    book.addOrder({4, Side::Buy, 9980, 25});
    book.addOrder({5, Side::Buy, 9950, 10});
    book.addOrder({6, Side::Buy, 9900, 50});
    book.addOrder({7, Side::Buy, 9900, 20});

    // Test 1: a buyer who crosses the book
    book.addOrder({8, Side::Buy, 10050, 20});

    // Test 2: a big seller who eats the top bids and has shares left over
    book.addOrder({8, Side::Sell, 9950, 40});


    book.printTrades();
    book.print();
    return 0;
}