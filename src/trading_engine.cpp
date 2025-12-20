#include "trading_engine.hpp"
#include <random>
#include <iostream>
#include <iomanip>

std::string generateOrderId() {
    static int counter = 0;
    return "ORDER_" + std::to_string(++counter);
}

TradingEngine::TradingEngine() {
    scheduler_.start();
}

TradingEngine::~TradingEngine() {
    shutdown();
}

void TradingEngine::initialize(const std::string& configPath) {
    (void)configPath;
    std::cout << "TradingEngine initialized" << std::endl;
}

void TradingEngine::shutdown() {
    scheduler_.stop();
    std::cout << "TradingEngine shutdown" << std::endl;
}

std::string TradingEngine::submitOrder(const Order& order) {
    std::lock_guard lock(orderMutex_);
    
    std::string orderId = generateOrderId();
    
    OrderExecutionContext context;
    context.order = order;
    context.order.orderId = orderId;
    
    // Use EVEN smaller impact parameters for 50,000 shares
    context.model.setParameters(
        0.0020,         // sigma - 0.2% volatility during execution
        1.0e-11,        // gamma - much smaller permanent impact  
        1.0e-04,        // eta - much smaller temporary impact
        order.riskAversion,
        order.initialPrice,
        static_cast<double>(order.totalShares),
        order.timeHorizon
    );
    
    // Calculate optimal execution schedule
    calculateOptimalSchedule_(context);
    
    activeOrders_[orderId] = std::move(context);
    
    std::cout << "Submitted order: " << orderId 
              << " for " << order.totalShares << " shares of " << order.symbol << std::endl;
    
    return orderId;
}

void TradingEngine::cancelOrder(const std::string& orderId) {
    std::lock_guard lock(orderMutex_);
    
    auto it = activeOrders_.find(orderId);
    if (it != activeOrders_.end()) {
        it->second.status = OrderStatus::CANCELLED;
        std::cout << "Cancelled order: " << orderId << std::endl;
    }
}

OrderStatus TradingEngine::getOrderStatus(const std::string& orderId) const {
    std::lock_guard lock(orderMutex_);
    
    auto it = activeOrders_.find(orderId);
    if (it != activeOrders_.end()) {
        return it->second.status;
    }
    return OrderStatus::FAILED;
}

void TradingEngine::startExecution(const std::string& orderId) {
    std::lock_guard lock(orderMutex_);
    
    auto it = activeOrders_.find(orderId);
    if (it == activeOrders_.end()) {
        std::cout << "Order not found: " << orderId << std::endl;
        return;
    }
    
    it->second.status = OrderStatus::ACTIVE;
    std::cout << "Starting execution for: " << orderId << std::endl;
    
    // Schedule the first chunk
    scheduleNextChunk_(orderId);
}

void TradingEngine::pauseExecution(const std::string& orderId) {
    std::lock_guard lock(orderMutex_);
    
    auto it = activeOrders_.find(orderId);
    if (it != activeOrders_.end()) {
        it->second.status = OrderStatus::PAUSED;
        std::cout << "Paused execution for: " << orderId << std::endl;
    }
}

void TradingEngine::resumeExecution(const std::string& orderId) {
    std::lock_guard lock(orderMutex_);
    
    auto it = activeOrders_.find(orderId);
    if (it != activeOrders_.end() && it->second.status == OrderStatus::PAUSED) {
        it->second.status = OrderStatus::ACTIVE;
        std::cout << "Resumed execution for: " << orderId << std::endl;
        scheduleNextChunk_(orderId);
    }
}

void TradingEngine::onMarketDataUpdate(const MarketData& data) {
    std::lock_guard lock(marketDataMutex_);
    currentMarketData_[data.symbol] = data;
    // adjust schedules based on market data
}

void TradingEngine::onExecutionReport(const ExecutionReport& report) {
    (void)report; 
}

int TradingEngine::calculateOptimalIntervalCount_(int totalShares) {
    // Heuristic: More shares → more intervals (but not linear!)
    if (totalShares <= 1000) return 5;       
    if (totalShares <= 10000) return 10;      
    if (totalShares <= 100000) return 20; 
    if (totalShares <= 1000000) return 50;   
    return 100;                            
}

void TradingEngine::calculateOptimalSchedule_(OrderExecutionContext& context) {
    int numIntervals = context.order.numIntervals;

    if (numIntervals <= 0){
        numIntervals = calculateOptimalIntervalCount_(context.order.totalShares);
    }

    context.optimalSchedule = context.model.calculateOptimalSchedule(numIntervals); // 5 intervals
    
    // Validate the schedule
    double total = 0.0;
    for (double shares : context.optimalSchedule) {
        total += shares;
    }
    
    // Handle rounding errors
    if (std::abs(total - context.order.totalShares) > 1.0) {
        std::cerr << "Warning: Schedule total " << total 
                  << " doesn't match order total " << context.order.totalShares 
                  << std::endl;
        // Adjust last element to fix
        if (!context.optimalSchedule.empty()) {
            double diff = context.order.totalShares - total;
            context.optimalSchedule.back() += diff;
        }
    }
    
    std::cout << "Optimal schedule (Almgren-Chriss): ";
    for (double shares : context.optimalSchedule) {
        std::cout << static_cast<int>(shares) << " ";
    }
    std::cout << std::endl;
}


