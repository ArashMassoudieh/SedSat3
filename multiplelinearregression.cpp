#include "multiplelinearregression.h"
#include <gsl/gsl_multifit.h>

MultipleLinearRegression::MultipleLinearRegression():Interface()
{

}

MultipleLinearRegression::MultipleLinearRegression(const MultipleLinearRegression& mp):Interface(mp)
{

}
MultipleLinearRegression& MultipleLinearRegression::operator=(const MultipleLinearRegression &mp)
{
    Interface::operator=(mp);
}
bool MultipleLinearRegression::Regress(const vector<vector<double>> &independent, const vector<double> dependent)
{
    if (independent.size()==0) return false;
    if (independent[0].size()!=dependent.size()) return false;
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

    #define C(i) (gsl_vector_get(c,(i)))
    #define COV(i,j) (gsl_matrix_get(cov,(i),(j)))

    coefficients_intercept_.resize(number_of_variables+1);
    for (int i=0; i<number_of_variables+1; i++)
    {
        coefficients_intercept_[i] = gsl_vector_get(c,i);
    }

/*
        printf ("# covariance matrix:\n");
        printf ("[ %+.5e, %+.5e, %+.5e  \n",
                   COV(0,0), COV(0,1), COV(0,2));
        printf ("  %+.5e, %+.5e, %+.5e  \n",
                   COV(1,0), COV(1,1), COV(1,2));
        printf ("  %+.5e, %+.5e, %+.5e ]\n",
                   COV(2,0), COV(2,1), COV(2,2));
        printf ("# chisq = %g\n", chisq);
*/


      gsl_matrix_free (X);
      gsl_vector_free (y);
      gsl_vector_free (w);
      gsl_vector_free (c);
      gsl_matrix_free (cov);

      return true;
    }
}
QJsonObject MultipleLinearRegression::toJsonObject()
{

}
bool MultipleLinearRegression::ReadFromJsonObject(const QJsonObject &jsonobject)
{

}

vector<double> MultipleLinearRegression::CoefficientsIntercept()
{

}
