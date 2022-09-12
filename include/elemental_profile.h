#ifndef ELEMENTAL_PROFILE_H
#define ELEMENTAL_PROFILE_H

#include <map>
#include <string>
#include <vector>

using namespace std;
class Elemental_Profile : public map<string,double>
{
public:
    Elemental_Profile();
    Elemental_Profile(const Elemental_Profile& mp);
    Elemental_Profile& operator=(const Elemental_Profile &mp);
    double Val(const string &name) const;
    bool SetVal(const string &name, const double &val);
    bool AppendElement(const string &name,const double &val=0);
    vector<double> Vals();
private:
    
};

#endif // ELEMENTAL_PROFILE_H
