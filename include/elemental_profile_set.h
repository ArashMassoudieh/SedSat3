#ifndef ELEMENTAL_PROFILE_SET_H
#define ELEMENTAL_PROFILE_SET_H

#include <elemental_profile.h>
#include <map>
#include <string>

using namespace std;

class Elemental_Profile_Set
{
public:
    Elemental_Profile_Set();

private:
    map<string,Elemental_Profile> elemental_profiles;
};

#endif // ELEMENTAL_PROFILE_SET_H
