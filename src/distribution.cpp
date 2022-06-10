#include "distribution.h"
#include "math.h"

Distribution::Distribution()
{
    pi = 4 * atan(1.0);
}

double Distribution::Eval(const double &x)
{
    double out = 0;
    if (distribution==distribution_type::normal)
            out = 1.0 / (sqrt(2*pi)*parameters[1])*exp(-pow(x - parameters[0], 2) / (2 * parameters[1]*parameters[1]));
    if (distribution==distribution_type::lognormal)
            out = 1.0 / (sqrt(2*pi)*parameters[1] * x)*exp(-pow(log(x) - parameters[0], 2) / (2 * parameters[1] * parameters[1]));

    return out;
}

CTimeSeries<double> Distribution::EvaluateAsTimeSeries(int numberofpoints, const double &stdcoeff)
{
    CTimeSeries<double> out;
    double x;
    for (int i=0; i<numberofpoints; i++)
    {
        if (distribution == distribution_type::normal)
        {
            x = parameters[0] - stdcoeff*parameters[1] + i*2*stdcoeff*parameters[1]/(numberofpoints-1);
        }
        else if (distribution == distribution_type::lognormal)
        {
            x = exp(parameters[0] - stdcoeff*parameters[1] + i*2*stdcoeff*parameters[1]/(numberofpoints-1));
        }
        out.append(x,Eval(x));
    }
    return out;
}

Distribution::Distribution(const Distribution &dist)
{
    pi = 4 * atan(1.0);
    parameters = dist.parameters;
    distribution = dist.distribution;
}
Distribution& Distribution::operator = (const Distribution &dist)
{
    pi = 4 * atan(1.0);
    parameters = dist.parameters;
    distribution = dist.distribution;
    return *this;
}
