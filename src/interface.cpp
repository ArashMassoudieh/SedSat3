#include "interface.h"

Interface::Interface()
{

}

Interface::Interface(const Interface &intf)
{

}
Interface& Interface::operator=(const Interface &intf)
{
    return *this;
}

string Interface::ToString()
{
    return "";
}

bool Interface::writetofile(QFile* file)
{
    return false;
}
