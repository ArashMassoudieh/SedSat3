#ifndef RANGE_H
#define RANGE_H

#include <interface.h>
#include "parameter.h"

class Range : public Interface
{
public:
    Range();
    Range(const Range &rhs);
    Range& operator = (const Range &rhs);
    QJsonObject toJsonObject() override;
    bool ReadFromJsonObject(const QJsonObject &jsonobject) override;
    string ToString() override;
    bool writetofile(QFile*) override;
    bool Read(const QStringList &strlist) override;
    void Set(_range lowhigh,const double &value);
    double Get(_range lowhigh);
    void SetValue(const double value) {observed_value = value;}
    double GetValue() {return observed_value; }
private:
    vector<double> range;
    double observed_value=0;
};

#endif // RANGE_H
