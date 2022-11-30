#include "sourcesinkdata.h"
#include "iostream"
#include "NormalDist.h"



SourceSinkData::SourceSinkData():map<string, Elemental_Profile_Set>()
{

}

SourceSinkData::SourceSinkData(const SourceSinkData& mp):map<string, Elemental_Profile_Set>(mp)
{

    ElementInformation = mp.ElementInformation;
    element_distributions = mp.element_distributions;
    numberofconstituents = mp.numberofconstituents;
    numberofsourcesamplesets = mp.numberofsourcesamplesets;
    observations = mp.observations;
    outputpath = mp.outputpath;
    parameters = mp.parameters;
    target_group = mp.target_group;
    samplesetsorder = mp.samplesetsorder;
    constituent_order = mp.constituent_order;
    selected_target_sample = mp.selected_target_sample;
    element_order = mp.element_order;
    isotope_order = mp.isotope_order;
    size_om_order = mp.size_om_order;
    parameter_estimation_mode = mp.parameter_estimation_mode;


}
SourceSinkData& SourceSinkData::operator=(const SourceSinkData &mp)
{
    map<string, Elemental_Profile_Set>::operator=(mp);
    ElementInformation = mp.ElementInformation;
    element_distributions = mp.element_distributions;
    numberofconstituents = mp.numberofconstituents;
    numberofsourcesamplesets = mp.numberofsourcesamplesets;
    observations = mp.observations;
    outputpath = mp.outputpath;
    parameters = mp.parameters;
    target_group = mp.target_group;
    samplesetsorder = mp.samplesetsorder;
    constituent_order = mp.constituent_order;
    element_order = mp.element_order;
    isotope_order = mp.isotope_order;
    size_om_order = mp.size_om_order;
    selected_target_sample = mp.selected_target_sample;
    parameter_estimation_mode = mp.parameter_estimation_mode;
    return *this;
}

void SourceSinkData::Clear()
{
    ElementInformation.clear();
    element_distributions.clear();
    numberofconstituents = 0;
    numberofsourcesamplesets = 0;
    observations.clear();
    outputpath = "";
    parameters.clear();
    target_group = "";
    samplesetsorder.clear();
    constituent_order.clear();
    element_order.clear();
    isotope_order.clear();
    size_om_order.clear();
    selected_target_sample = "";
}

Elemental_Profile_Set* SourceSinkData::AppendSampleSet(const string &name, const Elemental_Profile_Set &elemental_profile_set)
{
    if (count(name)==0)
        operator[](name) = elemental_profile_set;
    else
    {   cout<<"Sample set type '" + name + "' already exists!"<<std::endl;
        return nullptr;
    }

    return &operator[](name);
}

Elemental_Profile_Set *SourceSinkData::sample_set(const string &name)
{
    if (count(name)!=0)
    {
        return &operator[](name);
    }
    return nullptr;
}

vector<string> SourceSinkData::SampleNames(const string groupname)
{
    if (sample_set(groupname))
    {
        return sample_set(groupname)->SampleNames();
    }
    return vector<string>();
}

vector<string> SourceSinkData::GroupNames()
{
    vector<string> out;
    for (map<string,Elemental_Profile_Set>::iterator it=begin(); it!=end(); it++)
    {
        out.push_back(it->first);
    }
    return out;
}
vector<string> SourceSinkData::ElementNames()
{
    vector<string> out;
    if (size()>0)
    for (map<string,double>::iterator it=begin()->second.begin()->second.begin(); it!=begin()->second.begin()->second.end(); it++)
    {
        out.push_back(it->first);
    }
    return out;
}

profiles_data SourceSinkData::ExtractData(const vector<vector<string>> &indicators)
{
    profiles_data extracted_data;
    extracted_data.element_names = ElementNames();
    for (int i=0; i<indicators.size(); i++)
    {
        vector<double> vals = sample_set(indicators[i][0])->GetProfileForSample(indicators[i][1]);
        extracted_data.values.push_back(vals);
        extracted_data.sample_names.push_back(indicators[i][1]);
    }
    return extracted_data;
}

element_data SourceSinkData::ExtractElementData(const string &element, const string &group)
{
    element_data extracted_data;
    extracted_data.group_name = group;
    for (map<string,Elemental_Profile>::iterator profile=sample_set(group)->begin(); profile!=sample_set(group)->end() ; profile++)
    {
        extracted_data.values.push_back(profile->second.Val(element));
        extracted_data.sample_names.push_back(profile->first);
    }
    return extracted_data;
}
void SourceSinkData::PopulateElementDistributions()
{
    vector<string> element_names = ElementNames();
    for (map<string,Elemental_Profile_Set>::iterator it=begin(); it!=end(); it++)
    {
        for (unsigned int i=0; i<element_names.size(); i++)
        {
            element_distributions[element_names[i]].Append(it->second.ElementalDistribution(element_names[i]));
        }
    }
}

