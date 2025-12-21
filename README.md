# Almgren-Chriss-Model

## Goal: How should I break up a large trade to minimize the cost of trading?

### Core problem: Market Impact and timing risk
If you want to trade a large amount of shares, a large buy/sell order will push the price up/down before the order is filled, But trade too slow and the market might move against you.

Almgren Chriss is a model that generates a sequence of numbers representing the number of shares that has to be bought/sold in order to minimize market Impact and timing risk

Minimize: E[x] + Var[x]

