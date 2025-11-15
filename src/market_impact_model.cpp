#include "market_impact_model.hpp"
#include <cmath>
#include <stdexcept>
#include <format>

void AlmgrenChrissModel::updateDerivedParameters_(){
    if(eta_ <= 0 || lambda_ < 0 || timeHorizon_ <= 0){
        throw std::invalid_argument("Invalid parameters: eta, lambda, timeHorizon must be positive");
    }
    kappa_ = std::sqrt(lambda_ * sigma_ * sigma_ / eta_);
    sinhKappaT_ = std::sinh(kappa_ * timeHorizon_);
    coshKappaT_ = std::cosh(kappa_ * timeHorizon_);

    // reset execution state when parameters change 
    elapsedTime_ = 0.0;  
    executedShares_ = 0.0;
    currentPrice_ = initialPrice_;
}

void AlmgrenChrissModel::setParameters(double sigma, double gamma, double eta, double lambda,
                                    double initialPrice, double totalShares, double timeHorizon){
    if(sigma <= 0; eta <= 0 || timeHorizon <= 0){
        throw std::invalid_argument("sigma, eta, and timeHorizon must be positive");
    }

    sigma_ = sigma;
    gamma_ = gamma;
    eta_ = eta;
    lambda_ = lambda;
    initialPrice_ = initialPrice;
    totalShares_ = totalShares;
    timeHorizon_ = timeHorizon;

    updateDerivedParameters_();
}

double AlmgrenChrissModel::computeRemainingShares(double t) const{
    if (t < 0 || t > timeHorizon_){
        throw std::format("Time must be in [0 , {}]", timeHorizon_);
    }
    return totalShares_ * std::sinh(kappa_ * (timeHorizon_ - t)) / sinhKappaT_;
}

double AlmgrenChrissModel::computeTradingRate(double t) const{
    if (t < 0 || t > timeHorizon_){
        throw std::format("Time must be in [0 , {}]", timeHorizon_);
    }
    return totalShares_ * kappa_ * std::cosh(kappa_ * (timeHorizon_ - t)) / sinhKappaT_;
}

double AlmgrenChrissModel::simulatePriceStep(double dt){
    if(elapsedTime_ + dt > timeHorizon_){
        dt = timeHorizon_ - elapsedTime_; // dont overshoot the horizon
    }

    const double v = computeTradingRate(elapsedTime_);
    const double dW = norm_(rng_) * std::sqrt(dt);  // Browian increment

    // update Price: dS = yv dt + σ dW
    currentPrice_ += gamma_ * v * dt + sigma_ * dW;

    // Update excution state
    elapsedTime_ += dt;
    executedShares_ = totalShares_ - computeRemainingShares(elapsedTime_);

    return currentPrice_;
}
