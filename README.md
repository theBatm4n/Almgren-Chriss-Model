# Almgren Chriss Model

## Goal: How should I break up a large trade to minimize the cost of trading?

### Core problem: Market Impact and timing risk
If you want to trade a large amount of shares, a large buy/sell order will push the price up/down before the order is filled, But trade too slow and the market might move against you.

Almgren Chriss is a model that generates a sequence of numbers representing the number of shares that has to be bought/sold in order to minimize market Impact and timing risk

Minimize: E[x] + Var[x]

### Price Simulation 
We could use websockets and retrieve real market data from exchanges but demo purposes we are simuluating the price with the following equation
dS(t) = -γ v(t) dt + σ dW(t)

dS(t): Price change over small time dt
-γ v(t) dt: Your trading impact
   γ (gamma): Permanent impact coefficient (your influence on price)
   v(t): Your trading rate now (shares/second)
   -sign: Selling (v>0) pushes price DOWN, buying (v<0) pushes UP

σ dW(t): Random market movement
  σ (sigma): Volatility (market randomness magnitude)
  dW(t): Brownian motion (random walk, mean=0, variance=dt)

