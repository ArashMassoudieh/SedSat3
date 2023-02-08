#include "parameter.h"

Parameter::Parameter()
{
    range.resize(2);
}

double Parameter::GetVal(const string &quantity)
{
    if (quantity == "high")
        return range[1];
    else if (quantity == "low")
        return range[0];
    else
        return 0;
}

void Parameter::SetRange(const vector<double> &rng)
{
    if (rng.size()==2)
        range = rng;
    UpdatePriorDistribution();
}

double Parameter::GetRange(_range lowhigh)
{
    if (range.size()==2)
    {
        if (lowhigh ==_range::low)
            return range[0];
        else
            return range[1];
    }
    return 0;
}
void Parameter::SetRange(_range lowhigh, double value)
{
    if (range.size()!=2)
        range.resize(2);
    if (lowhigh ==_range::low)
        range[0] = value;
    else
        range[1] = value;
    UpdatePriorDistribution();

}

void Parameter::SetRange(double low, double high)
{
    if (range.size()!=2)
        range.resize(2);
    range[0] = low;
    range[1] = high;
    
    UpdatePriorDistribution();
}

Parameter::Parameter(const Parameter &param)
{
    range = param.range;
    name = param.name;
    prior_distribution = param.prior_distribution;
}

Parameter& Parameter::operator=(const Parameter &param)
{
    range = param.range;
    name = param.name;
    prior_distribution = param.prior_distribution;
    return *this;
}

void Parameter::UpdatePriorDistribution()
{
    if (prior_distribution.distribution == distribution_type::lognormal)
    {
        prior_distribution.parameters.resize(2);
        prior_distribution.parameters[0] = 0.5*(log(range[0])+log(range[1]));
        prior_distribution.parameters[1] = 0.25*(log(range[1])-log(range[0]));
    }
    else if (prior_distribution.distribution == distribution_type::normal)
    {
        prior_distribution.parameters.resize(2);
        prior_distribution.parameters[0] = 0.5*(range[0]+range[1]);
        prior_distribution.parameters[1] = 0.25*(range[1]-range[0]);
    }

}

double Parameter::CalcLogPriorProbability(const double x)
{
    return prior_distribution.Eval(x);
}


