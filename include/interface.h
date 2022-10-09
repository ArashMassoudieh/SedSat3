#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>

using namespace std;

class Interface
{
public:
    Interface();
    Interface(const Interface &intf);
    Interface& operator=(const Interface &intf);
    virtual string ToString();

};

#endif // INTERFACE_H
