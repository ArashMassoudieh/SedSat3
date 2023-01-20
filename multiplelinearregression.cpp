#include "multiplelinearregression.h"
#include <gsl/gsl_multifit.h>
#include "Vector.h"


MultipleLinearRegression::MultipleLinearRegression():Interface()
{

}

MultipleLinearRegression::MultipleLinearRegression(const MultipleLinearRegression& mp):Interface(mp)
{
    chisq = mp.chisq;
    coefficients_intercept_ = mp.coefficients_intercept_;
    correlation_matrix_ = mp.correlation_matrix_;
    independent_variables_names = mp.independent_variables_names;

}
MultipleLinearRegression& MultipleLinearRegression::operator=(const MultipleLinearRegression &mp)
{
    Interface::operator=(mp);
    chisq = mp.chisq;
    coefficients_intercept_ = mp.coefficients_intercept_;
    correlation_matrix_ = mp.correlation_matrix_;
    independent_variables_names = mp.independent_variables_names;
    return *this;
}
bool MultipleLinearRegression::Regress(const vector<vector<double>> &independent, const vector<double> dependent, const vector<string> &indep_var_names)
{
    if (independent.size()==0) return false;
    if (independent[0].size()!=dependent.size()) return false;
    independent_variables_names = indep_var_names;
    int number_of_data_points = dependent.size();
    int number_of_variables = independent.size();

    double chisq;
    gsl_matrix *X, *cov;
    gsl_vector *y, *w, *c;

    X = gsl_matrix_alloc (number_of_data_points, number_of_variables+1);
    y = gsl_vector_alloc (number_of_data_points);
    w = gsl_vector_alloc (number_of_data_points);

    c = gsl_vector_alloc (number_of_variables+1);
    cov = gsl_matrix_alloc (number_of_variables+1, number_of_variables+1);

    for (int i = 0; i < number_of_data_points; i++)
    {
        gsl_matrix_set (X, i, 0, 1.0);
        for (int j=0; j<number_of_variables; j++)
        {
            gsl_matrix_set (X, i, j+1, independent[j][i]);
        }
        gsl_vector_set (y, i, dependent[i]);
        gsl_vector_set (w, i, 1.0);
    }

    {
        gsl_multifit_linear_workspace * work
          = gsl_multifit_linear_alloc (number_of_data_points, number_of_variables+1);
        gsl_multifit_wlinear (X, w, y, c, cov,&chisq, work);
        gsl_multifit_linear_free (work);
    }


    coefficients_intercept_.resize(number_of_variables+1);
    for (int i=0; i<number_of_variables+1; i++)
    {
        coefficients_intercept_[i] = gsl_vector_get(c,i);
    }

    correlation_matrix_ = CMBMatrix(number_of_variables+1);
    for (int i=0; i<number_of_variables+1; i++)
        for (int j=0; j<number_of_variables+1; j++)
            correlation_matrix_[i][j] = gsl_matrix_get(cov,i,j);

    gsl_matrix_free (X);
    gsl_vector_free (y);
    gsl_vector_free (w);
    gsl_vector_free (c);
    gsl_matrix_free (cov);

    return true;

}
QJsonObject MultipleLinearRegression::toJsonObject()
{
    QJsonObject out;
    out["Intercept"] = coefficients_intercept_[0];
    for (unsigned int i=1; i<coefficients_intercept_.size(); i++)
    {
        out[QString::fromStdString("Coefficient for " + independent_variables_names[i-1])] = coefficients_intercept_[i];
    }
    return out;

}
bool MultipleLinearRegression::ReadFromJsonObject(const QJsonObject &jsonobject)
{
    independent_variables_names.clear();
    coefficients_intercept_[0] = jsonobject["Intercept"].toDouble();
    int i=1;
    for(QString key: jsonobject.keys() ) {
        if (key.contains("Coefficient for "))
        {   coefficients_intercept_[i] = jsonobject[key].toDouble();
            independent_variables_names[i-1] = key.split("Coefficient for ")[0].toStdString();
        }
    }
}

vector<double> MultipleLinearRegression::CoefficientsIntercept()
{
    return coefficients_intercept_;
}

string MultipleLinearRegression::ToString()
{
    string out;
    out += "Intercept: " + QString::number(coefficients_intercept_[0]).toStdString() + "\n";
    for (unsigned int i=1; i<coefficients_intercept_.size(); i++)
    {
        out += "Coefficient for " + independent_variables_names[i-1] +":" + QString::number(coefficients_intercept_[i]).toStdString() + "\n";
    }
    return out;
}


