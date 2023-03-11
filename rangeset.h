#ifndef RANGESET_H
#define RANGESET_H

#include "interface.h"
#include "range.h"

class RangeSet : public Interface, public map<string,Range>
{
public:
    RangeSet();
    RangeSet(const RangeSet &rhs);
    RangeSet& operator = (const RangeSet &rhs);
    QJsonObject toJsonObject() override;
    bool ReadFromJsonObject(const QJsonObject &jsonobject) override;
    string ToString() override;
    bool writetofile(QFile*) override;
    bool Read(const QStringList &strlist) override;
    double maxval();
    double minval();
};

#endif // RANGESET_H
