#include "sourcesinkdata.h"
#include "iostream"

SourceSinkData::SourceSinkData()
{

}

SourceSinkData::SourceSinkData(const SourceSinkData& mp)
{
    sample_sets = mp.sample_sets;
}
SourceSinkData& SourceSinkData::operator=(const SourceSinkData &mp)
{
    sample_sets = mp.sample_sets;
    return *this;
}

Elemental_Profile_Set* SourceSinkData::AppendSampleSet(const string &name, const Elemental_Profile_Set &elemental_profile_set)
{
    if (sample_sets.count(name)==0)
        sample_sets[name] = elemental_profile_set;
    else
    {   cout<<"Sample set type '" + name + "' already exists!"<<endl;
        return nullptr;
    }

    return &sample_sets[name];
}

Elemental_Profile_Set *SourceSinkData::sample_set(const string &name)
{
    if (sample_sets.count(name)!=0)
    {
        return sample_set(name);
    }
    return nullptr;
}

vector<string> SourceSinkData::SampleNames(const string groupname)
{
    if (sample_set(groupname))
    {
        return sample_set(groupname)->SampleNames();
    }
    return vector<string>();
}

vector<string> SourceSinkData::GroupNames()
{
    vector<string> out;
    for (map<string,Elemental_Profile_Set>::iterator it=sample_sets.begin(); it!=sample_sets.end(); it++)
    {
        out.push_back(it->first);
    }
    return out;
}
vector<string> SourceSinkData::ElementNames()
{
    vector<string> out;

    for (map<string,double>::iterator it=sample_sets.begin()->second.begin()->second.begin(); it!=sample_sets.begin()->second.begin()->second.end(); it++)
    {
        out.push_back(it->first);
    }
    return out;
}

profiles_data SourceSinkData::ExtractData(const vector<vector<string>> &indicators)
{
    profiles_data extracted_data;
    extracted_data.element_names = ElementNames();
    for (int i=0; i<indicators.size(); i++)
    {
        vector<double> vals = sample_set(indicators[i][0])->GetProfileForSample(indicators[i][1]);
        extracted_data.values.push_back(vals);
        extracted_data.sample_names.push_back(indicators[i][1]);
    }
    return extracted_data;
}

void SourceSinkData::PopulateElementDistributions()
{
    vector<string> element_names = ElementNames();
    for (map<string,Elemental_Profile_Set>::iterator it=sample_sets.begin(); it!=sample_sets.end(); it++)
    {
        for (unsigned int i=0; i<element_names.size(); i++)
        {
            element_distributions[element_names[i]].Append(it->second.ElementalDistribution(element_names[i]));
        }
    }
}

void SourceSinkData::AssignAllDistributions()
{
    vector<string> element_names = ElementNames();
    for (unsigned int i=0; i<element_names.size(); i++)
    {
        element_distributions[element_names[i]].FittedDistribution()->distribution = element_distributions[element_names[i]].SelectBestDistribution();
        element_distributions[element_names[i]].FittedDistribution()->parameters = element_distributions[element_names[i]].EstimateParameters();
    }
}


