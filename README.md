# C++ Stock Market Order Simulator

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/std/the-standard) [![Build Status](https://img.shields.io/badge/Build-Passing-brightgreen.svg)](#)

Simulate a simplified single-stock order book: read orders from a file, match buys and sells under real-world–inspired rules, execute trades, and log everything.

---

## Table of Contents

- [What Is This?](#what-is-this)  
- [Key Features](#key-features)  
- [How It Works](#how-it-works)  
- [Architecture & Algorithms](#architecture--algorithms)  
- [Tech Stack](#tech-stack)  
- [Getting Started](#getting-started)  
  - [Prerequisites](#prerequisites)  
  - [Building](#building)  
  - [Running](#running)  
- [Example](#example)  
- [License](#license)  

---

## What Is This?

A **Stock Market Order Simulator** in C++ that:

- Reads a batch of buy/sell orders from a text file (one stock only).  
- Maintains an in-memory order book with price/time/match priority.  
- Executes trades according to limit/market rules and partial fills.  
- Prints the book state to the console before/after each match.  
- Writes every execution (and final unfilled residuals) to an output file.

This project showcases data structures, algorithms, and modern C++ coding patterns in a self-contained, interactive console application.

---

## Key Features

- **Limit & Market Orders**:  
  - Limit orders specify a maximum (buy) or minimum (sell) price.  
  - Market orders execute immediately at the best available price.

- **Priority Matching**:  
  1. **Market orders** outrank any limit order.  
  2. **Price priority**: highest‐buy vs. lowest‐sell.  
  3. **Time priority**: older orders match first on ties.

- **Partial Fills & Residuals**:  
  - Orders divisible into smaller quantities; residual portions re-enter matching until exhausted.

- **Execution Price Logic**:  
  - **Two limits** → older order’s limit price.  
  - **Limit + market** → limit order’s price.  
  - **Two markets** → last traded price.

- **Console & File Outputs**:  
  - Interactive console view of “before”/“after” book states.  
  - Canonical log file (`output#.txt`) with exact phrasing for automated grading.

---

## How It Works

1. **Initialization**  
   - Read the first line of `input#.txt` as the “last traded price” from the previous trading session.

2. **Order Parsing**  
   - Each subsequent line:  
     ```text
     <orderID> <B|S> <quantity> [<limitPrice>]
     ```  
   - No `<limitPrice>` → market order.

3. **OrderBook Class**  
   - Two `std::priority_queue<Order>` (buy & sell), ordered by custom comparator.  
   - `addOrder()` pushes new orders into the correct queue.  

4. **Matching Loop**  
   - After each insert, continuously attempt to match the top buy vs. top sell:  
     - Check `canMatch()` (price or market rule).  
     - Pop, compute `tradedQuantity` = `min(buy.qty, sell.qty)`.  
     - Determine `executionPrice()` by type/timestamp.  
     - Record trade in `output#.txt`.  
     - If partial fill, push residual back into its queue—then re-match immediately.

5. **State Display**  
   - `displayPendingOrders()` prints the current book (sorted by priority) and last traded price before & after matching.

6. **Finalization**  
   - Once all orders processed, dump any unexecuted residuals to the output file.

---

## Architecture & Algorithms

```cpp
struct Order {
  std::string id;
  char        type;           // 'B' or 'S'
  int         quantity;
  double      limitPrice;     
  bool        isMarketOrder;  
  int         timestamp;      // arrival order
  bool operator<(Order const& o) const;
};

class OrderBook {
private:
  std::priority_queue<Order> buyOrders;
  std::priority_queue<Order> sellOrders;
  double                     lastTradedPrice;

  bool   canMatch(Order const& b, Order const& s) const;
  double determinePrice(Order const& b, Order const& s) const;
  void   displayOrders(...) const;
public:
  OrderBook(double initialPrice);
  void addOrder(Order const&);
  void matchOrders(std::ofstream&);
  void displayPendingOrders() const;
  void writeUnexecutedOrders(std::ofstream&) const;
};
```

- **Priority Queue Comparator**  
  - Buys: higher `limitPrice` → higher priority.  
  - Sells: lower `limitPrice` → higher priority.  
  - Market orders rank above any limit orders.  
  - Ties broken by earlier `timestamp`.

- **Complexity**  
  - Each insert/match is **O(log n)**.  
  - Worst-case partial fills may incur multiple pushes/pops per order, but overall stays efficient for typical problem sizes.

---

## Tech Stack

- **Language**:  
  - C++17 (STL containers & algorithms)  
  - No external libraries—zero dependencies.

- **Build Tool**:  
  - GNU Make (`makefile`)  
  - `-std=c++17` `-O2` `-Wall` `-Wextra`

- **Data Structures**:  
  - `std::priority_queue` for order sorting.  
  - `std::vector` for temporary order lists.  
  - `std::ostringstream` + `<iomanip>` for price formatting.

- **I/O**:  
  - File streams (`<fstream>`) for batch input/output.  
  - Console I/O (`<iostream>`) for interactive state dumps.

---

## Getting Started

### Prerequisites

- A C++17-compatible compiler (e.g. `g++` ≥ 7.0)  
- GNU Make  

### Building

```bash
git clone https://github.com/yourusername/stock-order-simulator.git
cd stock-order-simulator
make
```

This produces the executable `main`.

### Running

```bash
./main input1.txt
```

- Reads `input1.txt`, writes `output1.txt`.  
- Console shows “Before Matching” and “After Matching” book states at each step.

---

## Example

```text
$ cat input1.txt
10.00
ord001 B 100 9.75
ord002 S 50
...

$ ./main input1.txt
Before Matching:
Last trading price: 10.00
Buy              Sell
-------------------------
ord001  9.75 100  ord002 M 50

After Matching:
Last trading price: 9.75
Buy              Sell
-------------------------
[...]
```

Contents of `output1.txt`:

```text
order ord001 50 shares purchased at price 9.75
order ord002 50 shares sold at price 9.75
order ord001 50 shares unexecuted
```

---

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE.md) for details.

