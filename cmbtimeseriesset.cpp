#include "cmbtimeseriesset.h"
#include "QJsonArray"
#include "Vector.h"

CMBTimeSeriesSet::CMBTimeSeriesSet():CTimeSeriesSet<double>(),Interface()
{

}

CMBTimeSeriesSet::CMBTimeSeriesSet(const CMBTimeSeriesSet& mp):CTimeSeriesSet<double>(mp)
{

}

CMBTimeSeriesSet::CMBTimeSeriesSet(int n):CTimeSeriesSet<double>(n), Interface()
{

}

CMBTimeSeriesSet& CMBTimeSeriesSet::operator=(const CMBTimeSeriesSet &mp)
{
    CTimeSeriesSet<double>::operator=(mp);
    return *this;
}

CMBTimeSeriesSet& CMBTimeSeriesSet::operator=(const CTimeSeriesSet<double> &mp)
{
    CTimeSeriesSet<double>::operator=(mp);
    return *this;
}
CMBTimeSeriesSet::CMBTimeSeriesSet(const CTimeSeriesSet<double>& mp):CTimeSeriesSet<double>(mp)
{

}

QJsonObject CMBTimeSeriesSet::toJsonObject()
{
    QJsonObject out;

    for (int i=0; i<0; i<nvars)
    {
        QJsonObject timeseries;
        QJsonArray t_values;
        QJsonArray C_values;
        for (int i=0; i<BTC[i].n; i++)
        {
            t_values.append(BTC[i].GetT(i));
            C_values.append(BTC[i].GetC(i));
        }
        timeseries["time"] = t_values;
        timeseries["value"] = C_values;
        out[QString::fromStdString(names[i])] = timeseries;
    }

    return out;
}
bool CMBTimeSeriesSet::ReadFromJsonObject(const QJsonObject &jsonobject)
{
    return true;
}


string CMBTimeSeriesSet::ToString()
{

    string out;
    for (unsigned int i=0; i<names.size(); i++)
        out += "\t" + names[i];
    out += "\n";
    for (int j=0; j<maxnumpoints(); j++)
    {
        for (int i=0; i<nvars; i++)
        {
            {
                if (i>0)
                    out += ", ";
                if (j<BTC[i].n)
                    out+= QString::number(BTC[i].GetT(j)).toStdString() + "," + QString::number(BTC[i].GetC(j)).toStdString();
                else
                    out+= ", ";
            }
        }

        out += "\n";
    }
    return out;
}

