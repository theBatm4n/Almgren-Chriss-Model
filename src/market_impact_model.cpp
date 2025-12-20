#include "market_impact_model.hpp"
#include <cmath>
#include <stdexcept>
#include <random>
#include <iostream>
#include <format>

// Initialize static random members
std::random_device AlmgrenChrissModel::rd_{};
std::mt19937 AlmgrenChrissModel::rng_{AlmgrenChrissModel::rd_()};
std::normal_distribution<double> AlmgrenChrissModel::norm_{0.0, 1.0};

void AlmgrenChrissModel::updateDerivedParameters_(){
    if(eta_ <= 0 || lambda_ < 0 || timeHorizon_ <= 0){
        throw std::invalid_argument("Invalid parameters: eta, lambda, timeHorizon must be positive");
    }
    
    // Handle risk-neutral case (lambda = 0)
    if (lambda_ == 0.0) {
        kappa_ = 0.0;
        sinhKappaT_ = 0.0;
        coshKappaT_ = 1.0;
    } else {
        kappa_ = std::sqrt(lambda_ * sigma_ * sigma_ / eta_);
        std::cout << "value of kappa" << kappa_ << std::endl;
        // Check for valid kappa before calculating hyperbolic functions
        if (std::isnan(kappa_) || std::isinf(kappa_)) {
            throw std::invalid_argument("Invalid kappa calculation - check parameters");
        }
        sinhKappaT_ = std::sinh(kappa_ * timeHorizon_);
        coshKappaT_ = std::cosh(kappa_ * timeHorizon_);
    }

    reset();
    
    std::cout << "Derived params: κ=" << kappa_ << ", sinh(κT)=" << sinhKappaT_ 
              << ", cosh(κT)=" << coshKappaT_ << std::endl;
}

void AlmgrenChrissModel::setParameters(double sigma, double gamma, double eta, double lambda,
                                    double initialPrice, double totalShares, double timeHorizon){
    // Use more reasonable parameter ranges
    if(sigma <= 0 || sigma > 1.0) {
        throw std::invalid_argument("sigma must be in (0, 1.0]");
    }
    if(eta <= 0 || eta > 1e-3) {
        throw std::invalid_argument("eta must be in (0, 1e-3]");
    }
    if(timeHorizon <= 0) {
        throw std::invalid_argument("timeHorizon must be positive");
    }

    sigma_ = sigma;
    gamma_ = gamma;
    eta_ = eta;
    lambda_ = lambda;
    initialPrice_ = initialPrice;
    totalShares_ = totalShares;
    timeHorizon_ = timeHorizon;

    updateDerivedParameters_();
    
    std::cout << "Model params: σ=" << sigma_ << " γ=" << gamma_ 
              << " η=" << eta_ << " λ=" << lambda_ << " S₀=" << initialPrice_ 
              << " X=" << totalShares_ << " T=" << timeHorizon_ << std::endl;
}

double AlmgrenChrissModel::computeRemainingShares(double t) const {
    if (t < 0 || t > timeHorizon_) {
        throw std::out_of_range(std::format("Time must be in [0, {}], got {}", timeHorizon_, t));
    }
    
    if (lambda_ == 0.0 || kappa_ == 0.0) {
        return totalShares_ * (1.0 - t / timeHorizon_);
    }
    
    // Handle very small kappaT to avoid numerical issues
    if (std::abs(kappa_ * timeHorizon_) < 1e-10) {
        return totalShares_ * (1.0 - t / timeHorizon_);
    }
    
    double result = totalShares_ * std::sinh(kappa_ * (timeHorizon_ - t)) / sinhKappaT_;
    
    // Ensure result is valid
    if (std::isnan(result) || std::isinf(result)) {
        return totalShares_ * (1.0 - t / timeHorizon_); // Fallback to linear
    }
    
    return std::max(0.0, std::min(totalShares_, result));
}

double AlmgrenChrissModel::computeTradingRate(double t) const {
    if (t < 0 || t > timeHorizon_) {
        throw std::out_of_range(std::format("Time must be in [0, {}], got {}", timeHorizon_, t));
    }
    
    // Handle risk-neutral case (lambda = 0) - constant rate
    if (lambda_ == 0.0 || kappa_ == 0.0) {
        return totalShares_ / timeHorizon_;
    }
    
    // Handle very small kappaT to avoid numerical issues
    if (std::abs(kappa_ * timeHorizon_) < 1e-10) {
        return totalShares_ / timeHorizon_;
    }
    
    double result = totalShares_ * kappa_ * std::cosh(kappa_ * (timeHorizon_ - t)) / sinhKappaT_;
    
    // Ensure result is valid
    if (std::isnan(result) || std::isinf(result)) {
        return totalShares_ / timeHorizon_; // Fallback to constant rate
    }
    
    return std::max(0.0, result);
}

std::vector<double> AlmgrenChrissModel::calculateOptimalSchedule(int intervals) {
    std::vector<double> schedule;
    schedule.reserve(intervals);
    
    double timeStep = timeHorizon_ / intervals;
    
    for (int i = 0; i < intervals; ++i) {
        double t_start = i * timeStep;
        double t_end = (i + 1) * timeStep;
        
        double shares_at_start = computeRemainingShares(t_start);
        double shares_at_end = computeRemainingShares(t_end);
        double shares_to_trade = shares_at_start - shares_at_end;
        
        schedule.push_back(std::max(0.0, shares_to_trade));
    }
    
    std::cout << "Optimal schedule (" << intervals << " intervals): ";
    double total = 0.0;
    for (double shares : schedule) {
        std::cout << static_cast<int>(shares) << " ";
        total += shares;
    }
    std::cout << " | Total: " << static_cast<int>(total) << std::endl;
    
    return schedule;
}

double AlmgrenChrissModel::simulatePriceStep(double dt){
    if (elapsedTime_ >= timeHorizon_) {
        return currentPrice_; // Already finished
    }
    
    if (elapsedTime_ + dt > timeHorizon_) {
        dt = timeHorizon_ - elapsedTime_; // don't overshoot the horizon
    }

    double v = computeTradingRate(elapsedTime_);
    const double dW = norm_(rng_) * std::sqrt(dt);

    // Price update: dS = -γ v dt + σ dW
    double permanentImpact = -gamma_ * v * dt ; // Scale for reasonable impact
    double randomWalk = sigma_ * dW * initialPrice_; // Scale volatility by price
    
    double priceChange = permanentImpact + randomWalk;
    currentPrice_ += priceChange;
    elapsedTime_ += dt;

    if (currentPrice_ <= 0.0){
        throw std::runtime_error("Price became negative - check parameters");
    }

    std::cout << "Price step: dt=" << dt << ", v=" << v << ", dW=" << dW 
              << ", dS=" << priceChange << ", S=" << currentPrice_ << std::endl;
    
    return currentPrice_;
}

void AlmgrenChrissModel::reset() {
    elapsedTime_ = 0.0;
    executedShares_ = 0.0;
    currentPrice_ = initialPrice_;
}

void AlmgrenChrissModel::printState() const {
    std::cout << std::format(
        "State: t={:.2f}/{:.2f}, x(t)={:.0f}/{:.0f}, S(t)={:.2f}", 
        elapsedTime_, timeHorizon_, executedShares_, totalShares_, currentPrice_
    ) << std::endl;
}