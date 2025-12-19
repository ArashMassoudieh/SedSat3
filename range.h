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
    double Get(_range lowhigh) const;
    double Mean() const {return mean;}
    void SetMean(const double &m) {mean = m;}
    double Median() const {return median;}
    void SetMedian(const double &m) {median = m;}
    void SetValue(const double value) {observed_value = value;}
    double GetValue() {return observed_value; }
private:
    vector<double> range;
    double median = 0;
    double mean = 0;
    double observed_value=0;
};

#endif // RANGE_H
