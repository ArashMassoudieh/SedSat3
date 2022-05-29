#include "elemental_profile.h"
#include "iostream"

Elemental_Profile::Elemental_Profile()
{

}

Elemental_Profile::Elemental_Profile(const Elemental_Profile& mp)
{
    profile = mp.profile;
}

Elemental_Profile& Elemental_Profile::operator=(const Elemental_Profile &mp)
{
    profile = mp.profile;
    return *this;
}

double Elemental_Profile::Val(const string &name) const
{
    if (profile.count(name)==0)
    {
        cout<<"Element '" + name + "' does not exist!"<<endl;
        return -1;
    }
    else
        return profile.at(name);
}


bool Elemental_Profile::SetVal(const string &name, const double &val)
{
    if (profile.count(name)==0)
    {
        cout<<"Element '" + name + "' does not exist!"<<endl;
        return false;
    }
    else
    {   profile[name]=val;
        return true;
    }
}

bool Elemental_Profile::AppendElement(const string &name,const double &val)
{
    if (profile.count(name)==0)
    {
        profile[name]=val;
        return true;
    }
    else
    {   cout<<"Element '" + name + "' already exists";
        return false;
    }
}

vector<double> Elemental_Profile::Vals()
{
    vector<double> vals;
    for (map<string,double>::iterator it=profile.begin(); it!=profile.end(); it++)
        vals.push_back(it->second);

    return vals;
}
