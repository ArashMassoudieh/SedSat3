#ifndef ELEMENTAL_PROFILE_SET_H
#define ELEMENTAL_PROFILE_SET_H

#include <elemental_profile.h>
#include <map>
#include <string>
#include <vector>

using namespace std;

class Elemental_Profile_Set
{
public:
    Elemental_Profile_Set();
    Elemental_Profile_Set(const Elemental_Profile_Set& mp);
    Elemental_Profile_Set& operator=(const Elemental_Profile_Set &mp);
    Elemental_Profile *Profile(const string &name);
    vector<double> GetAllConcentrationsFor(const string &element_name);
    vector<double> GetProfileForSample(const string &source_name);
    Elemental_Profile *Append_Profile(const string &name, const Elemental_Profile &profile=Elemental_Profile());
    map<string,Elemental_Profile>::iterator begin() {return elemental_profiles.begin(); }
    map<string,Elemental_Profile>::iterator end() {return elemental_profiles.end(); }
    vector<string> SampleNames();
private:
    map<string,Elemental_Profile> elemental_profiles;
};

#endif // ELEMENTAL_PROFILE_SET_H
