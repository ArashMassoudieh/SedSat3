#ifndef SOURCESINKDATA_H
#define SOURCESINKDATA_H

#include <elemental_profile_set.h>

class SourceSinkData
{
public:
    SourceSinkData();

private:
    map<string,Elemental_Profile_Set> sources;
    map<string,Elemental_Profile_Set> targets;
};

#endif // SOURCESINKDATA_H
