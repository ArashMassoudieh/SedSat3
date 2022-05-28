#ifndef SOURCESINKDATA_H
#define SOURCESINKDATA_H

#include <elemental_profile_set.h>

class SourceSinkData
{
public:
    SourceSinkData();
    SourceSinkData(const SourceSinkData& mp);
    SourceSinkData& operator=(const SourceSinkData &mp);
    void Append_Source(const string &name, const Elemental_Profile_Set &elemental_profile_set);
    void Append_Target(const string &name, const Elemental_Profile_Set &elemental_profile_set);
    Elemental_Profile_Set *source(const string &name);
    Elemental_Profile_Set *target(const string &name);
private:
    map<string,Elemental_Profile_Set> sources;
    map<string,Elemental_Profile_Set> targets;
};

#endif // SOURCESINKDATA_H
