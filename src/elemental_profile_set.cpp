#include "elemental_profile_set.h"
#include "iostream"

using namespace std;

Elemental_Profile_Set::Elemental_Profile_Set()
{

}

Elemental_Profile_Set::Elemental_Profile_Set(const Elemental_Profile_Set& mp)
{
    elemental_profiles = mp.elemental_profiles;
}

Elemental_Profile_Set& Elemental_Profile_Set::operator=(const Elemental_Profile_Set &mp)
{
    elemental_profiles = mp.elemental_profiles;
    return *this;
}

Elemental_Profile *Elemental_Profile_Set::Append_Profile(const string &name, const Elemental_Profile &profile)
{
    if (elemental_profiles.count(name)>0)
    {
        cout<<"Profile '" + name + "' already exists!"<<endl;
        return nullptr;
    }
    else
    {
        elemental_profiles[name] = profile;
    }
    for (map<string,double>::const_iterator it=profile.begin(); it!=profile.end(); it++)
    {
        element_distributions[it->first].push_back(it->second);
    }
    return &elemental_profiles[name];
}

vector<string> Elemental_Profile_Set::SampleNames()
{
    vector<string> out;
    for (map<string,Elemental_Profile>::iterator it=elemental_profiles.begin(); it!=elemental_profiles.end(); it++)
    {
        out.push_back(it->first);
    }
    return out;
}

Elemental_Profile *Elemental_Profile_Set::Profile(const string &name)
{
    if (elemental_profiles.count(name)==0)
    {
        cout<<"Sample '" + name + "' does not exist!"<<endl;
        return nullptr;
    }
    else
        return &elemental_profiles[name];

}

Elemental_Profile *Elemental_Profile_Set::Profile(unsigned int i)
{
    if (i>=elemental_profiles.size())
        return nullptr;
    else
    {
        map<string,Elemental_Profile>::iterator it=elemental_profiles.begin();
        for (int ii=0; ii<i; ii++)
            it++;
        return &it->second;
    }

}

Elemental_Profile Elemental_Profile_Set::Profile(const string &name) const
{
    if (elemental_profiles.count(name)==0)
    {
        cout<<"Sample '" + name + "' does not exist!"<<endl;
        return Elemental_Profile();
    }
    else
        return elemental_profiles.at(name);

}

Elemental_Profile Elemental_Profile_Set::Profile(unsigned int i) const
{
    if (i>=elemental_profiles.size())
        return Elemental_Profile();
    else
    {
        map<string,Elemental_Profile>::const_iterator it=elemental_profiles.begin();
        for (int ii=0; ii<i; ii++)
            it++;
        return it->second;
    }

}

vector<double> Elemental_Profile_Set::GetProfileForSample(const string &sample_name)
{
    if (!Profile(sample_name))
        return vector<double>();

    return Profile(sample_name)->Vals();
}
