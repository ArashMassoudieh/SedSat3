#include "sourcesinkdata.h"
#include "iostream"

SourceSinkData::SourceSinkData()
{

}

SourceSinkData::SourceSinkData(const SourceSinkData& mp)
{
    sources = mp.sources;
    targets = mp.targets;
}
SourceSinkData& SourceSinkData::operator=(const SourceSinkData &mp)
{
    sources = mp.sources;
    targets = mp.targets;
    return *this;
}

Elemental_Profile_Set* SourceSinkData::Append_Source(const string &name, const Elemental_Profile_Set &elemental_profile_set)
{
    if (sources.count(name)==0)
        sources[name] = elemental_profile_set;
    else
    {   cout<<"Source type '" + name + "' already exists!"<<endl;
        return nullptr;
    }

    return &sources[name];
}

Elemental_Profile_Set* SourceSinkData::Append_Target(const string &name, const Elemental_Profile_Set &elemental_profile_set)
{
    if (targets.count(name)==0)
        targets[name] = elemental_profile_set;
    else
    {   cout<<"Target type '" + name + "' already exists!"<<endl;
        return nullptr;
    }

    return &targets[name];
}

Elemental_Profile_Set *SourceSinkData::source(const string &name)
{
    if (sources.count(name)==0)
        return nullptr;
    else
        return &sources[name];
}
Elemental_Profile_Set *SourceSinkData::target(const string &name)
{
    if (targets.count(name)==0)
        return nullptr;
    else
        return &targets[name];
}

Elemental_Profile_Set *SourceSinkData::sample_set(const string &name)
{
    if (targets.count(name)!=0)
    {
        return target(name);
    }
    if (sources.count(name)!=0)
    {
        return source(name);
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
    for (map<string,Elemental_Profile_Set>::iterator it=targets.begin(); it!=targets.end(); it++)
    {
        out.push_back(it->first);
    }
    for (map<string,Elemental_Profile_Set>::iterator it=sources.begin(); it!=sources.end(); it++)
    {
        out.push_back(it->first);
    }
    return out;
}
vector<string> SourceSinkData::ElementNames()
{
    vector<string> out;

    for (map<string,double>::iterator it=targets.begin()->second.begin()->second.begin(); it!=targets.begin()->second.begin()->second.end(); it++)
    {
        out.push_back(it->first);
    }
    return out;
}