map<string,vector<double>> SourceSinkData::ExtractElementData(const string &element)
{
    map<string,vector<double>> extracted_data;
    for (unsigned int i=0; i<GroupNames().size(); i++)
    {
        element_data row = ExtractElementData(element,GroupNames()[i]);
        extracted_data[GroupNames()[i]]=row.values;
    }
    return extracted_data;
}
void SourceSinkData::AssignAllDistributions()
{
    vector<string> element_names = ElementNames();
    for (unsigned int i=0; i<element_names.size(); i++)
    {
        element_distributions[element_names[i]].FittedDistribution()->distribution = element_distributions[element_names[i]].SelectBestDistribution();
        element_distributions[element_names[i]].FittedDistribution()->parameters = element_distributions[element_names[i]].EstimateParameters();
        for (map<string, Elemental_Profile_Set>::iterator it=begin(); it!=end(); it++)
        {
            it->second.ElementalDistribution(element_names[i])->FittedDistribution()->distribution = FittedDistribution(element_names[i])->distribution;
            it->second.ElementalDistribution(element_names[i])->FittedDistribution()->parameters = it->second.ElementalDistribution(element_names[i])->EstimateParameters(FittedDistribution(element_names[i])->distribution);
        }
    }
}

Distribution *SourceSinkData::FittedDistribution(const string &element_name)
{
    if (element_distributions.count(element_name)>0)
        return element_distributions[element_name].FittedDistribution();
    else
        return nullptr;
}

void SourceSinkData::PopulateElementInformation()
{
    ElementInformation.clear();
    vector<string> element_names = ElementNames();
    for (unsigned int i=0; i<element_names.size(); i++)
    {
        ElementInformation[element_names[i]] = element_information();
    }
}

bool SourceSinkData::Execute(const string &command, const map<string,string> &arguments)
{
    return true;
}

string SourceSinkData::OutputPath()
{
    return outputpath;
}

bool SourceSinkData::SetOutputPath(const string &oppath)
{
    outputpath = oppath;
    return true;
}

string SourceSinkData::GetNameForParameterID(int i)
{
    if (parameter(i))
    {
        return parameter(i)->Name();
    }
    return "";
}

bool SourceSinkData::InitializeContributionsRandomly()
{
    sample_set(SourceOrder()[0])->SetContribution(1.2);
    while (ContributionVector(false).min()<0 || ContributionVector(false).sum()>1)
    {
        for (int i=0; i<samplesetsorder.size()-1; i++)
        {
            SetContribution(i,unitrandom());
        }
    }
    return true;
}

bool SourceSinkData::InitializeContributionsRandomly_softmax()
{
    sample_set(SourceOrder()[0])->SetContribution(1.2);
    CVector X = getnormal(samplesetsorder.size(), 0, 1).vec;
    SetContribution_softmax(X);
    return true;
}


