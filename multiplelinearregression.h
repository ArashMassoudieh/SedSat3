#ifndef MULTIPLELINEARREGRESSION_H
#define MULTIPLELINEARREGRESSION_H

#include "interface.h"
#include "cmbmatrix.h"

enum class regression_form {linear, power};

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
    vector<string> &GetIndependentVariableNames() {return independent_variables_names;}
    vector<double> &IndependentData(const string &var_name);
    vector<double> &DependentData()
    {
        return dependent_data;
    }
    double MeanIndependentVar(int i);
    double GeoMeanIndependentVar(int i);
    string DependentVariableName() {return dependent_variable_name;}
    void SetDependentVariableName(const string name) {dependent_variable_name = name;}
    vector<double> P_Value() {return p_value;}
    void SetEffective(int i, bool eff) {make_effective[i] = eff;}
    bool Effective(int i) {return make_effective[i];}
    regression_form Equation() {return regressionEquation;}
    void SetEquation(regression_form form) {
        regressionEquation = form;
    }
private:
    vector<double> coefficients_intercept_;
    vector<vector<double>> independent_data;
    vector<double> dependent_data;
    string dependent_variable_name;
    vector<string> independent_variables_names;
    CMBMatrix correlation_matrix_;
    double chisq, R2, R2_adj;
    vector<double> p_value;
    vector<bool> make_effective;
    regression_form regressionEquation = regression_form::linear;
};

#endif // MULTIPLELINEARREGRESSION_H
