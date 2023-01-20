#ifndef MULTIPLELINEARREGRESSION_H
#define MULTIPLELINEARREGRESSION_H

#include "interface.h"
#include "cmbmatrix.h"

class MultipleLinearRegression: public Interface
{
public:
    MultipleLinearRegression();
    MultipleLinearRegression(const MultipleLinearRegression& mp);
    MultipleLinearRegression& operator=(const MultipleLinearRegression &mp);
    bool Regress(const vector<vector<double>> &independent, const vector<double> dependent, const vector<string> &indep_vars_names);
    QJsonObject toJsonObject() override;
    string ToString() override;
    bool ReadFromJsonObject(const QJsonObject &jsonobject) override;
    vector<double> CoefficientsIntercept();
private:
    vector<double> coefficients_intercept_;
    vector<string> independent_variables_names;
    CMBMatrix correlation_matrix_;
    double chisq;
};

#endif // MULTIPLELINEARREGRESSION_H
