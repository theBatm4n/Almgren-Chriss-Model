#ifndef ALMGREN_CHRISS_MODEL_HPP
#define ALMGREN_CHRISS_MODEL_HPP

#include <cmath>
#include <random>

class AlmgrenChrissModel 
{
public:

    void setParameters(double sigma, double gamma, double eta, double lambda,
                      double initialPrice, double totalShares, double timeHorizon);
    double computeRemainingShares(double t) const;
    double computeTradingRate(double t) const;
    double simulatePriceStep(double dt);
    double getElapsedTime() const { return elapsedTime_; }
    double getExecutedShares() const { return executedShares_; }
    double getCurrentPrice() const { return currentPrice_; }

private:
    // Core model parameters
    double sigma_;          ///< Volatility (σ)
    double gamma_;          ///< Permanent impact coefficient (γ)
    double eta_;            ///< Temporary impact coefficient (η)
    double lambda_;         ///< Risk aversion (λ)
    double initialPrice_;   ///< Initial asset price (S₀)
    double totalShares_;    ///< Total shares to execute (X)
    double timeHorizon_;    ///< Execution horizon (T)

    // Derived parameters
    double kappa_;          ///< κ = sqrt(λσ²/η)
    double sinhKappaT_;     ///< sinh(κT), cached for efficiency
    double coshKappaT_;     ///< cosh(κT), cached for efficiency

    // Execution state
    double elapsedTime_ = 0.0;      ///< Time elapsed since execution start
    double executedShares_ = 0.0;   ///< Shares executed so far (X - x(t))
    double currentPrice_;           ///< Current simulated price (Sₜ)

    // Random number generation
    std::default_random_engine rng_;                ///< Random number generator
    std::normal_distribution<double> norm_{0.0, 1.0}; ///< Standard normal distribution

    void updateDerivedParameters_();
};

#endif // ALMGREN_CHRISS_MODEL_HPP