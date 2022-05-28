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
    Elemental_Profile_Set(const Elemental_Profile_Set& mp);
    Elemental_Profile_Set& operator=(const Elemental_Profile_Set &mp);
    Elemental_Profile *Profile(const string &name);
private:
    map<string,Elemental_Profile> elemental_profiles;
};

#endif // ELEMENTAL_PROFILE_SET_H
