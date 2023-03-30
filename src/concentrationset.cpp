#include "concentrationset.h"
#include "math.h"
#include "Utilities.h"
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_randist.h>

ConcentrationSet::ConcentrationSet():vector<double>()
{
    
}

ConcentrationSet::ConcentrationSet(const ConcentrationSet &cs):vector<double>(cs)
{
    EstimatedDistribution = cs.EstimatedDistribution;
}

ConcentrationSet::ConcentrationSet(int n):vector<double>(n)
{

}

ConcentrationSet& ConcentrationSet::operator=(const ConcentrationSet &cs)
{
    vector<double>::operator=(cs);
    EstimatedDistribution = cs.EstimatedDistribution;
    return *this;
}

double ConcentrationSet::mean()
{
    double sum=0;
    for (unsigned int i=0; i<size(); i++)
    {
        sum += at(i);
    }
    return sum/double(size());
}
double ConcentrationSet::stdev()
{
    double mean_value = mean();
    double sum=0;
    for (unsigned int i=0; i<size(); i++)
        sum+= pow(at(i)-mean_value,2);

    return sqrt(sum/double(size()));
}

double ConcentrationSet::stdevln()
{
    double mean_value = meanln();
    double sum=0;
    for (unsigned int i=0; i<size(); i++)
        sum+= pow(log(at(i))-mean_value,2);

    return sqrt(sum/double(size()));
}

double ConcentrationSet::meanln()
{
    double sum=0;
    for (unsigned int i=0; i<size(); i++)
    {
        sum += log(at(i));
    }
    return sum/double(size());
}
double ConcentrationSet::norm(const double &x)
{
    double sum=0;
    for (unsigned int i=0; i<size(); i++)
    {
        sum += pow(at(i),x);
    }
    return sum;
}
double ConcentrationSet::normln(const double &x)
{
    double sum=0;
    for (unsigned int i=0; i<size(); i++)
    {
        sum += pow(log(at(i)),x);
    }
    return sum;
}
ConcentrationSet ConcentrationSet::ln()
{
    ConcentrationSet out(size());;
    for (unsigned int i=0; i<size(); i++)
    {
        out[i] = log(at(i));
    }
    return out;
}

double ConcentrationSet::min()
{
    return aquiutils::Min(*this);
}
double ConcentrationSet::max()
{
    return aquiutils::Max(*this);
}

void ConcentrationSet::Append(const double &x)
{
    push_back(x);
}
void ConcentrationSet::Append(const ConcentrationSet &x)
{
    insert( end(), x.begin(), x.end());
}

void ConcentrationSet::Append(ConcentrationSet *x)
{
    insert( end(), x->begin(), x->end());
}

vector<double> ConcentrationSet::EstimateParameters(distribution_type disttype)
{
    vector<double> out;
    if (disttype==distribution_type::none)
        disttype = FittedDist.distribution;
    FittedDist.SetDataMean(mean());
    FittedDist.SetDataSTDev(stdev());
    if (disttype==distribution_type::normal)
    {
        out.push_back(mean());
        out.push_back(stdev());
    }
    else if (disttype==distribution_type::lognormal)
    {
        out.push_back(meanln());
        out.push_back(stdevln());
    }
    return out;
}

double ConcentrationSet::LogLikelihood(const vector<double> &params,distribution_type disttype)
{
    double loglikelihood=0;
    if (params.size()==0 && disttype==distribution_type::none)
    {
        for (unsigned int i=0; i<size(); i++)
            loglikelihood += log(FittedDist.Eval(at(i)));
    }
    if (params.size()!=0 && disttype!=distribution_type::none)
    {
        Distribution dist;
        dist.parameters = params;
        dist.distribution = disttype;
        for (unsigned int i=0; i<size(); i++)
            loglikelihood += log(dist.Eval(at(i)));
    }
    return loglikelihood;
}