bool SourceSinkData::InitializeParametersObservations(const string &targetsamplename, estimation_mode est_mode)
{
   populate_constituent_orders();
   selected_target_sample = targetsamplename;
   if (size()==0)
   {
        cout<<"Data has not been loaded!"<<std::endl;
        return false;
   }
   numberofsourcesamplesets = size()-1;
// Source contributions
    parameters.clear();
    samplesetsorder.clear();
    if (est_mode != estimation_mode::source_elemental_profiles_based_on_source_data)
    {   for (map<string,Elemental_Profile_Set>::iterator it = begin(); it!=end(); it++)
        {
            if (it->first!=TargetGroup())
            {
                Parameter p;
                p.SetName(it->first + "_contribution");
                p.SetPriorDistribution(distribution_type::dirichlet);
                p.SetRange(0, 1);
                parameters.push_back(p);
                samplesetsorder.push_back(it->first);
            }
        }
        parameters.pop_back();
    }
    else
    {
        for (map<string,Elemental_Profile_Set>::iterator it = begin(); it!=end(); it++)
        {
            if (it->first!=TargetGroup())
            {
                samplesetsorder.push_back(it->first);
            }
        }
    }
// Count the number of elements to be used
    numberofconstituents = 0;
    for (map<string, element_information>::iterator it = ElementInformation.begin(); it!=ElementInformation.end(); it++)
        if (it->second.Role == element_information::role::element)
            numberofconstituents ++;


    // Copying Estimated Distribution from the Fitted Distribution
    for (map<string, element_information>::iterator element_iterator = ElementInformation.begin(); element_iterator!=ElementInformation.end(); element_iterator++)
    {
        if (element_iterator->second.Role == element_information::role::element)
        {
            for (map<string,Elemental_Profile_Set>::iterator source_iterator = begin(); source_iterator!=end(); source_iterator++)
            {
                if (source_iterator->first!=target_group)
                {
                    *source_iterator->second.GetEstimatedDistribution(element_iterator->first) = *source_iterator->second.GetFittedDistribution(element_iterator->first);
                }
            }
        }
    }


    // Source elemental profile geometrical mean for each element
    if (est_mode != estimation_mode::only_contributions)
    {   for (map<string, element_information>::iterator element_iterator = ElementInformation.begin(); element_iterator!=ElementInformation.end(); element_iterator++)
        {
            if (element_iterator->second.Role == element_information::role::element)
            {

                for (map<string,Elemental_Profile_Set>::iterator source_iterator = begin(); source_iterator!=end(); source_iterator++)
                {
                    if (source_iterator->first!=target_group)
                    {   Parameter p;
                        p.SetName(source_iterator->first + "_" + element_iterator->first + "_mu");
                        p.SetPriorDistribution(distribution_type::normal);
                        source_iterator->second.GetEstimatedDistribution(element_iterator->first)->SetType(distribution_type::lognormal);
                        p.SetRange(source_iterator->second.GetFittedDistribution(element_iterator->first)->parameters[0]-0.2, source_iterator->second.GetFittedDistribution(element_iterator->first)->parameters[0]+0.2);
                        parameters.push_back(p);
                    }
                }
            }

        }
    // Source elemental profile standard deviation for each element
        for (map<string, element_information>::iterator element_iterator = ElementInformation.begin(); element_iterator!=ElementInformation.end(); element_iterator++)
        {
            if (element_iterator->second.Role == element_information::role::element)
            {
                for (map<string,Elemental_Profile_Set>::iterator source_iterator = begin(); source_iterator!=end(); source_iterator++)
                {
                    if (source_iterator->first!=target_group)
                    {   Parameter p;
                        p.SetName(source_iterator->first + "_" + element_iterator->first + "_sigma");
                        p.SetPriorDistribution(distribution_type::lognormal);
                        p.SetRange(max(source_iterator->second.GetFittedDistribution(element_iterator->first)->parameters[1] * 0.5,0.001), max(source_iterator->second.GetFittedDistribution(element_iterator->first)->parameters[1] * 2,2.0));
                        parameters.push_back(p);
                    }
                }
            }
        }
    }
// Error Standard Deviation
    if (est_mode != estimation_mode::source_elemental_profiles_based_on_source_data)
    {   Parameter p;
        p.SetName("Error STDev");
        p.SetPriorDistribution(distribution_type::lognormal);
        p.SetRange(1e-5, 1e5);
        parameters.push_back(p);


        // Observations
        for (map<string, element_information>::iterator element_iterator = ElementInformation.begin(); element_iterator!=ElementInformation.end(); element_iterator++)
        {
            if (element_iterator->second.Role == element_information::role::element)
            {
                Observation obs;
                obs.SetName(targetsamplename + "_" + element_iterator->first);
                obs.AppendValues(0,GetElementalProfile(targetsamplename)->Val(element_iterator->first));
                observations.push_back(obs);
            }

        }
    }

    return true;
}

CVector SourceSinkData::GetSourceContributions()
{
    CVector contributions(size()-1);
    for (unsigned long int i=0; i<size()-2; i++)
    {
        contributions[i] = parameter(i)->Value();
    }
    contributions[size()-2] = 1 - contributions.sum();
    return contributions;
}


double SourceSinkData::LogPriorContributions()
{
    if (GetSourceContributions().min()<0)
        return -1e10;
    else
        return 0;
}

double SourceSinkData::LogLikelihoodSourceElementalDistributions()
{
    double logLikelihood = 0;
    for (unsigned int element_counter=0; element_counter<element_order.size(); element_counter++)
    {
        for (unsigned int source_group_counter=0; source_group_counter<numberofsourcesamplesets; source_group_counter++)
        {
            Elemental_Profile_Set *this_source_group = sample_set(samplesetsorder[source_group_counter]);


            for (map<string,Elemental_Profile>::iterator sample = this_source_group->begin(); sample!=this_source_group->end(); sample++)
            {
                logLikelihood += this_source_group->ElementalDistribution(element_order[element_counter])->GetEstimatedDistribution()->EvalLog(sample->second.Val(element_order[element_counter]));
            }

        }
    }
    return logLikelihood;
}

CVector SourceSinkData::ObservedDataforSelectedSample(const string &SelectedTargetSample)
{
    CVector observed_data(element_order.size());
    for (unsigned int i=0; i<element_order.size(); i++)
    {   if (SelectedTargetSample!="")
            observed_data[i] = this->sample_set(TargetGroup())->Profile(SelectedTargetSample)->Val(element_order[i]);
        else if (selected_target_sample!="")
            observed_data[i] = this->sample_set(TargetGroup())->Profile(selected_target_sample)->Val(element_order[i]);
    }
    return observed_data;
}

double SourceSinkData::LogLikelihoodModelvsMeasured(estimation_mode est_mode)
{
    double LogLikelihood = 0;
    CVector C;
    if (est_mode == estimation_mode::elemental_profile_and_contribution)
        C = PredictTarget(parameter_mode::based_on_fitted_distribution);
    else
        C = PredictTarget(parameter_mode::direct);
    CVector C_obs = ObservedDataforSelectedSample(selected_target_sample);

    if (C.min()>0)
        LogLikelihood -= log(error_stdev) + pow((C.Log()-C_obs.Log()).norm2(),2)/(2*pow(error_stdev,2));
    else
        LogLikelihood -= 1e10;

    return LogLikelihood;
}

