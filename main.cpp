#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <queue>
#include <vector>
#include <map>
#include <algorithm>

// struct to represent an order in the order book (for all orders)
struct Order {
    std::string id;
    char type; // Using similar notiation as examples given (on Blackboard) --- 'B' for buy, 'S' for sell
    int quantity;
    double limitPrice;
    bool isMarketOrder;
    int timestamp;

    // Trying to use a sorting logic for a comparision operator for the priority queue of orders...
    bool operator<(const Order& other) const {
        if (type == 'B') { // Buy orders: Higher price first
            if (limitPrice != other.limitPrice) return limitPrice < other.limitPrice;
        } else { // Sell orders: Lower price first
            if (limitPrice != other.limitPrice) return limitPrice > other.limitPrice;
        }
        return timestamp > other.timestamp; // Older orders first
    }
};

// Helper function to format prices with 2 decimal places
std::string formatPrice(double price) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << price;
    return oss.str();
}

// Class to manage the order book and process trades
class OrderBook {
    std::priority_queue<Order> buyOrders; // Priority queue for buy orders
    std::priority_queue<Order> sellOrders; // Priority queue for sell orders
    double lastTradedPrice; // Stores the last traded price

public:
    // Initializing the order book with the initial price (and the logic)
    OrderBook(double initialPrice) : lastTradedPrice(initialPrice) {}

    // Adds a new order to the appropriate queue
    void addOrder(const Order& order) {
        if (order.type == 'B') {
            buyOrders.push(order);
        } else {
            sellOrders.push(order);
        }
    }

    // Matches and executes orders from the buy and sell queues
    void matchOrders(std::ofstream& output) {
        while (!buyOrders.empty() && !sellOrders.empty()) {
            Order buy = buyOrders.top();
            Order sell = sellOrders.top();

            if (!canMatch(buy, sell)) break;

            buyOrders.pop();
            sellOrders.pop();

            int tradedQuantity = std::min(buy.quantity, sell.quantity);
            double executionPrice = determinePrice(buy, sell);

            lastTradedPrice = executionPrice;

            // Log executed orders to the output file
            output << "order " << buy.id << " " << tradedQuantity << " shares purchased at price "
                   << std::fixed << std::setprecision(2) << executionPrice << "\n";
            output << "order " << sell.id << " " << tradedQuantity << " shares sold at price "
                   << std::fixed << std::setprecision(2) << executionPrice << "\n";

            if (buy.quantity > tradedQuantity) {
                buy.quantity -= tradedQuantity;
                buyOrders.push(buy);
            }

            if (sell.quantity > tradedQuantity) {
                sell.quantity -= tradedQuantity;
                sellOrders.push(sell);
            }
        }
    }

    void displayPendingOrders() const {
        std::cout << "Last trading price: " << std::fixed << std::setprecision(2) << lastTradedPrice << "\n";
        std::cout << "Buy                                    Sell\n";
        std::cout << "-------------------------------------------------\n";
        displayOrders(buyOrders, sellOrders);
        std::cout << "=================================================\n";
    }

    // This writess the unexecuted orders to the output file...
    void writeUnexecutedOrders(std::ofstream& output) const {
        // Combine buy and sell orders into a single vector
        std::vector<Order> unexecutedOrders;
        // Write unexecuted buy orders
        auto remainingBuyOrders = buyOrders;
        while (!remainingBuyOrders.empty()) {
            unexecutedOrders.push_back(remainingBuyOrders.top());
            remainingBuyOrders.pop();
        }
        // Write unexecuted sell orders
        auto remainingSellOrders = sellOrders;
        while (!remainingSellOrders.empty()) {
            unexecutedOrders.push_back(remainingSellOrders.top());
            remainingSellOrders.pop();
        }

        std::sort(unexecutedOrders.begin(), unexecutedOrders.end(),
                  [](const Order& a, const Order& b) { return a.timestamp < b.timestamp; });

        for (const auto& order : unexecutedOrders) {
            output << "order " << order.id << " " << order.quantity << " shares unexecuted\n";
        }
    }

private:
    // Determines if a buy and sell order can be matched
    bool canMatch(const Order& buy, const Order& sell) const {
        return (buy.isMarketOrder || sell.isMarketOrder || buy.limitPrice >= sell.limitPrice);
    }

