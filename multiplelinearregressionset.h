#ifndef MULTIPLELINEARREGRESSIONSET_H
#define MULTIPLELINEARREGRESSIONSET_H

#include "interface.h"
#include "multiplelinearregression.h"

class MultipleLinearRegressionSet: public map<string,MultipleLinearRegression>, Interface
{
public:
    MultipleLinearRegressionSet();
    MultipleLinearRegressionSet(const MultipleLinearRegressionSet& mp);
    MultipleLinearRegressionSet& operator=(const MultipleLinearRegressionSet &mp);
    QJsonObject toJsonObject() override;
    bool ReadFromJsonObject(const QJsonObject &jsonobject) override;

};

#endif // MULTIPLELINEARREGRESSIONSET_H
