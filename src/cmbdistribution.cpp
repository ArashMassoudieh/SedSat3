#include "cmbdistribution.h"
#include "math.h"

double Distribution::pi = 4 * atan(1.0);

Distribution::Distribution()
{

}

double Distribution::Eval(const double &x)
{
    double out = 0;
    if (distribution==distribution_type::normal)
            out = 1.0 / (sqrt(2*pi)*parameters[1])*exp(-pow(x - parameters[0], 2) / (2 * parameters[1]*parameters[1]));
    if (distribution==distribution_type::lognormal)
            out = 1.0 / (sqrt(2*pi)*parameters[1] * x)*exp(-pow(log(x) - parameters[0], 2) / (2 * parameters[1] * parameters[1]));
    if (distribution==distribution_type::uniform || distribution==distribution_type::dirichlet)
        if (x<0 || x>1)
            out = 1e-32;
    return out;
}

double Distribution::Eval(const double &x, const vector<double> parameters, distribution_type distribution)
{
    double out = 0;
    if (distribution==distribution_type::normal)
            out = 1.0 / (sqrt(2*pi)*parameters[1])*exp(-pow(x - parameters[0], 2) / (2 * parameters[1]*parameters[1]));
    if (distribution==distribution_type::lognormal)
            out = 1.0 / (sqrt(2*pi)*parameters[1] * x)*exp(-pow(log(x) - parameters[0], 2) / (2 * parameters[1] * parameters[1]));
    if (distribution==distribution_type::uniform || distribution==distribution_type::dirichlet)
        if (x<0 || x>1)
            out = 1e-32;
    return out;
}

double Distribution::EvalLog(const double &x)
{
    double out = 0;
    if (distribution==distribution_type::normal)
        out = -log(sqrt(2*pi)*parameters[1]) -pow(x - parameters[0], 2) / (2 * parameters[1]*parameters[1]);
    if (distribution==distribution_type::lognormal)
        out = -log(sqrt(2*pi)*parameters[1]*x) -pow(log(x) - parameters[0], 2) / (2 * parameters[1] * parameters[1]);
    if (distribution==distribution_type::uniform || distribution==distribution_type::dirichlet)
        if (x<0 || x>1)
            out = -1e6;

    return out;
}

TimeSeries<double> Distribution::EvaluateAsTimeSeries(int numberofpoints, const double &stdcoeff)
{
    TimeSeries<double> out;
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

TimeSeries<double> Distribution::EvaluateAsTimeSeries(int numberofpoints, const double &stdcoeff, const vector<double> parameters, distribution_type &dist_type)
{
    TimeSeries<double> out;
    double x;
    for (int i=0; i<numberofpoints; i++)
    {
        if (dist_type == distribution_type::normal)
        {
            x = parameters[0] - stdcoeff*parameters[1] + i*2*stdcoeff*parameters[1]/(numberofpoints-1);
        }
        else if (dist_type == distribution_type::lognormal)
        {
            x = exp(parameters[0] - stdcoeff*parameters[1] + i*2*stdcoeff*parameters[1]/(numberofpoints-1));
        }
        out.append(x,Eval(x,parameters,dist_type));
    }
    return out;
}


Distribution::Distribution(const Distribution &dist)
{
    pi = 4 * atan(1.0);
    parameters = dist.parameters;
    distribution = dist.distribution;
    mean_val = dist.mean_val;
    std_val = dist.std_val;
    SetType(dist.distribution);
}
Distribution& Distribution::operator = (const Distribution &dist)
{
    pi = 4 * atan(1.0);
    parameters = dist.parameters;
    distribution = dist.distribution;
    mean_val = dist.mean_val;
    std_val = dist.std_val;
    SetType(dist.distribution);
    return *this;
}

void Distribution::SetType(const distribution_type &typ)
{
    distribution = typ;
    if (typ == distribution_type::lognormal)
        parameters.resize(2);
    if (typ == distribution_type::normal)
        parameters.resize(2);
}


double Distribution::Mean(parameter_mode param_mode)
{
    if (param_mode==parameter_mode::direct)
        return mean_val;

    if (distribution == distribution_type::lognormal)
        return exp(parameters[0]+pow(parameters[1],2)/2);
    if (distribution == distribution_type::normal)
        return parameters[0];

    return 0;
}

bool Distribution::operator==(const string &dist_type)
{
    if (distribution==distribution_type::normal && dist_type == "normal")
        return true;
    if (distribution==distribution_type::lognormal && dist_type == "log-normal")
        return true;
    if (distribution==distribution_type::uniform && dist_type == "uniform")
        return true;
    if (distribution==distribution_type::dirichlet && dist_type == "uniform")
        return true;
    if (distribution==distribution_type::dirichlet && dist_type == "dirichlet")
        return true;

    return false;

}