void TradingEngine::scheduleNextChunk_(const std::string& orderId) {
    auto it = activeOrders_.find(orderId);
    if (it == activeOrders_.end() || it->second.status != OrderStatus::ACTIVE) {
        return;
    }
    
    auto& context = it->second;
    
    if (context.currentScheduleIndex >= context.optimalSchedule.size()) {
        handleCompletedOrder_(orderId);
        return;
    }
    
    double sharesToExecute = context.optimalSchedule[context.currentScheduleIndex];
    
    // If each chunk should take proportional time based on schedule
    double totalTime = context.order.timeHorizon; 
    size_t totalChunks = context.optimalSchedule.size(); 
    double timePerChunk = totalTime / totalChunks; 
    auto delay = std::chrono::milliseconds(static_cast<int>(timePerChunk * 1000));
    
    scheduler_.scheduleAfter(delay, [this, orderId, sharesToExecute]() {
        this->executeTradeChunk_(orderId, sharesToExecute);
    });
    
    context.currentScheduleIndex++;
}

void TradingEngine::executeTradeChunk_(const std::string& orderId, double shares) {
    std::lock_guard lock(orderMutex_);
    
    auto it = activeOrders_.find(orderId);
    if (it == activeOrders_.end() || it->second.status != OrderStatus::ACTIVE) {
        return;
    }
    
    auto& context = it->second;
    
    double executionPrice = context.model.simulatePriceStep(1.0); 
    if (context.order.isBuy) {
        executionPrice *= 1.001; 
    } else {
        executionPrice *= 0.999; 
    }
    
    // Update execution state
    context.executedShares += shares;
    context.executionHistory.emplace_back(context.model.getElapsedTime(), executionPrice);
    
    // Recalculate running average price (VWAP)
    double totalValue = context.averageExecutionPrice * (context.executedShares - shares);
    totalValue += shares * executionPrice;
    context.averageExecutionPrice = totalValue / context.executedShares;
    
    std::cout << "Executed " << static_cast<int>(shares) << " shares of " << context.order.symbol
              << " @ $" << std::fixed << std::setprecision(2) << executionPrice 
              << " for order " << orderId << std::endl;
    
    // Schedule next chunk or complete order
    if (context.executedShares >= context.order.totalShares) {
        handleCompletedOrder_(orderId);
    } else {
        scheduleNextChunk_(orderId);
    }
}

void TradingEngine::updateModelWithExecution_(const std::string& orderId, double executedShares, double price) {
    (void)orderId;
    (void)executedShares;
    (void)price;
}

void TradingEngine::handleCompletedOrder_(const std::string& orderId) {
    auto it = activeOrders_.find(orderId);
    if (it != activeOrders_.end()) {
        it->second.status = OrderStatus::COMPLETED;
        
        std::cout << "✅ Order COMPLETED: " << orderId 
                  << " | Total shares: " << it->second.executedShares
                  << " | Avg price: $" << std::fixed << std::setprecision(2) 
                  << it->second.averageExecutionPrice << std::endl;
    }
}

// Simple implementations for demo purposes
std::vector<TradingEngine::Order> TradingEngine::getActiveOrder() const {
    std::lock_guard lock(orderMutex_);
    std::vector<Order> orders;
    for (const auto& [id, context] : activeOrders_) {
        if (context.status == OrderStatus::ACTIVE || context.status == OrderStatus::PENDING) {
            orders.push_back(context.order);
        }
    }
    return orders;
}

ExecutionMetrics TradingEngine::getOrderMetrics(const std::string& orderId) const {
    std::lock_guard lock(orderMutex_);
    ExecutionMetrics metrics;
    
    auto it = activeOrders_.find(orderId);
    if (it != activeOrders_.end()) {
        const auto& context = it->second;
        metrics.totalShares = context.order.totalShares;
        metrics.executedShares = context.executedShares;
        metrics.averageExecutionPrice = context.averageExecutionPrice;
        // Simplified calculations for demo
        metrics.implementationShortfall = (context.averageExecutionPrice - context.order.initialPrice) * context.executedShares;
    }
    
    return metrics;
}

std::vector<double> TradingEngine::getRemainingSchedule(const std::string& orderId) const {
    std::lock_guard lock(orderMutex_);
    
    auto it = activeOrders_.find(orderId);
    if (it != activeOrders_.end()) {
        const auto& context = it->second;
        if (context.currentScheduleIndex < context.optimalSchedule.size()) {
            return std::vector<double>(
                context.optimalSchedule.begin() + context.currentScheduleIndex,
                context.optimalSchedule.end()
            );
        }
    }
    return {};
}