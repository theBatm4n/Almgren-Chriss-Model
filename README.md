# Almgren-Chriss Optimal Execution Engine

## What is this project?

A trading engine that helps break up large stock orders into smaller pieces to minimize trading costs. It's based on the Almgren-Chriss mathematical model.

## The Problem

When you want to buy or sell a large number of shares:
- **Trade too fast** → You move the market price against yourself (market impact)
- **Trade too slow** → The market might move against you randomly (timing risk)

## How it works

The engine finds the perfect balance between these two risks by calculating an optimal trading schedule.

### Price Simulation

The model simulates price changes using this simple equation:
Price change = (Your trading impact) + (Random market noise)

text

- **Your trading impact**: When you sell, price goes down; when you buy, price goes up
- **Random market noise**: Normal up/down movements that happen anyway

## Technical Architecture

- **C++ core**: Fast, multi-threaded execution engine
- **Python bindings**: Easy to use from Python (via pybind11)
- **Multi-threaded design**: 
  - Multiple threads can submit orders
  - One dedicated thread executes the trades

## Features

- Submit large orders with custom parameters
- Get optimal trading schedules
- Track execution progress
- Monitor performance metrics
- Real-time callbacks for execution updates

## Quick Example

```python
import almgren_chriss as ac

# Create engine
engine = ac.TradingEngine()
engine.initialize()

# Submit a large order
order_id = engine.submit_order(
    symbol="AAPL",
    total_shares=100000,
    is_buy=False,  # Sell order
    initial_price=150.0,
    time_horizon=3600,  # Execute over 1 hour
    risk_aversion=0.1
)

# Start execution
engine.start_execution()

# Track progress
metrics = engine.get_order_metrics(order_id)
print(f"Executed: {metrics.executed_shares}/{metrics.total_shares}")
print(f"Average price: {metrics.average_execution_price}")
```
Why use this?
Minimize trading costs for large orders

Handle institutional-sized trades

Quantify the trade-off between speed and cost

Real-time execution monitoring

<img width="2542" height="1294" alt="image" src="https://github.com/user-attachments/assets/16217c5e-a9c1-48c2-aaf2-af3167627bef" />


