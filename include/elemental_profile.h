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
    double Val(const string &name) const; // returns the value of elemental content for a specific element/constituent
    bool SetVal(const string &name, const double &val); //Sets the value of elemental content for a specific element/constituent
    bool AppendElement(const string &name,const double &val=0); //Append an element and it's value to the list of elements
    vector<double> Vals(); //Returns a vector containing the values of all emenents in the profile
private:
    
};

#endif // ELEMENTAL_PROFILE_H
