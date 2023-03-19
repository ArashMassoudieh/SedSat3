#ifndef CMBTimeSeries_H
#define CMBTimeSeries_H

#include "BTC.h"
#include "interface.h"

class CMBTimeSeries : public CTimeSeries<double>, public Interface
{
public:
    CMBTimeSeries();
    CMBTimeSeries(int n);
    CMBTimeSeries(const CMBTimeSeries& mp);
    CMBTimeSeries& operator=(const CMBTimeSeries &mp);
    QJsonObject toJsonObject() override;
    bool ReadFromJsonObject(const QJsonObject &jsonobject) override;
    string ToString() override;
    bool writetofile(QFile*) override;
    QTableWidget *ToTable() override;

private:

};

#endif // CMBTimeSeries_H
