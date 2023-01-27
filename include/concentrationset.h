#ifndef CONCENTRATIONSET_H
#define CONCENTRATIONSET_H

#include "vector"
#include "cmbdistribution.h"
#include "cmbtimeseries.h"


using namespace std;



class ConcentrationSet : public vector<double>
{
public:
    ConcentrationSet();
    ConcentrationSet(const ConcentrationSet& cs);
    ConcentrationSet(int n);
    ConcentrationSet& operator=(const ConcentrationSet &cs);
    double mean();
    double stdev();
    double stdevln();
    double meanln();
    double norm(const double &x);
    double normln(const double &x);
    double min();
    double max();
    ConcentrationSet ln();
    void Append(const double &x);
    void Append(const ConcentrationSet &x);
    void Append(ConcentrationSet *x);
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
private:
    Distribution FittedDist; // Distribution fitted based on the sample data
    Distribution EstimatedDistribution; //Estimated distribution based on inverse modeling
};

#endif // CONCENTRATIONSET_H
