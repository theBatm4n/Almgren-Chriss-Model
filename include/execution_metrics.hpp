#pragma once

#include <chrono>

// execution_metrics.hpp  
struct ExecutionMetrics {
    double totalShares;
    double executedShares;
    double averageExecutionPrice;
    double implementationShortfall;  // Actual cost vs arrival price
    double marketImpactCost;
    double timingRiskCost;
    double vwapBenchmark;           // Comparison to VWAP
    std::chrono::milliseconds executionTime;
    
    void printReport() const;
};

enum class OrderStatus {
    PENDING,
    ACTIVE, 
    PAUSED,
    COMPLETED,
    CANCELLED,
    FAILED
};

struct ExecutionReport {
    std::string orderId;
    double executedShares;
    double executionPrice;
    std::string executionId;
    std::chrono::system_clock::time_point executionTime;
};