distribution_type ConcentrationSet::SelectBestDistribution()
{
    if (this->min()<=0)
    {
        return distribution_type::normal;
    }
    double loglikelihood_normal = 0;
    double loglikelihood_lognormal = 0;

    vector<double> parameters_normal = EstimateParameters(distribution_type::normal);
    loglikelihood_normal = LogLikelihood(parameters_normal,distribution_type::normal);

    vector<double> parameters_lognormal = EstimateParameters(distribution_type::lognormal);
    loglikelihood_normal = LogLikelihood(parameters_normal,distribution_type::lognormal);

    if (loglikelihood_normal>loglikelihood_lognormal) return distribution_type::normal; else return distribution_type::lognormal;
}

CMBTimeSeries ConcentrationSet::DataCDF()
{

    vector<double> data = QSort(*this);
    CMBTimeSeries out;
    double dx = 1.0/double(data.size());

    for (unsigned int i=0; i<data.size(); i++)
    {
        out.append(data[i],(double(i)+0.5)*dx);
    }
    return out;


}

CMBTimeSeriesSet ConcentrationSet::DataCDFnFitted(distribution_type dist_type)
{
    CMBTimeSeriesSet out; 
    out.append(DataCDF(), "Observed");
    CMBTimeSeries fitted; 
    for (double x = out[0].mint(); x <= out[0].maxt(); x += (out[0].maxt() - out[0].mint()) / 50.0)
    {
        if (dist_type == distribution_type::normal)
        {
            fitted.append(x, gsl_cdf_gaussian_P(x - mean(), stdev()));
        }
        else if(dist_type == distribution_type::lognormal)
        {
            if (x>0)
                fitted.append(x, gsl_cdf_lognormal_P(x, meanln(), stdevln()));
            else
                fitted.append(x, 0);
        }
    }
    out.append(fitted, "Fitted");
    out.append(out[0] - out[1],"Error");
    return out; 
}

CMBTimeSeriesSet ConcentrationSet::DistFitted(distribution_type dist_type)
{
    CMBTimeSeriesSet out;
    out.append(DataCDF(), "Observed");
    CMBTimeSeries fitted;
    vector<double> parameters;
    if (dist_type == distribution_type::normal)
    {
        parameters.push_back(mean());
        parameters.push_back(stdev());
    }
    else if (dist_type == distribution_type::lognormal)
    {
        parameters.push_back(meanln());
        parameters.push_back(stdevln());
    }
    for (double x = out[0].mint(); x <= out[0].maxt(); x += (out[0].maxt() - out[0].mint()) / 50.0)
    {
        fitted.append(x, Distribution::Eval(x,parameters,dist_type));
    }

    out["Observed"] = fitted.maxC()/2;
    out.append(fitted, "Fitted");
    return out;
}

double ConcentrationSet::KolmogorovSmirnovStat(distribution_type dist_type)
{
    CMBTimeSeriesSet observed_fitted = DataCDFnFitted(dist_type);
    return std::max(fabs(observed_fitted[2].maxC()),fabs(observed_fitted[2].minC()));
}

ConcentrationSet ConcentrationSet::BoxCoxTransform(const double &lambda)
{
    ConcentrationSet transformed(size());
    if (min()<0)
        return transformed;

    for (unsigned int i=0; i<size(); i++)
    {
        if (lambda!=0)
            transformed[i] = (pow(at(i),lambda)-1.0)/lambda;
        else
            transformed[i] = log(at(i));
    }
    return transformed;
}

double ConcentrationSet::OptimalBoxCoxParam(const double &x_1,const double &x_2, int n_intervals)
{
    vector<double> vals;
    if (fabs(x_1-x_2)<1e-6)
        return (x_1+x_2)/2.0;
    double d_lambda = (x_2-x_1)/double(n_intervals);
    for (double lambda = x_1; lambda<=x_2; lambda+=d_lambda )
    {
        vals.push_back(BoxCoxTransform(lambda).KolmogorovSmirnovStat(distribution_type::normal));
    }
    return OptimalBoxCoxParam(x_1 + std::max(aquiutils::MinElement(vals)-1,0)*d_lambda,x_1+std::min(aquiutils::MinElement(vals)+1,int(vals.size()-1))*d_lambda,n_intervals);
}
