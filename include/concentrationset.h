#ifndef CONCENTRATIONSET_H
#define CONCENTRATIONSET_H

#include "vector"
#include "cmbdistribution.h"
#include "cmbtimeseries.h"
#include "cmbtimeseriesset.h"


//using namespace std;



class ConcentrationSet : public vector<double>
{
public:
    ConcentrationSet();
    ConcentrationSet(const ConcentrationSet& cs);
    ConcentrationSet(int n);
    ConcentrationSet& operator=(const ConcentrationSet &cs);
    double mean();
    double stdev(const double &mean=-999);
    double stdevln(const double &mean=-999);
    double SSE(const double &m = -999);
    double SSE_ln(const double &m = -999);
    double meanln();
    double norm(const double &x);
    double normln(const double &x);
    double min();
    double max();
    ConcentrationSet ln();
    void Append(const double &x);
    void Append(const ConcentrationSet &x);
    void Append(ConcentrationSet *x);
    vector<unsigned int> Rank();
    vector<double> EstimateParameters(distribution_type disttype=distribution_type::none);
    double LogLikelihood(const vector<double> &params=vector<double>(),distribution_type disttype=distribution_type::none);
    distribution_type SelectBestDistribution();
    Distribution* FittedDistribution() {
        return &FittedDist;
    }
    void SetEstimatedMu(const double &mu) {EstimatedDistribution.parameters[0] = mu;}
    void SetEstimatedSigma(const double &sigma) {EstimatedDistribution.parameters[1] = sigma;}
    void SetEstimatedDataMean(const double &value) {EstimatedDistribution.SetDataMean(value);}
    void SetEstimatedDataSTDev(const double &value) {EstimatedDistribution.SetDataSTDev(value);}
    double EstimatedMu() {return EstimatedDistribution.parameters[0];}
    double EstimatedSigma() {return EstimatedDistribution.parameters[1];}
    Distribution* GetEstimatedDistribution() {
        return &EstimatedDistribution;
    }
    CMBTimeSeries DataCDF();
    CMBTimeSeriesSet DataCDFnFitted(distribution_type dist_type);
    CMBTimeSeriesSet DistFitted(distribution_type dist_type);
    double KolmogorovSmirnovStat(distribution_type dist_type);
    double BoxCoxLogLikelihood(double lambda);
    ConcentrationSet BoxCoxTransform(const double &lambda, bool normalize);
    double OptimalBoxCoxParam(const double &x_1,const double &x_2, int n_intervals);
private:
    Distribution FittedDist; // Distribution fitted based on the sample data
    Distribution EstimatedDistribution; //Estimated distribution based on inverse modeling
};

#endif // CONCENTRATIONSET_H
