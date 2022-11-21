#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>

class QFile; 

using namespace std;

class Interface
{
public:
    Interface();
    Interface(const Interface &intf);
    Interface& operator=(const Interface &intf);
    virtual string ToString();
    virtual bool writetofile(QFile* file);

};

#endif // INTERFACE_H
