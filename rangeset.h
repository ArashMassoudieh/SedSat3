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
    QJsonObject toJsonObject() const override;
    bool ReadFromJsonObject(const QJsonObject &jsonobject) override;
    string ToString() const override;
    bool writetofile(QFile*) override;
    bool Read(const QStringList &strlist) override;
    QTableWidget *ToTable() override;
    double maxval();
    double minval();
};

#endif // RANGESET_H
