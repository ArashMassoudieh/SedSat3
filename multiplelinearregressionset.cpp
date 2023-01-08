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

    return out;
}
bool MultipleLinearRegressionSet::ReadFromJsonObject(const QJsonObject &jsonobject)
{
    return true;
}