CVector SourceSinkData::ResidualVector()
{
    CVector C = PredictTarget(parameter_mode::direct);
    if (!C.is_finite())
    {
        qDebug()<<"oops!";
    }
    CVector C_obs = ObservedDataforSelectedSample(selected_target_sample);
    CVector Residual = C.Log() - C_obs.Log();
    if (!Residual.is_finite())
    {
        CVector X = ContributionVector();
        CVector X_softmax = ContributionVector_softmax();
        qDebug()<<"oops!";
    }
    return Residual;
}

CVector_arma SourceSinkData::ResidualVector_arma()
{
    CVector_arma C = PredictTarget().vec;
    CVector_arma C_obs = ObservedDataforSelectedSample(selected_target_sample).vec;
    CVector_arma Residual = C.Log() - C_obs.Log();

    return Residual;
}

CMatrix_arma SourceSinkData::ResidualJacobian_arma()
{
    CMatrix_arma Jacobian(SourceOrder().size()-1,element_order.size());
    CVector_arma base_contribution = ContributionVector(false).vec;
    CVector_arma base_residual = ResidualVector_arma();
    for (unsigned int i = 0; i < SourceOrder().size()-1; i++)
    {
        double epsilon = (0.5 - base_contribution[i]) * 1e-6; 
        SetContribution(i, base_contribution[i] + epsilon);
        CVector_arma purturbed_residual = ResidualVector_arma();
        Jacobian.setcol(i,(purturbed_residual - base_residual) / epsilon);
        SetContribution(i, base_contribution[i]);
    }
    return Jacobian;
}

CMatrix SourceSinkData::ResidualJacobian()
{
    CMatrix Jacobian(SourceOrder().size()-1,element_order.size());
    CVector base_contribution = ContributionVector(false).vec;
    CVector base_residual = ResidualVector();
    for (unsigned int i = 0; i < SourceOrder().size()-1; i++)
    {
        double epsilon = (0.5 - base_contribution[i]) * 1e-3;
        SetContribution(i, base_contribution[i] + epsilon);
        CVector X = ContributionVector();
        CVector purturbed_residual = ResidualVector();
        Jacobian.setrow(i,(purturbed_residual - base_residual) / epsilon);
        SetContribution(i, base_contribution[i]);
    }
    return Jacobian;
}

CMatrix SourceSinkData::ResidualJacobian_softmax()
{
    CMatrix Jacobian(SourceOrder().size(),element_order.size());
    CVector base_contribution = ContributionVector_softmax().vec;
    CVector base_residual = ResidualVector();
    for (unsigned int i = 0; i < SourceOrder().size(); i++)
    {
        double epsilon = -sgn(base_contribution[i]) * 1e-3;

        CVector X = base_contribution;
        X[i]+=epsilon;
        SetContribution_softmax(X);
        CVector purturbed_residual = ResidualVector();
        Jacobian.setrow(i,(purturbed_residual - base_residual) / epsilon);
        SetContribution_softmax(base_contribution);
    }
    return Jacobian;
}

CVector SourceSinkData::OneStepLevenBerg_Marquardt(double lambda)
{

    CVector V = ResidualVector();
    CMatrix M = ResidualJacobian();
    CMatrix JTJ = M*Transpose(M);
    JTJ.ScaleDiagonal(1+lambda);
    CVector J_epsilon = M*V;

    if (det(JTJ)<=1e-6)
    {
        JTJ += lambda*CMatrix::Diag(JTJ.getnumcols());
    }

    CVector dx = J_epsilon/JTJ;
    return dx;
}

CVector SourceSinkData::OneStepLevenBerg_Marquardt_softmax(double lambda)
{

    CVector V = ResidualVector();
    CMatrix M = ResidualJacobian_softmax();
    CMatrix JTJ = M*Transpose(M);
    JTJ.ScaleDiagonal(1+lambda);
    CVector J_epsilon = M*V;

    if (det(JTJ)<=1e-6)
    {
        JTJ += lambda*CMatrix::Diag(JTJ.getnumcols());
    }
    CVector dx = J_epsilon / JTJ;

    return dx;
}


