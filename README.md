# Almgren Chriss Model

## Goal: 
How should I break up a large trade to minimize the cost of trading?

### Core Problem: Market Impact and Timing Risk
If you want to trade a large amount of shares, a large buy/sell order will push the price up/down before the order is filled. However, if you trade too slowly, the market might move against you.

The Almgren Chriss model generates a sequence of numbers representing the number of shares that should be bought/sold in order to minimize market impact and timing risk.

### Objective:
Minimize: \( E[x] + Var[x] \)

### Price Simulation
We could use websockets to retrieve real market data from exchanges, but for demonstration purposes, we are simulating the price with the following equation:

\[ dS(t) = -**γ** v(t) dt + **σ** dW(t) \]

Where:
- **dS(t)**: Price change over small time dt
- **-γ v(t) dt**: Your trading impact
  - **γ (gamma)**: Permanent impact coefficient (your influence on price)
  - **v(t)**: Your trading rate now (shares/second)
  - **-sign**: Selling (v>0) pushes price DOWN, buying (v<0) pushes price UP
- **σ dW(t)**: Random market movement
  - **σ (sigma)**: Volatility (magnitude of market randomness)
  - **dW(t)**: Brownian motion (random walk, mean=0, variance=dt)


