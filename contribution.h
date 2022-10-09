#ifndef CONTRIBUTION_H
#define CONTRIBUTION_H

#include <string>
#include <map>
#include "interface.h"

using namespace std;

class Contribution: public map<string,double>, public Interface
{
public:
    Contribution();
    Contribution(const Contribution &rhs);
    Contribution& operator = (const Contribution &rhs);
    string ToString() override;
};

#endif // CONTRIBUTION_H
