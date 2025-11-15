#include "trading_engine.hpp"

TradingEngine::TradingEngine() {
    scheduler_.start();
}

TradingEngine::~TradingEngine() {
    shutdown();
}

std::string TradingEngine::submitOrder(const Order& order) {
    std::lock_guard lock(orderMutex_);

    std::string orderId = generateOrderId_();

    OrderExecutionContext context;
    context.order = order;

    context.model.setParameters(
        /*sigma*/ 0.02,
        /*gamma*/ 2.5e-6,
        /*eta*/ 1.0e-6,
        order.riskAversion,
        order.initialPrice,
        static_cast<double>(order.totalShares),
        order.timeHorizon
    );

    calculateOptimalSchedule_(context);

    activeOrders_[order] = std::move(context);
    return orderId;
}

void TradingEngine::startExecution(const std::string& orderId) {
    std::lock_guard lock(orderMutex_);

    auto it = activeOrders_.find(orderId);
    if (it == activeOrders_.end()){
        throw std::runtime_error("Order not found: " + orderId);
    }

    it->second.status = OrderStatus::ACTIVE;
    scheduleNextChunk_(orderId);
}