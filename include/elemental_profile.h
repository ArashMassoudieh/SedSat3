#ifndef ELEMENTAL_PROFILE_H
#define ELEMENTAL_PROFILE_H

#include <map>
#include <string>

using namespace std;
class Elemental_Profile
{
public:
    Elemental_Profile();
    Elemental_Profile(const Elemental_Profile& mp);
    Elemental_Profile& operator=(const Elemental_Profile &mp);
    double Val(const string &name) const;
    bool SetVal(const string &name, const double &val);
    bool AppendElement(const string &name,const double &val=0);
    map<string,double>::iterator begin() {return profile.begin(); }
    map<string,double>::iterator end() {return profile.end(); }
private:
    map<string,double> profile;
};

#endif // ELEMENTAL_PROFILE_H
