#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/chrono.h>
#include <iostream>
#include "../include/trading_engine.hpp"
#include "../include/execution_metrics.hpp"

namespace py = pybind11;

PYBIND11_MODULE(almgren_chriss, m) {
    m.doc() = "Almgren-Chriss Optimal Execution Engine";
    
    // Order struct (nested in TradingEngine)
    py::class_<TradingEngine::Order>(m, "Order")
        .def(py::init<>())
        .def_readwrite("symbol", &TradingEngine::Order::symbol)
        .def_readwrite("total_shares", &TradingEngine::Order::totalShares)
        .def_readwrite("is_buy", &TradingEngine::Order::isBuy)
        .def_readwrite("initial_price", &TradingEngine::Order::initialPrice)
        .def_readwrite("time_horizon", &TradingEngine::Order::timeHorizon)
        .def_readwrite("risk_aversion", &TradingEngine::Order::riskAversion)
        .def_readwrite("order_id", &TradingEngine::Order::orderId);
    
    // ExecutionMetrics
    py::class_<ExecutionMetrics>(m, "ExecutionMetrics")
        .def(py::init<>())
        .def_readwrite("total_shares", &ExecutionMetrics::totalShares)
        .def_readwrite("executed_shares", &ExecutionMetrics::executedShares)
        .def_readwrite("average_execution_price", &ExecutionMetrics::averageExecutionPrice)
        .def_readwrite("implementation_shortfall", &ExecutionMetrics::implementationShortfall);
    
    // OrderStatus enum
    py::enum_<OrderStatus>(m, "OrderStatus")
        .value("PENDING", OrderStatus::PENDING)
        .value("ACTIVE", OrderStatus::ACTIVE)
        .value("COMPLETED", OrderStatus::COMPLETED)
        .value("CANCELLED", OrderStatus::CANCELLED)
        .export_values();
    
    // TradingEngine 
    py::class_<TradingEngine>(m, "TradingEngine")
        .def(py::init<>())
        .def("initialize", &TradingEngine::initialize, py::arg("config_path") = "")
        .def("shutdown", &TradingEngine::shutdown)
        .def("submit_order", &TradingEngine::submitOrder)
        .def("start_execution", &TradingEngine::startExecution)
        .def("get_order_status", &TradingEngine::getOrderStatus)
        .def("get_order_metrics", &TradingEngine::getOrderMetrics)
        .def("get_remaining_schedule", &TradingEngine::getRemainingSchedule)
        .def("set_execution_callback", [](TradingEngine& engine, py::function callback) {
            engine.setExecutionCallback([callback](const std::string& orderId,
                                                   const std::string& symbol,
                                                   double shares,
                                                   double price,
                                                   double totalExecuted,
                                                   double totalShares) {
                // Acquire GIL for Python thread safety
                py::gil_scoped_acquire acquire;
                try {
                    callback(orderId, symbol, shares, price, totalExecuted, totalShares);
                } catch (const py::error_already_set& e) {
                    // Python exception occurred in callback
                    std::cerr << "Python callback error: " << e.what() << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "C++ callback error: " << e.what() << std::endl;
                }
            });
        }, py::arg("callback"))
        .def("set_status_callback", [](TradingEngine& engine, py::function callback) {
            engine.setStatusCallback([callback](const std::string& orderId, OrderStatus status) {
                py::gil_scoped_acquire acquire;
                try {
                    callback(orderId, static_cast<int>(status));
                } catch (const py::error_already_set& e) {
                    std::cerr << "Python callback error: " << e.what() << std::endl;
                }
            });
        }, py::arg("callback"))
        .def("set_progress_callback", [](TradingEngine& engine, py::function callback) {
            engine.setProgressCallback([callback](const std::string& orderId, double progress) {
                py::gil_scoped_acquire acquire;
                try {
                    callback(orderId, progress);
                } catch (const py::error_already_set& e) {
                    std::cerr << "Python callback error: " << e.what() << std::endl;
                }
            });
        }, py::arg("callback"));
}