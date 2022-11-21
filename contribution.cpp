#include "contribution.h"
#include "Utilities.h"

Contribution::Contribution():map<string, double>(),Interface()
{

}

Contribution::Contribution(const Contribution &rhs):map<string, double>(rhs),Interface()
{

}
Contribution& Contribution::operator = (const Contribution &rhs)
{
    map<string,double>::operator=(rhs);
    Interface::operator=(rhs);
    return *this;
}

string Contribution::ToString()
{
    string out;
    for (map<string,double>::const_iterator it=cbegin(); it!=cend(); it++ )
    {
        out += it->first + ":" + aquiutils::numbertostring(it->second) + "\n";
    }
    return out;
}
