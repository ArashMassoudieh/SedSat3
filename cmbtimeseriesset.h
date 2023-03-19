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
    QTableWidget *ToTable() override;
    void AppendLastContribution(int colnumber,const string &name);
    void SetObservedValue(int i, const double &value)
    {
        if (i<nvars)
            observed_value[i] = value;
    }
    double ObservedValue(int i)
    {
        if (i<nvars)
            return observed_value[i];
        else
            return 0;
    }
    double ObservedValue(string variable_name)
    {
        for (int i=0; i<nvars; i++)
        {
            if (names[i]==variable_name)
                return observed_value[i];
        }
        return 0;
    }
private:
    vector<double> observed_value;
};

#endif // CMBTimeSeriesSet_H
