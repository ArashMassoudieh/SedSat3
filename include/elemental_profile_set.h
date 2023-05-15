#ifndef ELEMENTAL_PROFILE_SET_H
#define ELEMENTAL_PROFILE_SET_H

#include <elemental_profile.h>
#include <map>
#include <string>
#include <vector>
#include "concentrationset.h"
#include "interface.h"
#include "multiplelinearregressionset.h"
#include "resultitem.h"
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include "cmbvector.h"

using namespace std;



class Elemental_Profile_Set: public map<string, Elemental_Profile>, public Interface
{
public:
    Elemental_Profile_Set();
    Elemental_Profile_Set(const Elemental_Profile_Set& mp);
    Elemental_Profile_Set& operator=(const Elemental_Profile_Set &mp);
    Elemental_Profile_Set CopyandCorrect(bool exclude_samples, bool exclude_elements, bool omnsizecorrect, const double &om, const double &psize, const map<string, element_information> *elementinfo=nullptr) const;
    Elemental_Profile_Set Extract(const vector<string> &element_list) const;
    Elemental_Profile *Profile(const string &name);
    Elemental_Profile Profile(const string &name) const;
    Elemental_Profile *Profile(unsigned int i);
    Elemental_Profile Profile(unsigned int i) const;
    vector<double> GetAllConcentrationsFor(const string &element_name);
    vector<double> GetProfileForSample(const string &source_name);

    Elemental_Profile *Append_Profile(const string &name, const Elemental_Profile &profile=Elemental_Profile(), map<string,element_information> *elementinfo=nullptr);
    void UpdateElementalDistribution();
    vector<string> SampleNames(); // Return the list of the name of samples
    ConcentrationSet *ElementalDistribution(const string &element_name)
    {
        return &element_distributions[element_name];
    }
    ConcentrationSet ElementalDistribution(const string &element_name) const {
        return element_distributions.at(element_name);
    }
    distribution_type DistributionAssigned(const string &element_name)
    {
        if (element_distributions.count(element_name)==0)
            return distribution_type::none;

        return element_distributions[element_name].FittedDistribution()->distribution;
    }
    double Estimated_mu(const string &element) //return the estimated mean for an element
    {
        if (element_distributions.count(element)>0)
            return element_distributions[element].EstimatedMu();
        else
            return 0;
    }
    double Estimated_sigma(const string &element)
    {
        if (element_distributions.count(element)>0)
            return element_distributions[element].EstimatedSigma();
        else
            return 0;
    }
    bool Set_Estimated_mu(const string &element, const double &value)
    {
        if (element_distributions.count(element)>0)
        {   element_distributions[element].SetEstimatedMu(value);
            return true;
        }
        else
            return false;

    }
    bool Set_Estimated_sigma(const string &element, const double &value)
    {
        if (element_distributions.count(element)>0)
        {   element_distributions[element].SetEstimatedSigma(value);
            return true;
        }
        else
            return false;

    }
    bool SetContribution(const double &x)
    {
        contribution = x;
        return true;
    }

    bool SetContribution_softmax(const double &x)
    {
        contribution_softmax = x;
        return true;
    }
    double Contribution() {return contribution; }
    double Contribution_softmax() {return contribution_softmax; }
    Distribution* GetEstimatedDistribution(const string &element_name )
    {
        if (element_distributions.count(element_name)>0)
            return element_distributions[element_name].GetEstimatedDistribution();
        else
            return nullptr;

    }

    Distribution* GetFittedDistribution(const string& element_name)
    {
        if (element_distributions.count(element_name) > 0)
            return element_distributions[element_name].FittedDistribution();
        else
            return nullptr;

    }
    string ToString() override;
    vector<string> ElementNames();
    double max();
    double min(); 
    bool writetofile(QFile*) override;
    QJsonObject toJsonObject() override;
    bool ReadFromJsonObject(const QJsonObject &jsonobject) override;
    bool Read(const QStringList &strlist) override;
    bool ContainsElement(const string &elementname);
    MultipleLinearRegressionSet regress_vs_size_OM(const string &om, const string &d,regression_form form=regression_form::linear);
    MultipleLinearRegression regress_vs_size_OM(const string &element, const string &om, const string &d, regression_form form=regression_form::linear);
    void SetRegression(const string &om, const string &d, regression_form form = regression_form::linear);
    void SetRegression(const MultipleLinearRegressionSet *mlrset);
    MultipleLinearRegressionSet* GetExistingRegressionSet();
    ResultItem GetRegressions();
    CMBMatrix CovarianceMatrix();
    CMBMatrix CorrelationMatrix();
    CMBVector KolmogorovSmirnovStat(distribution_type dist_type);
    gsl_matrix *CopytoGSLMatrix();
    CVector ElementMeans();
    QTableWidget *ToTable() override;
    Elemental_Profile_Set CopyIncludedinAnalysis(bool applyomsizecorrection, const double &om, const double &size, map<string, element_information> *elementinfo=nullptr);
    Elemental_Profile_Set OrganicandSizeCorrect(const double &size, const double &om);
    CMBVector BoxCoxParameters();
    CMBMatrix Outlier();
    CMBMatrix toMatrix();
    Elemental_Profile_Set BocCoxTransformed(CMBVector *lambda_vals=nullptr);
private:
    map<string,ConcentrationSet> element_distributions; // concentrations for each element
    MultipleLinearRegressionSet mlr_vs_om_size;
    double contribution = 0;
    double contribution_softmax = 0;

};

#endif // ELEMENTAL_PROFILE_SET_H
