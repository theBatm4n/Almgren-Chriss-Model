#pragma once

#include <chrono>

struct MarketData {
    std::string symbol;
    double bidPrice;
    double askPrice;
    int bidSize;
    int askSize;
    double lastPrice;
    double volume;
    std::chrono::system_clock::time_point timestamp;
};