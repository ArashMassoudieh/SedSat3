#include "elemental_profile.h"
#include "iostream"
#include "Utilities.h"
#include <QFile>

using namespace std;

Elemental_Profile::Elemental_Profile() :map<string, double>(), Interface()
{

}

Elemental_Profile::Elemental_Profile(const Elemental_Profile& mp):map<string,double>(mp),Interface()
{
    
}

Elemental_Profile& Elemental_Profile::operator=(const Elemental_Profile &mp)
{
    Interface::operator=(mp);
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

string Elemental_Profile::ToString()
{
    string out;
    for (map<string,double>::iterator it=begin(); it!=end(); it++)
    {
        out += it->first + ":" + aquiutils::numbertostring(it->second) + "\n";
    }
    return out;
}

bool Elemental_Profile::writetofile(QFile* file)
{
    file->write(QString::fromStdString(ToString()).toUtf8());
    return true;
}

double Elemental_Profile::max()
{
    double _max = -1e12; 
    for (map<string, double>::iterator it = begin(); it != end(); it++)
    {
        if (it->second > _max) _max = it->second; 
    }
    return _max; 
}
double Elemental_Profile::min()
{
    double _min = 1e12;
    for (map<string, double>::iterator it = begin(); it != end(); it++)
    {
        if (it->second < _min) _min = it->second;
    }
    return _min;
}

vector<string> Elemental_Profile::ElementNames()
{
    vector<string> out;
    for (map<string, double>::iterator it = begin(); it != end(); it++)
    {
        out.push_back(it->first);
    }
    return out; 
}