bool SourceSinkData::SolveLevenBerg_Marquardt(transformation trans)
{
    if (trans == transformation::linear)
        InitializeContributionsRandomly();
    else if (trans == transformation::softmax)
        InitializeContributionsRandomly_softmax();
    double err = 1000;
    double err_p;
    double tol = 1e-10;
    double lambda = 1;
    int counter = 0;
    double err_0 = ResidualVector().norm2();
    double err_x = 10000;
    double err_x0 = 10000;
    while (err>tol && err_x>tol && counter<1000)
    {
        CVector X0;
        if (trans == transformation::linear)
            X0 = ContributionVector(false);
        else if (trans == transformation::softmax)
            X0 = ContributionVector_softmax();

        err_p = err;
        CVector dx;

        if (trans == transformation::linear)
            dx = OneStepLevenBerg_Marquardt(lambda);
        else if (trans == transformation::softmax)
            dx = OneStepLevenBerg_Marquardt_softmax(lambda);

        if (dx.num == 0)
           lambda *= 5; 
        else
        {
            err_x = dx.norm2();
            if (counter == 0) err_x0 = err_x;
            CVector X = X0 - dx;
            if (trans == transformation::linear)
                SetContribution(X);
            else if (trans == transformation::softmax)
                SetContribution_softmax(X);

            CVector V = ResidualVector();
            err = V.norm2();
            if (err < err_p * 0.8)
            {
                lambda /= 1.2;
            }
            else if (err > err_p)
            {
                lambda *= 1.2;
                if (trans == transformation::linear)
                    SetContribution(X0);
                else if (trans == transformation::softmax)
                    SetContribution_softmax(X0);
                err = err_p;
            }
            if (rtw)
            {
                rtw->AppendPoint(counter, err);
                rtw->SetXRange(0, counter);
                rtw->SetProgress(1 - err_x / err_x0);
                QCoreApplication::processEvents();
            }
            
        }
        counter++;
    }

    return false;
}


CVector SourceSinkData::PredictTarget(parameter_mode param_mode)
{
    CVector C = SourceMeanMatrix(param_mode)*ContributionVector();
    return C;
}

double SourceSinkData::GetObjectiveFunctionValue()
{
    return -LogLikelihood(parameter_estimation_mode);
}

double SourceSinkData::LogLikelihood(estimation_mode est_mode)
{
    double YLogLikelihood = 0;
    double CLogLikelihood = 0;
    if (est_mode != estimation_mode::only_contributions)
        YLogLikelihood = LogLikelihoodSourceElementalDistributions();
    double LogPrior = LogPriorContributions();
    if (est_mode != estimation_mode::source_elemental_profiles_based_on_source_data)
        CLogLikelihood = LogLikelihoodModelvsMeasured(est_mode);
    return YLogLikelihood + CLogLikelihood + LogPrior;
}

CMatrix SourceSinkData::SourceMeanMatrix(parameter_mode param_mode)
{
    CMatrix Y(element_order.size(),numberofsourcesamplesets);
    for (unsigned int element_counter=0; element_counter<element_order.size(); element_counter++)
    {
        for (unsigned int source_group_counter=0; source_group_counter<numberofsourcesamplesets; source_group_counter++)
        {
            Elemental_Profile_Set *this_source_group = sample_set(samplesetsorder[source_group_counter]);
            if (param_mode==parameter_mode::based_on_fitted_distribution)
                Y[element_counter][source_group_counter] = this_source_group->GetEstimatedDistribution(element_order[element_counter])->Mean();
            else
                Y[element_counter][source_group_counter] = this_source_group->GetEstimatedDistribution(element_order[element_counter])->DataMean();
        }
    }
    return Y;
}

CVector SourceSinkData::ContributionVector(bool full)
{
    if (full)
    {   CVector X(numberofsourcesamplesets);
        for (unsigned int source_group_counter=0; source_group_counter<numberofsourcesamplesets; source_group_counter++)
        {
            Elemental_Profile_Set *this_source_group = sample_set(samplesetsorder[source_group_counter]);
            X[source_group_counter] = this_source_group->Contribution();
        }
        return X;
    }
    else
    {   CVector X(numberofsourcesamplesets-1);
        for (unsigned int source_group_counter=0; source_group_counter<numberofsourcesamplesets-1; source_group_counter++)
        {
            Elemental_Profile_Set *this_source_group = sample_set(samplesetsorder[source_group_counter]);
            X[source_group_counter] = this_source_group->Contribution();
        }
        return X;
    }
}

CVector SourceSinkData::ContributionVector_softmax()
{

   CVector X(numberofsourcesamplesets);
    for (unsigned int source_group_counter=0; source_group_counter<numberofsourcesamplesets; source_group_counter++)
    {
        Elemental_Profile_Set *this_source_group = sample_set(samplesetsorder[source_group_counter]);
        X[source_group_counter] = this_source_group->Contribution_softmax();
    }
    return X;

}





void SourceSinkData::SetContribution(int i, double value)
{
    sample_set(samplesetsorder[i])->SetContribution(value);
    sample_set(samplesetsorder[samplesetsorder.size()-1])->SetContribution(1-ContributionVector(false).sum());
}

void SourceSinkData::SetContribution_softmax(int i, double value)
{
    sample_set(samplesetsorder[i])->SetContribution_softmax(value);
}

void SourceSinkData::SetContribution(const CVector &X)
{
    for (int i=0; i<X.num; i++)
    {
        SetContribution(i,X.vec[i]);
    }
    if (X.min()<0)
    {
        qDebug()<<"oops!";
    }
}

