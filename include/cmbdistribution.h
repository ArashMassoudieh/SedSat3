#ifndef CMBDISTRIBUTION_H
#define CMBDISTRIBUTION_H

#include <vector>
#include "BTC.h"

using namespace std;
enum class distribution_type {normal, lognormal, dirichlet, none, uniform};
enum class parameter_mode {direct, based_on_fitted_distribution};
class Distribution
{
public:
    Distribution();
    Distribution(const Distribution &dist);
    Distribution& operator = (const Distribution &dist);
    double Eval(const double &x);
    static double Eval(const double &x, const vector<double> parameters, distribution_type distribution);
    double EvalLog(const double &x);
    CTimeSeries<double> EvaluateAsTimeSeries(int numberofpoint=100, const double &stdcoeff = 4);
    static CTimeSeries<double> EvaluateAsTimeSeries(int numberofpoints, const double &stdcoeff, const vector<double> parameters, distribution_type &dist_type);
    vector<double> parameters;
    distribution_type distribution = distribution_type::none;
    void SetType(const distribution_type &typ);
    double Mean(parameter_mode param_mode = parameter_mode::based_on_fitted_distribution);
    void SetDataMean(const double &val) {mean_val = val;}
    void SetDataSTDev(const double &val) {std_val = val;}
    double DataMean() {return  mean_val;}
    double DataSTDev() {return std_val;}
    bool operator==(const string &dist_type);
private:
    static double pi;
    double mean_val=0;
    double std_val=0;
};

#endif // CMBDISTRIBUTION_H
