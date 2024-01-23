#include "cmbtimeseriesset.h"
#include "QJsonArray"
#include "Vector.h"

CMBTimeSeriesSet::CMBTimeSeriesSet():CTimeSeriesSet<double>(),Interface()
{

}

CMBTimeSeriesSet::CMBTimeSeriesSet(const CMBTimeSeriesSet& mp):CTimeSeriesSet<double>(mp)
{
     observed_value = mp.observed_value;
     labels = mp.labels;
}

CMBTimeSeriesSet::CMBTimeSeriesSet(int n):CTimeSeriesSet<double>(n), Interface()
{
    observed_value.resize(n);
}

CMBTimeSeriesSet& CMBTimeSeriesSet::operator=(const CMBTimeSeriesSet &mp)
{
    CTimeSeriesSet<double>::operator=(mp);
    observed_value = mp.observed_value;
    labels = mp.labels;
    return *this;
}

CMBTimeSeriesSet& CMBTimeSeriesSet::operator=(const CTimeSeriesSet<double> &mp)
{
    CTimeSeriesSet<double>::operator=(mp);
    observed_value.resize(nvars);
    return *this;
}
CMBTimeSeriesSet::CMBTimeSeriesSet(const CTimeSeriesSet<double>& mp):CTimeSeriesSet<double>(mp)
{
    observed_value.resize(nvars);
}

QJsonObject CMBTimeSeriesSet::toJsonObject()
{
    QJsonObject out;
    QJsonArray labels;
    {
        for (int j=0; j<maxnumpoints(); j++)
            labels.append(QString::fromStdString(Label(j)));
    }
    out["labels"] = labels;
    for (int i=0; i<nvars; i++)
    {
        QJsonObject timeseries;
        QJsonArray t_values;
        QJsonArray C_values;
        for (int j=0; j<BTC[i].n; j++)
        {
            t_values.append(BTC[i].GetT(j));
            C_values.append(BTC[i].GetC(j));

        }
        timeseries["time"] = t_values;
        timeseries["value"] = C_values;
        out[QString::fromStdString(names[i])] = timeseries;
    }
    QJsonArray Jobserved_values;
    for (unsigned int i=0; i<observed_value.size(); i++)
        Jobserved_values.append(observed_value[i]);
    out["Observed Values"] = Jobserved_values;
    return out;
}
bool CMBTimeSeriesSet::ReadFromJsonObject(const QJsonObject &jsonobject)
{
    clear();
    observed_value.clear();
    for (QString key: jsonobject.keys())
    {
        if (key=="Observed Values")
        {
            QJsonArray observed_value_JArray = jsonobject[key].toArray();
            for (int i=0; i<observed_value_JArray.count(); i++)
                observed_value.push_back(observed_value_JArray[i].toDouble());
        }
        else if (key=="labels")
        {
            QJsonArray labels_JArray = jsonobject[key].toArray();
            for (int i=0; i<labels_JArray.count(); i++)
                labels.push_back(labels_JArray[i].toString().toStdString());
        }
        else
        {
            QString SeriesName = key;
            QJsonArray TimeJArray = jsonobject[key].toObject()["time"].toArray();
            QJsonArray ValueJArray = jsonobject[key].toObject()["value"].toArray();
            CTimeSeries<double> this_series;
            for (unsigned int i=0; i<TimeJArray.count(); i++)
                this_series.append(TimeJArray[i].toDouble(), ValueJArray[i].toDouble());
            append(this_series,SeriesName.toStdString());
        }
    }
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
                    out+= Label(j,i) + "," + QString::number(BTC[i].GetC(j)).toStdString();
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
        if (1.0-sum<0)
        {
            cout<<"Negative contribution!";
        }
        last_contribution.append(j,1.0-sum);
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

QTableWidget *CMBTimeSeriesSet::ToTable()
{
    QTableWidget *tablewidget = new QTableWidget();
    tablewidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    if (Interface::Option(options_key::single_column_x))
    {
        tablewidget->setColumnCount(nvars);
    }
    else
    {
        tablewidget->setColumnCount(nvars*2);
    }
    tablewidget->setRowCount(maxnumpoints());
    QStringList headers;
    QStringList rowlabels;

    for (int j=0; j<nvars; j++)
    {
        if (Interface::Option(options_key::single_column_x))
            headers << QString::fromStdString(names[j]) + GetOptions().Y_suffix;
        else
            headers << QString::fromStdString(names[j]) + GetOptions().X_suffix << QString::fromStdString(names[j]) + GetOptions().Y_suffix;
        for (int i=0; i<BTC[j].n; i++)
        {
            if (!Interface::Option(options_key::single_column_x))
            {   if (j==0) rowlabels<<QString::number(i);
                tablewidget->setItem(i,j*2, new QTableWidgetItem(QString::fromStdString(Label(i,j))));
                tablewidget->setItem(i,j*2+1, new QTableWidgetItem(QString::number(BTC[j].GetC(i))));
            }
            else
            {   if (j==0) rowlabels<<QString::fromStdString(Label(i,j));
                tablewidget->setItem(i,j, new QTableWidgetItem(QString::number(BTC[j].GetC(i))));
            }
        }
    }

    tablewidget->setHorizontalHeaderLabels(headers);
    tablewidget->setVerticalHeaderLabels(rowlabels);
    return tablewidget;
}
