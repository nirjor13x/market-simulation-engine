#include<iostream>
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

    std::cout << "cancel #2: " << (book.cancelOrder(2) ? "ok" : "not found") << "\n";
    book.addOrder({9, Side::Buy, 0, 20, OrderType::Market});
    book.addOrder({10, Side::Sell, 0, 80, OrderType::Market});
    std::cout << "cancel #999: " << (book.cancelOrder(999) ? "ok" : "not found") << "\n";

    std::cout << "\n=== Stage 4 test ===\n";
    orderbook q;
    q.addOrder({101, Side::Buy, 9980, 10});
    q.addOrder({102, Side::Sell, 10020, 10});
    q.printSummary();

    std::cout << "\n--- empty book ---\n";
    orderbook empty;
    empty.printSummary();



    book.printTrades();
    book.print();
    return 0;
}