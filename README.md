# Market Simulation Engine

A limit order book and matching engine written in C++, paired with a
simulator that populates it with automated traders and a Python toolkit
for analyzing the results.

## Overview

This project models how a financial exchange works under the hood. Orders
are matched using **price-time priority**: the best price executes first,
and orders at the same price are filled in the order they arrived. The
engine supports limit orders, market orders, partial fills, and
cancellations, and records every trade for later analysis.

On top of the engine, simulated traders (random participants and a simple
market maker) generate order flow, so you can study how prices, spreads,
and liquidity emerge from many small decisions.

## Features

- Limit order book with bid/ask price levels
- Price-time priority matching with partial fills
- Market orders and order cancellation
- Trade log with best bid, best ask, spread, and mid price
- Simulated traders: random traders and a market maker
- Python analysis: price history, spread distribution, volume, and
  market maker profit and inventory

## Tech stack

- **C++17**: matching engine and simulator
- **Python**: data analysis and plotting (pandas, matplotlib)

## Project structure

    src/        C++ engine and simulator
    tests/      Hand-checked matching scenarios
    analysis/   Python scripts and plots

## Build and run

    g++ -std=c++17 -Wall -Wextra src/main.cpp src/orderbook.cpp -o build/engine
    ./build/engine

    g++ -std=c++17 -Wall -Wextra tests/test_orderbook.cpp src/orderbook.cpp -o build/tests
    ./build/tests

## Roadmap

- [x] Order book and limit order matching
- [x] Market orders and cancellations
- [x] Trade log and book queries
- [x] Test suite
- [ ] Simulated traders
- [ ] Python analysis and plots

## What I'm learning

Data structures for fast order lookup, deterministic testing of matching
logic, and how market microstructure shapes price behavior.
