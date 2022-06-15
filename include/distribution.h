#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

#include <vector>
#include "BTC.h"

using namespace std;
enum class distribution_type {normal, lognormal, dirichlet, none};

class Distribution
{
public:
    Distribution();
    Distribution(const Distribution &dist);
    Distribution& operator = (const Distribution &dist);
    double Eval(const double &x);
    double EvalLog(const double &x);
    CTimeSeries<double> EvaluateAsTimeSeries(int numberofpoint=100, const double &stdcoeff = 4);
    vector<double> parameters;
    distribution_type distribution;
    void SetType(const distribution_type &typ);
    double Mean();
private:
    double pi;
};

#endif // DISTRIBUTION_H
