#include "multiplelinearregressionset.h"

MultipleLinearRegressionSet::MultipleLinearRegressionSet():map<string, MultipleLinearRegression>(),Interface()
{

}

MultipleLinearRegressionSet::MultipleLinearRegressionSet(const MultipleLinearRegressionSet& mp):map<string, MultipleLinearRegression>(mp),Interface()
{

}
MultipleLinearRegressionSet& MultipleLinearRegressionSet::operator=(const MultipleLinearRegressionSet &mp)
{
    map<string,MultipleLinearRegression>::operator=(mp);
    Interface::operator=(mp);
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
    }
    return out;
}
