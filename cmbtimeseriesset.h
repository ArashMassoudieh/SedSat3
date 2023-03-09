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
    CMBTimeSeriesSet& operator=(const CTimeSeriesSet<double> &mp);
    CMBTimeSeriesSet(const CTimeSeriesSet<double>& mp);
    QJsonObject toJsonObject() override;
    bool ReadFromJsonObject(const QJsonObject &jsonobject) override;
    string ToString() override;
    bool writetofile(QFile*) override;
    void AppendLastContribution(int colnumber,const string &name);

private:

};

#endif // CMBTimeSeriesSet_H
