#include "multiplelinearregression.h"
#include <gsl/gsl_multifit.h>
#include "Vector.h"
#include <gsl/gsl_statistics_double.h>
#include <gsl/gsl_cdf.h>
#include <qjsonarray.h>


MultipleLinearRegression::MultipleLinearRegression():Interface()
{

}

MultipleLinearRegression::MultipleLinearRegression(const MultipleLinearRegression& mp):Interface(mp)
{
    chisq = mp.chisq;
    R2 = mp.R2;
    R2_adj = mp.R2_adj;
    p_value = mp.p_value;
    coefficients_intercept_ = mp.coefficients_intercept_;
    correlation_matrix_ = mp.correlation_matrix_;
    independent_variables_names = mp.independent_variables_names;
    dependent_data = mp.dependent_data;
    dependent_variable_name = mp.dependent_variable_name;
    independent_data = mp.independent_data;
    make_effective = mp.make_effective;
    regressionEquation = mp.regressionEquation;
    p_value_threshold = mp.p_value_threshold; 

}
MultipleLinearRegression& MultipleLinearRegression::operator=(const MultipleLinearRegression &mp)
{
    Interface::operator=(mp);
    chisq = mp.chisq;
    R2 = mp.R2;
    R2_adj = mp.R2_adj;
    p_value = mp.p_value;
    coefficients_intercept_ = mp.coefficients_intercept_;
    correlation_matrix_ = mp.correlation_matrix_;
    independent_variables_names = mp.independent_variables_names;
    dependent_data = mp.dependent_data;
    independent_data = mp.independent_data;
    dependent_variable_name = mp.dependent_variable_name;
    make_effective = mp.make_effective;
    regressionEquation = mp.regressionEquation;
    p_value_threshold = mp.p_value_threshold;
    return *this;
}

double MultipleLinearRegression::SSE_reduced_model(const vector<vector<double>> &independent, const vector<double> dependent, int eliminated_var)
{
    if (independent.size()==0) return false;
    if (independent[0].size()!=dependent.size()) return false;
    int number_of_data_points = dependent.size();
    int number_of_variables = independent.size();

    gsl_matrix *X, *cov;
    gsl_vector *y, *w, *c;

    double SSE=0;

    if (number_of_variables==1)
    {
        if (regressionEquation == regression_form::linear)
            SSE = pow(CVector(dependent).stdev(),2)*(dependent.size()-1);
        else
            SSE = pow(CVector(dependent).Log().stdev(),2)*(dependent.size()-1);
        return SSE;
    }

    X = gsl_matrix_alloc (number_of_data_points, number_of_variables);
    y = gsl_vector_alloc (number_of_data_points);
    w = gsl_vector_alloc (number_of_data_points);

    c = gsl_vector_alloc (number_of_variables);
    cov = gsl_matrix_alloc (number_of_variables, number_of_variables);

    for (int i = 0; i < number_of_data_points; i++)
    {
        gsl_matrix_set (X, i, 0, 1.0);
        for (int j=0; j<number_of_variables; j++)
        {
            if (j<eliminated_var)
            {   if (regressionEquation==regression_form::linear)
                    gsl_matrix_set (X, i, j+1, independent[j][i]);
                else
                    gsl_matrix_set (X, i, j+1, log(independent[j][i]));
            }
            else if (j>eliminated_var)
            {   if (regressionEquation==regression_form::linear)
                    gsl_matrix_set (X, i, j, independent[j][i]);
                else
                    gsl_matrix_set (X, i, j, log(independent[j][i]));
            }
            if (regressionEquation==regression_form::linear)
                gsl_vector_set (y, i, dependent[i]);
            else
                gsl_vector_set (y, i, log(dependent[i]));
            gsl_vector_set (w, i, 1.0);
        }
    }

    {
        gsl_multifit_linear_workspace * work
          = gsl_multifit_linear_alloc (number_of_data_points, number_of_variables);
        gsl_multifit_wlinear (X, w, y, c, cov,&SSE, work);
        gsl_multifit_linear_free (work);
    }

    return SSE;

}

