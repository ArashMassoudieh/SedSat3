#ifndef TESTMCMC_H
#define TESTMCMC_H

#include <vector>
#include "parameter.h"

using namespace std;

class TestMCMC
{
public:
    TestMCMC();
    void InitializeParametersObservations(const vector<double> &mins, const vector<double> &maxs);
    vector<Parameter> &Parameters() {return parameters;}
    bool SetParameterValue(unsigned int i, double value); //set the parameter values for estimation
    double GetObjectiveFunctionValue();
    CVector GetPredictedValues(); // copied the predicted constituents and isotopes from observations into a vector
    size_t ObservationsCount() {return 0; }

private:
    vector<Parameter> parameters;
};

#endif // TESTMCMC_H
