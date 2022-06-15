#ifndef PARAMETER_H
#define PARAMETER_H

#include "distribution.h"

enum class _range {low, high};

class Parameter
{
public:
    Parameter();
    Parameter(const Parameter &param);
    Parameter& operator=(const Parameter &param);
    void SetPriorDistribution(distribution_type dist_type) {prior_distribution.distribution = dist_type;}
    Distribution& GetPriorDistribution() {return prior_distribution;}
    double GetVal(const string &quantity);
    void SetRange(const vector<double> &rng);
    void SetRange(double low, double high);
    double GetRange(_range lowhigh);
    void SetRange(_range, double value);
    void SetName(const string &nam) {name = nam;}
    string Name() {return name;}
    double Value() {return value;}
    void SetValue(double val) {value = val;}
    void UpdatePriorDistribution();

private:
    Distribution prior_distribution;
    vector<double> range;
    string name;
    double value = 0;
};

#endif // PARAMETER_H
