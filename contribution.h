#ifndef CONTRIBUTION_H
#define CONTRIBUTION_H

#include <string>
#include <map>
#include "interface.h"

using namespace std;

class QFile; 

class Contribution: public map<string,double>, public Interface
{
public:
    Contribution();
    Contribution(const Contribution &rhs);
    Contribution& operator = (const Contribution &rhs);
    QJsonObject toJsonObject() override;
    bool ReadFromJsonObject(const QJsonObject &jsonobject) override;
    string ToString() override;
    bool writetofile(QFile*) override;
    bool Read(const QStringList &strlist) override;
};

#endif // CONTRIBUTION_H
