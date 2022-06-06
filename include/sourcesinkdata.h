#ifndef SOURCESINKDATA_H
#define SOURCESINKDATA_H

#include <elemental_profile_set.h>
#include "vector"

struct profiles_data
{
    vector<string> element_names;
    vector<string> sample_names;
    vector<vector<double>> values;
};

struct element_data
{
    string group_name;
    vector<string> sample_names;
    vector<double> values;
};

struct element_data_groups
{
    vector<string> group_names;
    vector<string> sample_names;
    vector<vector<double>> values;
};


class SourceSinkData
{
public:
    SourceSinkData();
    SourceSinkData(const SourceSinkData& mp);
    SourceSinkData& operator=(const SourceSinkData &mp);
    Elemental_Profile_Set* AppendSampleSet(const string &name, const Elemental_Profile_Set &elemental_profile_set=Elemental_Profile_Set());
    Elemental_Profile_Set *sample_set(const string &name);
    vector<string> GroupNames();
    vector<string> ElementNames();
    vector<string> SampleNames(const string groupname);
    profiles_data ExtractData(const vector<vector<string>> &indicators);
    element_data ExtractElementData(const string &element, const string &group);
    map<string,vector<double>> ExtractElementData(const string &element);
    void PopulateElementDistributions();
    void AssignAllDistributions();
    Distribution *FittedDistribution(const string &element_name);
    map<string,Elemental_Profile_Set>::iterator begin() {return sample_sets.begin();}
    map<string,Elemental_Profile_Set>::iterator end() {return sample_sets.end();}
    map<string,Elemental_Profile_Set>::const_iterator cbegin() {return sample_sets.begin();}
    map<string,Elemental_Profile_Set>::const_iterator cend() {return sample_sets.end();}
private:
    map<string,Elemental_Profile_Set> sample_sets;
    map<string,ConcentrationSet> element_distributions;
};

#endif // SOURCESINKDATA_H
