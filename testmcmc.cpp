#include "testmcmc.h"

TestMCMC::TestMCMC()
{

}

void TestMCMC::InitializeParametersObservations(const vector<double> &mins, const vector<double> &maxs)
{
    parameters.resize(mins.size());
    for (int i=0; i<parameters.size(); i++)
    {   parameters[i].SetPriorDistribution(distribution_type::lognormal);
        parameters[i].SetRange(mins[i],maxs[i]);
        parameters[i].SetName("Parameter_" + aquiutils::numbertostring(i+1));
    }
}

bool TestMCMC::SetParameterValue(unsigned int i, double value)
{
    parameters[i].SetValue(value);
    return true;
}

double TestMCMC::GetObjectiveFunctionValue()
{
    return 0;
}