    // Calculates the execution price for a matched pair of orders
    double determinePrice(const Order& buy, const Order& sell) const {
        if (!buy.isMarketOrder && !sell.isMarketOrder) {
            return buy.timestamp < sell.timestamp ? buy.limitPrice : sell.limitPrice;
        }
        if (!buy.isMarketOrder) return buy.limitPrice;
        if (!sell.isMarketOrder) return sell.limitPrice;
        return lastTradedPrice;
    }

    // Displays buy and sell orders side by side
    void displayOrders(const std::priority_queue<Order>& buys, const std::priority_queue<Order>& sells) const {
        std::vector<Order> buyOrders;
        std::vector<Order> sellOrders;

        auto buyCopy = buys;
        while (!buyCopy.empty()) {
            buyOrders.push_back(buyCopy.top());
            buyCopy.pop();
        }

        auto sellCopy = sells;
        while (!sellCopy.empty()) {
            sellOrders.push_back(sellCopy.top());
            sellCopy.pop();
        }

        std::sort(buyOrders.begin(), buyOrders.end());
        std::sort(sellOrders.begin(), sellOrders.end());

        size_t maxRows = std::max(buyOrders.size(), sellOrders.size());
        for (size_t i = 0; i < maxRows; ++i) {
            if (i < buyOrders.size()) {
                const auto& order = buyOrders[i];
                std::cout << order.id << " "
                          << (order.isMarketOrder ? "M" : formatPrice(order.limitPrice)) << " "
                          << order.quantity << "\t\t";
            } else {
                std::cout << "\t\t\t\t";
            }

            if (i < sellOrders.size()) {
                const auto& order = sellOrders[i];
                std::cout << order.id << " "
                          << (order.isMarketOrder ? "M" : formatPrice(order.limitPrice)) << " "
                          << order.quantity;
            }

            std::cout << "\n";
        }
    }
};

// Parses an input line into an Order structure
Order parseOrder(const std::string& line, int timestamp) {
    std::istringstream iss(line);
    Order order;
    order.timestamp = timestamp;
    std::string limitPriceStr;

    iss >> order.id >> order.type >> order.quantity;
    if (iss >> limitPriceStr) {
        order.isMarketOrder = false;
        order.limitPrice = std::stod(limitPriceStr);
    } else {
        order.isMarketOrder = true;
        order.limitPrice = 0;
    }
    return order;
}

// Main function to process orders from an input file...(and some error handling + output file)
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./main <input_file>\n";
        return 1;
    }

    std::ifstream inputFile(argv[1]);
    if (!inputFile) {
        std::cerr << "Error: Could not open file " << argv[1] << "\n";
        return 1;
    }
    // Outputing the file with same input(x) number by replcing "input" with "output"....
    std::string inputFilename = argv[1];
    std::string outputFilename = inputFilename;
    size_t inputPos = inputFilename.find("input");
    if (inputPos != std::string::npos) {
        outputFilename.replace(inputPos, 5, "output");
    } else {
        outputFilename = inputFilename.substr(0, inputFilename.find_last_of('.')) + ".out";
    }
    std::ofstream outputFile(outputFilename);

    double initialPrice;
    inputFile >> initialPrice;
    inputFile.ignore();

    OrderBook orderBook(initialPrice);

    std::string line;
    int timestamp = 0;

    // Process each line in the input file
    while (std::getline(inputFile, line)) {
        ++timestamp;
         // Parse and add the new order to the orderbok
        Order order = parseOrder(line, timestamp);
        orderBook.addOrder(order);
        // Display the current state of the order book before matching...
        std::cout << "\nBefore Matching:\n";
        orderBook.displayPendingOrders();
         // Match and execute the orders
        orderBook.matchOrders(outputFile);
        // Now finally display the updated state of the order book after matching...
        std::cout << "\nAfter Matching:\n";
        orderBook.displayPendingOrders();
    }

    std::cout << "\nFinal State of Orders:\n";
    orderBook.displayPendingOrders();
    orderBook.writeUnexecutedOrders(outputFile);
    return 0;
}