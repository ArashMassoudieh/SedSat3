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
    double Regress(const vector<vector<double>> &independent, const vector<double> dependent, const vector<string> &indep_vars_names);
    double SSE_reduced_model(const vector<vector<double>> &independent, const vector<double> dependent, int eliminated_var);
    QJsonObject toJsonObject() override;
    string ToString() override;
    bool ReadFromJsonObject(const QJsonObject &jsonobject) override;
    vector<double> CoefficientsIntercept();
private:
    vector<double> coefficients_intercept_;
    vector<string> independent_variables_names;
    CMBMatrix correlation_matrix_;
    double chisq, R2, R2_adj;
    vector<double> p_value;
};

#endif // MULTIPLELINEARREGRESSION_H
