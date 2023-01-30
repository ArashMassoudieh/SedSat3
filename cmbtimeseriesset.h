#ifndef CMBTimeSeriesSet_H
#define CMBTimeSeriesSet_H

#include "BTCSet.h"
#include "interface.h"

class CMBTimeSeriesSet : public CTimeSeriesSet<double>, public Interface
{
public:
    CMBTimeSeriesSet();
    CMBTimeSeriesSet(int n);
    CMBTimeSeriesSet(int n, int m);
    CMBTimeSeriesSet(const CMBTimeSeriesSet& mp);
    CMBTimeSeriesSet& operator=(const CMBTimeSeriesSet &mp);
    QJsonObject toJsonObject() override;
    bool ReadFromJsonObject(const QJsonObject &jsonobject) override;
    string ToString() override;

private:

};

#endif // CMBTimeSeriesSet_H
