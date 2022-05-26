#ifndef ELEMENTAL_PROFILE_H
#define ELEMENTAL_PROFILE_H

#include <map>
#include <string>

using namespace std;
class Elemental_Profile
{
public:
    Elemental_Profile();

private:
    map<string,double> profile;
};

#endif // ELEMENTAL_PROFILE_H
