#ifndef ELEMENTAL_PROFILE_SET_H
#define ELEMENTAL_PROFILE_SET_H

#include <elemental_profile.h>
#include <map>
#include <string>
#include <vector>
#include "concentrationset.h"

using namespace std;



class Elemental_Profile_Set
{
public:
    Elemental_Profile_Set();
    Elemental_Profile_Set(const Elemental_Profile_Set& mp);
    Elemental_Profile_Set& operator=(const Elemental_Profile_Set &mp);
    Elemental_Profile *Profile(const string &name);
    Elemental_Profile Profile(const string &name) const;
    Elemental_Profile *Profile(unsigned int i);
    Elemental_Profile Profile(unsigned int i) const;
    vector<double> GetAllConcentrationsFor(const string &element_name);
    vector<double> GetProfileForSample(const string &source_name);
    Elemental_Profile *Append_Profile(const string &name, const Elemental_Profile &profile=Elemental_Profile());
    map<string,Elemental_Profile>::iterator begin() {return elemental_profiles.begin(); }
    map<string,Elemental_Profile>::iterator end() {return elemental_profiles.end(); }
    vector<string> SampleNames();
    ConcentrationSet *ElementalDistribution(const string &element_name)
    {
        return &element_distributions[element_name];
    }
    ConcentrationSet ElementalDistribution(const string &element_name) const {
        return element_distributions.at(element_name);
    }
    distribution_type DistributionAssigned(const string &element_name)
    {
        if (element_distributions.count(element_name)==0)
            return distribution_type::none;

        return element_distributions[element_name].FittedDistribution()->distribution;
    }
    double Estimated_mu(const string &element)
    {
        if (element_distributions.count(element)>0)
            return element_distributions[element].EstimatedMu();
        else
            return 0;
    }
    double Estimated_sigma(const string &element)
    {
        if (element_distributions.count(element)>0)
            return element_distributions[element].EstimatedSigma();
        else
            return 0;
    }
    bool Set_Estimated_mu(const string &element, const double &value)
    {
        if (element_distributions.count(element)>0)
        {   element_distributions[element].SetEstimatedMu(value);
            return true;
        }
        else
            return false;

    }
    bool Set_Estimated_sigma(const string &element, const double &value)
    {
        if (element_distributions.count(element)>0)
        {   element_distributions[element].SetEstimatedSigma(value);
            return true;
        }
        else
            return false;

    }
    bool SetContribution(const double &x)
    {
        contribution = x;
        return true;
    }
    double Contribution() {return contribution; }
    Distribution* GetEstimatedDistribution(const string &element_name )
    {
        if (element_distributions.count(element_name)>0)
            return element_distributions[element_name].GetEstimatedDistribution();
        else
            return nullptr;

    }
private:
    map<string,Elemental_Profile> elemental_profiles;
    map<string,ConcentrationSet> element_distributions;
    double contribution = 0;

};

#endif // ELEMENTAL_PROFILE_SET_H
