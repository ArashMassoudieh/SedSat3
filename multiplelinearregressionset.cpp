#include "multiplelinearregressionset.h"

MultipleLinearRegressionSet::MultipleLinearRegressionSet():map<string, MultipleLinearRegression>(),Interface()
{

}

MultipleLinearRegressionSet::MultipleLinearRegressionSet(const MultipleLinearRegressionSet& mp):map<string, MultipleLinearRegression>(mp),Interface()
{
    Source = mp.Source; 
}
MultipleLinearRegressionSet& MultipleLinearRegressionSet::operator=(const MultipleLinearRegressionSet &mp)
{
    map<string,MultipleLinearRegression>::operator=(mp);
    Interface::operator=(mp);
    Source = mp.Source; 
    return *this; 
}
QJsonObject MultipleLinearRegressionSet::toJsonObject()
{
    QJsonObject out;
    for (map<string,MultipleLinearRegression>::iterator it = begin(); it!= end(); it++)
    {
        out[QString::fromStdString(it->first)] = it->second.toJsonObject();
    }
    return out;
}
bool MultipleLinearRegressionSet::ReadFromJsonObject(const QJsonObject &jsonobject)
{
    clear();
    for(QString key: jsonobject.keys() ) {
        MultipleLinearRegression MLR;
        MLR.ReadFromJsonObject(jsonobject[key].toObject());
        operator[](key.toStdString()) = MLR;
    }
    return true;
}

bool MultipleLinearRegressionSet::Append(QString key, const MultipleLinearRegression &MLR)
{
    if (count(key.toStdString())==0)
    {   operator[](key.toStdString()) = MLR;
        return true;
    }
    return false;

}

string MultipleLinearRegressionSet::ToString()
{
    string out;
    for (map<string,MultipleLinearRegression>::iterator it = begin(); it!= end(); it++)
    {
        out += it->first + ":\n";
        out += it->second.ToString();
        out += "_______________________________________\n";
    }
    return out;
}

QTableWidget *MultipleLinearRegressionSet::ToTable()
{
    QTableWidget *tablewidget = new QTableWidget();
    tablewidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    if (begin()->second.CoefficientsIntercept().size()>2)
        tablewidget->setColumnCount(5);
    else
        tablewidget->setColumnCount(3);

    tablewidget->setRowCount(size());
    QStringList headers;
    QStringList sources;
    int i=0;
    for (map<string,MultipleLinearRegression>::iterator it=begin(); it!=end(); it++ )
    {
        sources<<QString::fromStdString(it->first);
        tablewidget->setItem(i,0, new QTableWidgetItem(QString::number(it->second.CoefficientsIntercept()[0])));
        tablewidget->setItem(i,1, new QTableWidgetItem(QString::number(it->second.CoefficientsIntercept()[1])));
        tablewidget->setItem(i,2, new QTableWidgetItem(QString::number(it->second.P_Value()[0])));
        if (it->second.P_Value()[0]< it->second.PValueThreshold())
            tablewidget->item(i, 2)->setForeground(Qt::red);
        if (it->second.CoefficientsIntercept().size() > 2)
        {
            tablewidget->setItem(i, 3, new QTableWidgetItem(QString::number(it->second.CoefficientsIntercept()[2])));
            tablewidget->setItem(i, 4, new QTableWidgetItem(QString::number(it->second.P_Value()[1])));
            if (it->second.P_Value()[1] < it->second.PValueThreshold())
                tablewidget->item(i, 4)->setForeground(Qt::red);
        }
        i++;
    }
    if (begin()->second.CoefficientsIntercept().size() > 2)
        headers << "Intercept" << QString::fromStdString(begin()->second.GetIndependentVariableNames()[0]) + " coefficient" << QString::fromStdString(begin()->second.GetIndependentVariableNames()[0]) + " P-value" << QString::fromStdString(begin()->second.GetIndependentVariableNames()[1]) + " coefficient" << QString::fromStdString(begin()->second.GetIndependentVariableNames()[1]) + " P-value";
    else
        headers << "Intercept" << QString::fromStdString(begin()->second.GetIndependentVariableNames()[0]) + " coefficient" << QString::fromStdString(begin()->second.GetIndependentVariableNames()[0]) + " P-value" << "Size coefficient" << "Size P-value";
    tablewidget->setHorizontalHeaderLabels(headers);
    tablewidget->setVerticalHeaderLabels(sources);
    return tablewidget;
}

string MultipleLinearRegressionSet::Key(int i)
{
    string out;
    int counter = 0;
    for (map<string,MultipleLinearRegression>::iterator it = begin(); it!= end(); it++)
    {
        if (counter==i)
            return it->first;
        counter++;
    }
    return "";
}