void SourceSinkData::SetContribution_softmax(const CVector &X)
{
    double denominator = 0;
    for (int i=0; i<X.num; i++)
    {
        denominator += exp(X[i]);
    }
    for (int i=0; i<X.num; i++)
    {
        SetContribution_softmax(i,X.at(i));
        SetContribution(i,exp(X.at(i))/denominator);
    }
    CVector XX = ContributionVector();
    if (XX.min()<0)
    {
        qDebug()<<"oops!";
    }
}


Parameter* SourceSinkData::ElementalContent_mu(int element_iterator, int source_iterator)
{
    return &parameters[size()-2+element_iterator*numberofsourcesamplesets +source_iterator];
}
Parameter* SourceSinkData::ElementalContent_sigma(int element_iterator, int source_iterator)
{
    return &parameters[size()-2+element_iterator*numberofsourcesamplesets +source_iterator + numberofconstituents*numberofsourcesamplesets];
}

double SourceSinkData::ElementalContent_mu_value(int element_iterator, int source_iterator)
{
    return ElementalContent_mu(element_iterator,source_iterator)->Value();
}
double SourceSinkData::ElementalContent_sigma_value(int element_iterator, int source_iterator)
{
    return ElementalContent_sigma(element_iterator,source_iterator)->Value();
}

bool SourceSinkData::SetParameterValue(unsigned int i, double value)
{
    if (i<0 || i>parameters.size())
        return false;

    int est_mode_key_1=1;
    int est_mode_key_2=1;
    if (parameter_estimation_mode==estimation_mode::only_contributions)
        est_mode_key_2 = 0;
    if (parameter_estimation_mode==estimation_mode::source_elemental_profiles_based_on_source_data)
        est_mode_key_1 = 0;

    parameters[i].SetValue(value);
    if (i<est_mode_key_1*(numberofsourcesamplesets-1))
    {
        sample_set(samplesetsorder[i])->SetContribution(value);
        sample_set(samplesetsorder[numberofsourcesamplesets-1])->SetContribution(1-ContributionVector(false).sum());//+sample_set(samplesetsorder[numberofsourcesamplesets-1])->Contribution()
        return true;
    }
    else if (i<est_mode_key_1*(numberofsourcesamplesets-1)+numberofconstituents*numberofsourcesamplesets && parameter_estimation_mode==estimation_mode::elemental_profile_and_contribution)
    {
        int element_counter = (i-(numberofsourcesamplesets-1))/numberofsourcesamplesets;
        int group_counter = (i-(numberofsourcesamplesets-1))%numberofsourcesamplesets;
        GetElementDistribution(element_order[element_counter],samplesetsorder[group_counter])->SetEstimatedMu(value);
        return true;
    }
    else if (i<est_mode_key_1*(numberofsourcesamplesets-1)+2*numberofconstituents*numberofsourcesamplesets && parameter_estimation_mode==estimation_mode::elemental_profile_and_contribution)
    {
        int element_counter = (i-(numberofsourcesamplesets-1)-numberofconstituents*numberofsourcesamplesets)/numberofsourcesamplesets;
        int group_counter = (i-(numberofsourcesamplesets-1)-numberofconstituents*numberofsourcesamplesets)%numberofsourcesamplesets;
        GetElementDistribution(element_order[element_counter],samplesetsorder[group_counter])->SetEstimatedSigma(value);
        return true;
    }
    else if (i==est_mode_key_1*(numberofsourcesamplesets-1)+2*numberofconstituents*numberofsourcesamplesets*est_mode_key_2)
    {
        error_stdev = value;
        return true;
    }

    return false;
}


vector<string> SourceSinkData::ElementsToBeUsedInCMB()
{
    numberofconstituents = 0;
    constituent_order.clear();
    element_order.clear();
    size_om_order.clear();
    isotope_order.clear();

    for (map<string, element_information>::iterator it = ElementInformation.begin(); it!=ElementInformation.end(); it++)
        if (it->second.Role == element_information::role::element)
        {   numberofconstituents ++;
            constituent_order.push_back(it->first);
        }
    return constituent_order;
}

void SourceSinkData::populate_constituent_orders()
{
    numberofconstituents = 0;
    constituent_order.clear();
    element_order.clear();
    size_om_order.clear();
    isotope_order.clear();

    for (map<string, element_information>::iterator it = ElementInformation.begin(); it!=ElementInformation.end(); it++)
        {   numberofconstituents ++;
            constituent_order.push_back(it->first);
        }
    for (map<string, element_information>::iterator it = ElementInformation.begin(); it!=ElementInformation.end(); it++)
        if (it->second.Role == element_information::role::element)
        {
            element_order.push_back(it->first);
        }
    for (map<string, element_information>::iterator it = ElementInformation.begin(); it!=ElementInformation.end(); it++)
        if (it->second.Role == element_information::role::isotope)
        {
            isotope_order.push_back(it->first);
        }
    for (map<string, element_information>::iterator it = ElementInformation.begin(); it!=ElementInformation.end(); it++)
        if (it->second.Role == element_information::role::particle_size)
        {
            size_om_order.push_back(it->first);
        }

}

