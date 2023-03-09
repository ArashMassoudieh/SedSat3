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

bool CMBTimeSeriesSet::writetofile(QFile* file)
{
    file->write(QString::fromStdString(ToString()).toUtf8());
    return true;
}

void CMBTimeSeriesSet::AppendLastContribution(int colnumber, const string &name)
{
    CMBTimeSeriesSet out;
    CTimeSeries<double> last_contribution;
    for (int j=0; j<maxnumpoints(); j++)
    {
        double sum = 0;
        for (int i=0; i<colnumber; i++)
        {
            sum+=BTC[i].GetC(j);
        }
        last_contribution.append(j,1-sum);
    }
    for (int i = 0; i<colnumber; i++)
    {
        out.append(BTC[i],names[i]);
    }
    out.append(last_contribution,name);
    for (int i=colnumber; i<nvars; i++)
    {
        out.append(BTC[i],names[i]);
    }
    *this = out;

}
