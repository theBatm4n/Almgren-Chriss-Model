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

    double getElapsedTime() const { return elapsedTime_; }
    double getExecutedShares() const { return executedShares_; }
    double getCurrentPrice() const { return currentPrice_; }
    double getKappa() const { return kappa_; }
    
    void reset();
    void printState() const;

private:
    void updateDerivedParameters_();

    double sigma_ = 0.02;         
    double gamma_ = 2.5e-6;      
    double eta_ = 1.0e-6;        
    double lambda_ = 1.0;         
    double initialPrice_ = 150.0;  
    double totalShares_ = 100000;  
    double timeHorizon_ = 3600.0;  

    double kappa_ = 0.0;           
    double sinhKappaT_ = 0.0;     
    double coshKappaT_ = 1.0;      


    double elapsedTime_ = 0.0;    
    double executedShares_ = 0.0; 
    double currentPrice_ = 150.0;  

    // Random number generation - as static members for simplicity
    static std::random_device rd_;
    static std::mt19937 rng_;
    static std::normal_distribution<double> norm_;
};