vector<string> SourceSinkData::SourceGroupNames()
{
    vector<string> sourcegroups;
    for (map<string,Elemental_Profile_Set>::iterator source_iterator = begin(); source_iterator!=end(); source_iterator++)
    {
        if (source_iterator->first!=target_group)
            sourcegroups.push_back(source_iterator->first);
    }
    return sourcegroups;
}

bool SourceSinkData::SetSelectedTargetSample(const string &sample_name)
{
    if (sample_set(TargetGroup())->Profile(sample_name))
    {
        selected_target_sample = sample_name;
        return true;
    }
    return false;
}
string SourceSinkData::SelectedTargetSample()
{
    return selected_target_sample;
}

Elemental_Profile *SourceSinkData::GetElementalProfile(const string sample_name)
{

    for (map<string,Elemental_Profile_Set>::const_iterator group=begin(); group!=end(); group++ )
    {
        for (map<string,Elemental_Profile>::const_iterator sample=group->second.cbegin(); sample!=group->second.cend(); sample++)
            if (sample->first == sample_name)
                return sample_set(group->first)->Profile(sample->first);
    }
    return nullptr;
}

result_item SourceSinkData::GetContribution()
{
    result_item result_cont;
    Contribution *contribution = new Contribution();
    for (int i=0; i<SourceOrder().size(); i++)
    {
        contribution->operator[](SourceOrder()[i]) = ContributionVector()[i];
    }
    result_cont.name = "Contributions";
    result_cont.result = contribution;
    result_cont.type = result_type::contribution;

    return  result_cont;
}
result_item SourceSinkData::GetPredictedElementalProfile(parameter_mode param_mode)
{
    result_item result_modeled;

    Elemental_Profile *modeled_profile = new Elemental_Profile();
    CVector predicted_profile = PredictTarget(param_mode);
    vector<string> element_names = ElementOrder();
    for (int i=0; i<element_names.size(); i++)
    {
        modeled_profile->AppendElement(element_names[i],predicted_profile[i]);
    }
    result_modeled.name = "Modeled Elemental Profile";
    result_modeled.result = modeled_profile;
    result_modeled.type = result_type::predicted_concentration;
    return result_modeled;
}

result_item SourceSinkData::GetObservedvsModeledElementalProfile(parameter_mode param_mode)
{
    result_item result_obs;

    Elemental_Profile* predicted = static_cast<Elemental_Profile*>(GetPredictedElementalProfile(param_mode).result);
    Elemental_Profile* observed = static_cast<Elemental_Profile*>(GetObservedElementalProfile().result);
    Elemental_Profile_Set* modeled_vs_observed = new Elemental_Profile_Set(); 
    modeled_vs_observed->Append_Profile("Observed", *observed);
    modeled_vs_observed->Append_Profile("Modeled", *predicted);
    
    result_obs.name = "Observed vs Modeled Elemental Profile";
    result_obs.result = modeled_vs_observed;
    result_obs.type = result_type::elemental_profile_set;
    return result_obs;
}

result_item SourceSinkData::GetObservedElementalProfile()
{
    result_item result_obs;

    Elemental_Profile* obs_profile = new Elemental_Profile();
    CVector observed_profile = ObservedDataforSelectedSample(selected_target_sample);
    vector<string> element_names = ElementOrder();
    for (int i = 0; i < element_names.size(); i++)
    {
        obs_profile->AppendElement(element_names[i], observed_profile[i]);
    }
    CVector modeled_profile = ObservedDataforSelectedSample(selected_target_sample);
    
    for (int i = 0; i < element_names.size(); i++)
    {
        obs_profile->AppendElement(element_names[i], observed_profile[i]);
    }
    result_obs.name = "Observed Elemental Profile";
    result_obs.result = obs_profile;
    result_obs.type = result_type::predicted_concentration;
    return result_obs;
}

