#pragma once

#include <vector>
#include <random>

class AlmgrenChrissModel {
public:
    void setParameters(double sigma, double gamma, double eta, double lambda,
                      double initialPrice, double totalShares, double timeHorizon);
    
    std::vector<double> calculateOptimalSchedule(int intervals = 10);
    double computeRemainingShares(double t) const;
    double computeTradingRate(double t) const;
    double simulatePriceStep(double dt);
    
    // Getters
    double getElapsedTime() const { return elapsedTime_; }
    double getExecutedShares() const { return executedShares_; }
    double getCurrentPrice() const { return currentPrice_; }
    double getKappa() const { return kappa_; }
    
    // Utility methods
    void reset();
    void printState() const;

private:
    void updateDerivedParameters_();

    // Core model parameters
    double sigma_ = 0.02;          ///< Volatility (σ)
    double gamma_ = 2.5e-6;        ///< Permanent impact coefficient (γ)
    double eta_ = 1.0e-6;          ///< Temporary impact coefficient (η)
    double lambda_ = 1.0;          ///< Risk aversion (λ)
    double initialPrice_ = 150.0;  ///< Initial asset price (S₀)
    double totalShares_ = 100000;  ///< Total shares to execute (X)
    double timeHorizon_ = 3600.0;  ///< Execution horizon (T)

    // Derived parameters
    double kappa_ = 0.0;           ///< κ = sqrt(λσ²/η)
    double sinhKappaT_ = 0.0;      ///< sinh(κT), cached for efficiency
    double coshKappaT_ = 1.0;      ///< cosh(κT), cached for efficiency

    // Execution state
    double elapsedTime_ = 0.0;     ///< Time elapsed since execution start
    double executedShares_ = 0.0;  ///< Shares executed so far
    double currentPrice_ = 150.0;  ///< Current simulated price (Sₜ)

    // Random number generation - as static members for simplicity
    static std::random_device rd_;
    static std::mt19937 rng_;
    static std::normal_distribution<double> norm_;
};