#include "elemental_profile.h"
#include "iostream"

using namespace std;

Elemental_Profile::Elemental_Profile()
{

}

Elemental_Profile::Elemental_Profile(const Elemental_Profile& mp):map<string,double>(mp)
{
    
}

Elemental_Profile& Elemental_Profile::operator=(const Elemental_Profile &mp)
{
    map<string,double>::operator=(mp);
    return *this;
}



double Elemental_Profile::Val(const string &name) const
{
    if (count(name)==0)
    {
        cout<<"Element '" + name + "' does not exist!"<<endl;
        return -1;
    }
    else
        return at(name);
}


bool Elemental_Profile::SetVal(const string &name, const double &val)
{
    if (count(name)==0)
    {
        cout<<"Element '" + name + "' does not exist!"<<endl;
        return false;
    }
    else
    {   operator[](name)=val;
        return true;
    }
}

bool Elemental_Profile::AppendElement(const string &name,const double &val)
{
    if (count(name)==0)
    {
        operator[](name) =val;
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
    for (map<string,double>::iterator it=begin(); it!=end(); it++)
        vals.push_back(it->second);

    return vals;
}
