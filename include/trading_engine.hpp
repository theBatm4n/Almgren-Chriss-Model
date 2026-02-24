#pragma once

#include "execution_scheduler.hpp"
#include "market_impact_model.hpp"
#include "market_data.hpp"
#include "execution_metrics.hpp"
#include <memory>
#include <vector>
#include <map>

class TradingEngine {
public:
    using ExecutionCallback = std::function<void(const std::string& orderId,
        const std::string& symbol,
        double Shares,
        double price,
        double totalExecuted,
        double totalShares)>;
    
    using StatusCallback = std::function<void(const std::string& orderId, OrderStatus status)>;
    using ProgressCallback = std::function<void(const std::string& orderId, double progressPercent)>;

    TradingEngine();
    ~TradingEngine();

    void initialize(const std::string& configPath = "");
    void shutdown();

    struct Order {
        std::string symbol;
        int totalShares;
        bool isBuy;
        double initialPrice;
        double timeHorizon;
        double riskAversion;
        std::string orderId;
        int numIntervals{10};
    };

    std::string submitOrder(const Order& order);
    void cancelOrder(const std::string& orderId);
    OrderStatus getOrderStatus(const std::string& orderId) const;

    void startExecution(const std::string& orderId);
    void pauseExecution(const std::string& orderId);
    void resumeExecution(const std::string& orderId);

    void onMarketDataUpdate(const MarketData& data);
    void onExecutionReport(const ExecutionReport& report);

    std::vector<Order> getActiveOrder() const;
    ExecutionMetrics getOrderMetrics(const std::string& orderId) const;
    std::vector<double> getRemainingSchedule(const std::string& orderId) const;
    int calculateOptimalIntervalCount_(int totalShares);

    void setExecutionCallback(ExecutionCallback callback){
        executionCallback_ = callback;
    }
    void setStatusCallback(StatusCallback callback) {
        statusCallback_ = callback;
    }
    void setProgressCallback(ProgressCallback callback) { 
        progressCallback_ = callback;
    }

private:
    struct OrderExecutionContext{
        Order order;
        AlmgrenChrissModel model;
        
        std::vector<double> optimalSchedule;
        size_t currentScheduleIndex{0};
        double executedShares{0.0};
        double averageExecutionPrice{0.0};
        std::vector<std::pair<double, double>> executionHistory; // time, price
        OrderStatus status{OrderStatus::PENDING};

        double remainingTime() const{
            return order.timeHorizon - executedShares;
        }
    };

    ExecutionCallback executionCallback_;
    StatusCallback statusCallback_;
    ProgressCallback progressCallback_;

    execution_scheduler scheduler_;
    std::map<std::string, OrderExecutionContext> activeOrders_;

    std::map<std::string, MarketData> currentMarketData_;

    mutable std::mutex orderMutex_;
    mutable std::mutex marketDataMutex_;

    void calculateOptimalSchedule_(OrderExecutionContext& context);
    void scheduleNextChunk_(const std::string& orderId);
    void executeTradeChunk_(const std::string& orderId, double shares);
    void updateModelWithExecution_(const std::string& orderId, double executedShares, double price);
    void handleCompletedOrder_(const std::string& orderId);
    void adjustScheduleDynamically_(const std::string& orderId, const MarketData& newData);
    
    void createExecutionTask_(const std::string& orderId, double sharesToExecute);

    void emitExecution(const std::string& orderId, const std::string& symbol,
                             double shares, double price,
                             double totalExecuted, double totalShares)
    {
        if (executionCallback_) {
            executionCallback_(orderId, symbol, shares, price, totalExecuted, totalShares);
        }
    }
        void emitStatus(const std::string& orderId, OrderStatus status) {
        if (statusCallback_) {
            statusCallback_(orderId, status);
        }
    }
    
    void emitProgress(const std::string& orderId, double progressPercent) {
        if (progressCallback_) {
            progressCallback_(orderId, progressPercent);
        }
    }
};