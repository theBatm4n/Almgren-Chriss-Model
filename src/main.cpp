#include "trading_engine.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>

int main() {
    std::cout << "=== ALMGREN-CHRISS TRADING ENGINE DEMO ===" << std::endl;
    std::cout << "Starting execution..." << std::endl;
    
    try {
        TradingEngine engine;
        engine.initialize();
        
        // Create a sell order
        TradingEngine::Order order;
        order.symbol = "AAPL";
        order.totalShares = 50000;
        order.isBuy = false;  // Selling
        order.initialPrice = 150.0;
        order.timeHorizon = 30.0; // 5 minutes in seconds
        order.riskAversion = 1.0;
        
        std::cout << "\n Order Details:" << std::endl;
        std::cout << "- Symbol: " << order.symbol << std::endl;
        std::cout << "- Shares: " << order.totalShares << std::endl;
        std::cout << "- Side: " << (order.isBuy ? "BUY" : "SELL") << std::endl;
        std::cout << "- Initial Price: $" << order.initialPrice << std::endl;
        std::cout << "- Risk Aversion: " << order.riskAversion << std::endl;
        
        // Submit and execute order
        std::string orderId = engine.submitOrder(order);
        std::cout << "\n Starting execution for order: " << orderId << std::endl;
        
        engine.startExecution(orderId);
        
        // Monitor execution progress
        std::cout << "\n Monitoring execution..." << std::endl;
        for (int i = 0; i < 15; ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            
            auto status = engine.getOrderStatus(orderId);
            auto metrics = engine.getOrderMetrics(orderId);
            auto remaining = engine.getRemainingSchedule(orderId);
            
            std::cout << "Progress: " << metrics.executedShares << "/" << metrics.totalShares 
                      << " shares | Status: ";
                      
            switch(status) {
                case OrderStatus::PENDING: std::cout << "PENDING"; break;
                case OrderStatus::ACTIVE: std::cout << "ACTIVE"; break;
                case OrderStatus::COMPLETED: std::cout << "COMPLETED"; break;
                case OrderStatus::CANCELLED: std::cout << "CANCELLED"; break;
                default: std::cout << "UNKNOWN"; break;
            }
            std::cout << " | Remaining chunks: " << remaining.size() << std::endl;
            
            if (status == OrderStatus::COMPLETED) {
                std::cout << " Order completed!" << std::endl;
                break;
            }
        }
        
        // Get final metrics
        auto finalMetrics = engine.getOrderMetrics(orderId);
        std::cout << "\n FINAL EXECUTION METRICS:" << std::endl;
        std::cout << "Total Shares: " << finalMetrics.totalShares << std::endl;
        std::cout << "Executed Shares: " << finalMetrics.executedShares << std::endl;
        std::cout << "Average Execution Price: $" << std::fixed << std::setprecision(2) 
                  << finalMetrics.averageExecutionPrice << std::endl;
        std::cout << "Implementation Shortfall: $" << std::fixed << std::setprecision(2) 
                  << finalMetrics.implementationShortfall << std::endl;
        std::cout << "Initial Price: $" << order.initialPrice << std::endl;
        std::cout << "Price Improvement/Deterioration: $" << std::fixed << std::setprecision(2)
                  << (finalMetrics.averageExecutionPrice - order.initialPrice) << std::endl;
        
        engine.shutdown();
        
    } catch (const std::exception& e) {
        std::cerr << " Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n Demo completed successfully!" << std::endl;
    return 0;
}