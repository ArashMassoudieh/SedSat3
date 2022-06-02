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
    Elemental_Profile_Set* Append_Source(const string &name, const Elemental_Profile_Set &elemental_profile_set=Elemental_Profile_Set());
    Elemental_Profile_Set* Append_Target(const string &name, const Elemental_Profile_Set &elemental_profile_set=Elemental_Profile_Set());
    Elemental_Profile_Set *source(const string &name);
    Elemental_Profile_Set *target(const string &name);
    Elemental_Profile_Set *sample_set(const string &name);
    vector<string> GroupNames();
    vector<string> ElementNames();
    vector<string> SampleNames(const string groupname);
    profiles_data ExtractData(const vector<vector<string>> &indicators);
    element_data ExtractElementData(const string &element, const string &group);
    map<string,vector<double>> ExtractElementData(const string &element);
private:
    map<string,Elemental_Profile_Set> sources;
    map<string,Elemental_Profile_Set> targets;
};

#endif // SOURCESINKDATA_H
