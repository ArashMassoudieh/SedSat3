#ifndef PARAMETER_H
#define PARAMETER_H

#include "distribution.h"

enum class _range {low, high};

class Parameter
{
public:
    Parameter();
    void SetPriorDistribution(distribution_type dist_type) {prior_distribution = dist_type;}
    distribution_type GetPriorDistribution() {return prior_distribution;}
    double GetVal(const string &quantity);
    void SetRange(const vector<double> &rng);
    double GetRange(_range lowhigh);
    void SetRange(_range, double value);

private:
    distribution_type prior_distribution;
    vector<double> range;
};

#endif // PARAMETER_H