double MultipleLinearRegression::Regress(const vector<vector<double>> &independent, const vector<double> dependent, const vector<string> &indep_var_names)
{
    if (independent.size()==0) return false;
    if (independent[0].size()!=dependent.size()) return false;
    independent_variables_names = indep_var_names;
    int number_of_data_points = dependent.size();
    int number_of_variables = independent.size();

    gsl_matrix *X, *cov;
    gsl_vector *y, *w, *c;

    X = gsl_matrix_alloc (number_of_data_points, number_of_variables+1);
    y = gsl_vector_alloc (number_of_data_points);
    w = gsl_vector_alloc (number_of_data_points);

    c = gsl_vector_alloc (number_of_variables+1);
    cov = gsl_matrix_alloc (number_of_variables+1, number_of_variables+1);
    independent_data = independent;
    dependent_data = dependent;
    for (int i = 0; i < number_of_data_points; i++)
    {

        gsl_matrix_set (X, i, 0, 1.0);
        vector<double> independent_column;
        for (int j=0; j<number_of_variables; j++)
        {
            if (regressionEquation==regression_form::linear)
                gsl_matrix_set (X, i, j+1, independent[j][i]);
            else
                gsl_matrix_set (X, i, j+1, log(independent[j][i]));

        }

        if (regressionEquation==regression_form::linear)
            gsl_vector_set (y, i, dependent[i]);
        else
            gsl_vector_set (y, i, log(dependent[i]));
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

    double var_y = gsl_stats_tss(y->data, y->stride, y->size);

    R2 = 1-chisq/var_y;
    R2_adj = 1-(chisq/(number_of_data_points-(number_of_variables+1))/(var_y/(number_of_data_points-1)));
    p_value.clear();
    make_effective.clear();
    for (unsigned int i=0; i<number_of_variables; i++)
    {
        double SSE_reduced = SSE_reduced_model(independent,dependent, i);
        double F = (SSE_reduced - chisq)/chisq*(number_of_data_points-number_of_variables-1);
        p_value.push_back(gsl_cdf_fdist_Q (F, number_of_variables, number_of_data_points-(number_of_variables)));
        if (p_value[i]<p_value_threshold && aquiutils::lookup(independent_variables_names,dependent_variable_name)==-1)
            make_effective.push_back(true);
        else
            make_effective.push_back(false);
        qDebug()<<chisq<<","<<SSE_reduced<<","<<F<<","<<p_value[i];

    }

    gsl_matrix_free (X);
    gsl_vector_free (y);
    gsl_vector_free (w);
    gsl_vector_free (c);
    gsl_matrix_free (cov);

    return chisq;

}
QJsonObject MultipleLinearRegression::toJsonObject()
{
    QJsonObject out;
    if (regressionEquation==regression_form::linear)
        out["form"]="Linear";
    else
        out["form"]="Power";
    out["Intercept"] = coefficients_intercept_[0];
    QJsonArray json_dependent_data;
    for (unsigned int i = 0; i < dependent_data.size(); i++)
        json_dependent_data << dependent_data[i];

    out["Dependent Data"] = json_dependent_data;


    QJsonArray json_independent_data;
    for (unsigned int j = 0; j < independent_data.size(); j++)
    {
        QJsonArray json_independent_data_item;
        for (unsigned int i = 0; i < independent_data[j].size(); i++)
            json_independent_data_item.append(independent_data[j][i]);

        json_independent_data.append(json_independent_data_item);
    }

    out["Independent Data"] = json_independent_data;


    for (unsigned int i=1; i<coefficients_intercept_.size(); i++)
    {

        out[QString::fromStdString("Coefficient for;" + aquiutils::numbertostring(i)+";" + independent_variables_names[i-1])] = coefficients_intercept_[i];
        out[QString::fromStdString("P-value for;" + aquiutils::numbertostring(i)+";" + independent_variables_names[i-1])] = p_value[i-1];
        out[QString::fromStdString("Effective for;" + aquiutils::numbertostring(i)+";" + independent_variables_names[i-1])] = Effective(i-1);
    }


    out["Chisq"] = chisq;
    out["R2"] = R2;
    out["R2_adjusted"]=R2_adj;


    return out;

}
bool MultipleLinearRegression::ReadFromJsonObject(const QJsonObject &jsonobject)
{
    independent_variables_names.clear();
    coefficients_intercept_.clear();
    p_value.clear();
    if (jsonobject["form"]=="Power")
        regressionEquation = regression_form::power;
    else
        regressionEquation = regression_form::linear;
    coefficients_intercept_.push_back(jsonobject["Intercept"].toDouble());
    int i=1;
    for(QString key: jsonobject.keys() ) {
        if (key.contains("Coefficient for"))
        {   coefficients_intercept_.push_back(jsonobject[key].toDouble());
            independent_variables_names.push_back(key.split(";")[2].toStdString());

        }
        if (key.contains("P-value for"))
        {
            p_value.push_back(jsonobject[key].toDouble());
        }
        if (key.contains("Effective for"))
        {
            make_effective.push_back(jsonobject[key].toBool());
        }
    }

// Read dependent and independent data here
    QJsonArray json_dependent_data = jsonobject["Dependent Data"].toArray();
    for (unsigned int i = 0; i < json_dependent_data.size(); i++)
    {
        dependent_data.push_back(json_dependent_data[i].toDouble());
    }

    QJsonArray json_independent_data = jsonobject["Independent Data"].toArray();
    for (unsigned int i = 0; i < json_independent_data.size(); i++)
    {
        vector<double> independent_data_item; 
        QJsonArray json_independent_data_item = json_independent_data[i].toArray();
        for (unsigned int j = 0; j < json_independent_data_item.size(); j++)
        {
            independent_data_item.push_back(json_independent_data_item[j].toDouble());
        }
        independent_data.push_back(independent_data_item);
    }

    R2 = jsonobject["R2"].toDouble();
    R2_adj = jsonobject["R2_adjusted"].toDouble();
    chisq = jsonobject["Chisq"].toDouble();
    return true;
}

vector<double> MultipleLinearRegression::CoefficientsIntercept() const
{
    return coefficients_intercept_;
}


double MultipleLinearRegression::MeanIndependentVar(int i)
{
    return CVector(independent_data[i]).mean();
}

double MultipleLinearRegression::GeoMeanIndependentVar(int i)
{
    return exp(CVector(independent_data[i]).Log().mean());
}
string MultipleLinearRegression::ToString()
{
    string out;
    if (regressionEquation == regression_form::linear)
    {   out += "form: Linear\n";
        out += "Coefficient: " + QString::number(coefficients_intercept_[0]).toStdString() + "\n";
    }
    else
    {   out += "form: Power\n";
        out += "Coefficient: " + QString::number(coefficients_intercept_[0]).toStdString() + "\n";
    }

    for (unsigned int i=1; i<coefficients_intercept_.size(); i++)
    {
        if (regressionEquation == regression_form::linear)
            out += "Coefficient for " + independent_variables_names[i-1] +":" + QString::number(coefficients_intercept_[i]).toStdString() + "\n";
        else
            out += "Exponent for " + independent_variables_names[i-1] +":" + QString::number(coefficients_intercept_[i]).toStdString() + "\n";
        out += "P-value for " + independent_variables_names[i-1] +":" + QString::number(p_value[i-1]).toStdString() + "\n";

    }
    out += "Chisq: " + QString::number(chisq).toStdString() + "\n";
    out += "R2: " + QString::number(R2).toStdString() + "\n";
    out += "Adusted R2: " + QString::number(R2_adj).toStdString() + "\n";
    return out;
}

vector<double> &MultipleLinearRegression::IndependentData(const string &var_name)
{
    for (int i=0; i<independent_variables_names.size(); i++)
        if (var_name == independent_variables_names[i])
            return independent_data[i];

}