result_item SourceSinkData::GetCalculatedElementMeans()
{
    Elemental_Profile_Set *profile_set = new Elemental_Profile_Set();
    for (map<string,Elemental_Profile_Set>::iterator it=begin(); it!=end(); it++ )
    { 
        if (it->first != target_group)
        {
            Elemental_Profile element_profile;
            for (unsigned int element_counter = 0; element_counter < element_order.size(); element_counter++)
            {
                element_profile.AppendElement(element_order[element_counter], it->second.ElementalDistribution(element_order[element_counter])->mean());
            }
            profile_set->Append_Profile(it->first, element_profile);
        }
    }
    result_item resitem;
    resitem.name = "Calculated mean elemental contents";
    resitem.type = result_type::elemental_profile_set;
    resitem.result = profile_set;
    return resitem;
    
}
result_item SourceSinkData::GetCalculatedElementStandardDev()
{
    Elemental_Profile_Set* profile_set = new Elemental_Profile_Set();
    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != end(); it++)
    {
        if (it->first != target_group)
        {
            Elemental_Profile element_profile;
            for (unsigned int element_counter = 0; element_counter < element_order.size(); element_counter++)
            {
                if (it->second.ElementalDistribution(element_order[element_counter])->FittedDistribution()->distribution == distribution_type::normal)
                    element_profile.AppendElement(element_order[element_counter], it->second.ElementalDistribution(element_order[element_counter])->stdev());
                else if (it->second.ElementalDistribution(element_order[element_counter])->FittedDistribution()->distribution == distribution_type::lognormal)
                    element_profile.AppendElement(element_order[element_counter], it->second.ElementalDistribution(element_order[element_counter])->stdevln());
            }
            profile_set->Append_Profile(it->first, element_profile);
        }
    }
    result_item resitem;
    resitem.name = "Calculated elemental contents standard deviations";
    resitem.type = result_type::elemental_profile_set;
    resitem.result = profile_set;
    return resitem;
}
result_item SourceSinkData::GetCalculatedElementMu()
{
    Elemental_Profile_Set* profile_set = new Elemental_Profile_Set();
    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != end(); it++)
    {
        if (it->first != target_group)
        {
            Elemental_Profile element_profile;
            for (unsigned int element_counter = 0; element_counter < element_order.size(); element_counter++)
            {
                element_profile.AppendElement(element_order[element_counter], it->second.ElementalDistribution(element_order[element_counter])->meanln());
            }
            profile_set->Append_Profile(it->first, element_profile);
        }
    }
    result_item resitem;
    resitem.name = "Calculated geometrical mean of elemental contents";
    resitem.type = result_type::elemental_profile_set;
    resitem.result = profile_set;
    return resitem;
}
result_item SourceSinkData::GetEstimatedElementMu()
{
    Elemental_Profile_Set* profile_set = new Elemental_Profile_Set();
    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != end(); it++)
    {
        if (it->first != target_group)
        {
            Elemental_Profile element_profile;
            for (unsigned int element_counter = 0; element_counter < element_order.size(); element_counter++)
            {
                element_profile.AppendElement(element_order[element_counter], it->second.ElementalDistribution(element_order[element_counter])->EstimatedMu());
            }
            profile_set->Append_Profile(it->first, element_profile);
        }
    }
    result_item resitem;
    resitem.name = "Infered geometrical mean of elemental contents";
    resitem.type = result_type::elemental_profile_set;
    resitem.result = profile_set;
    return resitem;
}

result_item SourceSinkData::GetEstimatedElementMean()
{
    Elemental_Profile_Set* profile_set = new Elemental_Profile_Set();
    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != end(); it++)
    {
        if (it->first != target_group)
        {
            Elemental_Profile element_profile;
            for (unsigned int element_counter = 0; element_counter < element_order.size(); element_counter++)
            {
                double sigma = it->second.ElementalDistribution(element_order[element_counter])->EstimatedSigma();
                double mu = it->second.ElementalDistribution(element_order[element_counter])->EstimatedMu();
                element_profile.AppendElement(element_order[element_counter], exp(mu+pow(sigma,2)/2));
            }
            profile_set->Append_Profile(it->first, element_profile);
        }
    }
    result_item resitem;
    resitem.name = "Infered mean of elemental contents";
    resitem.type = result_type::elemental_profile_set;
    resitem.result = profile_set;
    return resitem;
}

result_item SourceSinkData::GetEstimatedElementSigma()
{
    Elemental_Profile_Set* profile_set = new Elemental_Profile_Set();
    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != end(); it++)
    {
        if (it->first != target_group)
        {
            Elemental_Profile element_profile;
            for (unsigned int element_counter = 0; element_counter < element_order.size(); element_counter++)
            {
                element_profile.AppendElement(element_order[element_counter], it->second.ElementalDistribution(element_order[element_counter])->EstimatedSigma());
            }
            profile_set->Append_Profile(it->first, element_profile);
        }
    }
    result_item resitem;
    resitem.name = "Infered sigma elemental contents";
    resitem.type = result_type::elemental_profile_set;
    resitem.result = profile_set;
    return resitem;
}

bool SourceSinkData::WriteElementInformationToFile(QFile *file)
{
    file->write("***");
    file->write("Element Information");
    for (map<string,element_information>::iterator it = ElementInformation.begin(); it!=ElementInformation.end(); it++)
        file->write(QString::fromStdString(it->first).toUtf8()+ "\t" + Role(it->second.Role).toUtf8());
    file->write("***");
    return true;
}

QString SourceSinkData::Role(const element_information::role &rl)
{
    if (rl == element_information::role::do_not_include)
        return "DoNotInclude";
    else if (rl == element_information::role::element)
        return "Element";
    else if (rl == element_information::role::isotope)
        return "Isotope";
    else if (rl == element_information::role::particle_size)
        return "ParticleSize";
    return "DoNotInclude";
}

element_information::role SourceSinkData::Role(const QString &rl)
{
    if (rl == "DoNotInclude")
        return element_information::role::do_not_include;
    else if (rl == "Element")
        return element_information::role::element;
    else if (rl == "Isotope")
        return element_information::role::isotope;
    else if (rl == "ParticleSize")
        return element_information::role::particle_size;
    return element_information::role::do_not_include;
}
