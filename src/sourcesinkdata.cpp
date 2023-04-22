#include "sourcesinkdata.h"
#include "iostream"
#include "NormalDist.h"
#include "qjsondocument.h"
#include "resultitem.h"



SourceSinkData::SourceSinkData():map<string, Elemental_Profile_Set>()
{

}

SourceSinkData::SourceSinkData(const SourceSinkData& mp):map<string, Elemental_Profile_Set>(mp)
{

    ElementInformation = mp.ElementInformation;
    element_distributions = mp.element_distributions;
    numberofconstituents = mp.numberofconstituents;
    numberofisotopes = mp.numberofisotopes;
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
    omconstituent = mp.omconstituent;
    sizeconsituent = mp.sizeconsituent;

}

SourceSinkData SourceSinkData::Corrected(const string &target, bool omnsizecorrect, map<string, element_information> *elementinfo)
{
    SourceSinkData out;
    selected_target_sample = target;
    for (map<string, Elemental_Profile_Set>::iterator it=begin(); it!=end(); it++)
    {
        if (it->first!=target_group)
        {
            double om = at(target_group)[selected_target_sample][omconstituent];
            double size = at(target_group)[selected_target_sample][sizeconsituent];
            out[it->first] = it->second.CopyIncludedinAnalysis(omnsizecorrect,om,size);
        }
        else
            out[it->first] = it->second;
    }
    out.omconstituent = omconstituent;
    out.sizeconsituent = sizeconsituent;
    if (elementinfo)
    for (map<string,element_information>::iterator it = ElementInformation.begin(); it!=ElementInformation.end(); it++)
    {
        if (it->second.include_in_analysis && it->second.Role!=element_information::role::do_not_include)
        {
            out.ElementInformation[it->first] = ElementInformation[it->first];
            out.element_distributions[it->first] = element_distributions[it->first];
        }
    }
    else
    {
        out.ElementInformation = ElementInformation;
        out.element_distributions = element_distributions;
    }
    out.numberofconstituents = numberofconstituents;
    out.numberofisotopes = numberofisotopes;
    out.numberofsourcesamplesets = numberofsourcesamplesets;
    out.observations = observations;
    out.outputpath = outputpath;
    out.target_group = target_group;
    out.samplesetsorder = samplesetsorder;
    out.constituent_order = constituent_order;
    out.selected_target_sample = selected_target_sample;
    out.element_order = element_order;
    out.isotope_order = isotope_order;
    out.size_om_order = size_om_order;
    out.parameter_estimation_mode = parameter_estimation_mode;

    //out.PopulateElementInformation();
    out.PopulateElementDistributions();
    out.AssignAllDistributions();
    return out;
}

SourceSinkData SourceSinkData::CopyandCorrect(bool exclude_samples, bool exclude_elements, bool omnsizecorrect, const string &target) const
{
    SourceSinkData out;
    out.target_group = target_group;
    if (target!="")
        out.selected_target_sample = target;
    double om,psize;
    if (omnsizecorrect)
    {
        om = at(target_group).at(selected_target_sample).at(omconstituent);
        psize = at(target_group).at(selected_target_sample).at(sizeconsituent);
    }
    for (map<string, Elemental_Profile_Set>::const_iterator it=cbegin(); it!=cend(); it++)
    {
        if (it->first!=target_group)
        {
            out[it->first] = it->second.CopyandCorrect(false,exclude_elements,false, om, psize, &ElementInformation);
        }
        else
            out[it->first] = it->second.CopyandCorrect(exclude_samples,exclude_elements,omnsizecorrect,om,psize,&ElementInformation);
    }
    out.PopulateElementInformation();
    out.PopulateElementDistributions();
    out.AssignAllDistributions();
    return out;
}

SourceSinkData& SourceSinkData::operator=(const SourceSinkData &mp)
{
    map<string, Elemental_Profile_Set>::operator=(mp);
    ElementInformation = mp.ElementInformation;
    element_distributions = mp.element_distributions;
    numberofconstituents = mp.numberofconstituents;
    numberofisotopes = mp.numberofisotopes;
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
    omconstituent = mp.omconstituent;
    sizeconsituent = mp.sizeconsituent;

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
    element_distributions.clear();
    for (map<string,Elemental_Profile_Set>::iterator it=begin(); it!=end(); it++)
    {
        it->second.UpdateElementalDistribution();
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
    observations.clear();
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
        if (it->second.Role == element_information::role::element && it->second.include_in_analysis)
            numberofconstituents ++;

    // Count the number of isotopes to be used
    numberofisotopes = 0;
        for (map<string, element_information>::iterator it = ElementInformation.begin(); it!=ElementInformation.end(); it++)
            if (it->second.Role == element_information::role::isotope && it->second.include_in_analysis)
                numberofisotopes ++;


    // Copying Estimated Distribution from the Fitted Distribution for elements
    for (map<string, element_information>::iterator element_iterator = ElementInformation.begin(); element_iterator!=ElementInformation.end(); element_iterator++)
    {
        if (element_iterator->second.Role == element_information::role::element && element_iterator->second.include_in_analysis)
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

    // Copying Estimated Distribution from the Fitted Distribution for isotopes
    for (map<string, element_information>::iterator isotope_iterator = ElementInformation.begin(); isotope_iterator!=ElementInformation.end(); isotope_iterator++)
    {
        if (isotope_iterator->second.Role == element_information::role::isotope && isotope_iterator->second.include_in_analysis)
        {
            for (map<string,Elemental_Profile_Set>::iterator source_iterator = begin(); source_iterator!=end(); source_iterator++)
            {
                if (source_iterator->first!=target_group)
                {
                    *source_iterator->second.GetEstimatedDistribution(isotope_iterator->first) = *source_iterator->second.GetFittedDistribution(isotope_iterator->first);
                }
            }
        }
    }


    // Source elemental profile geometrical mean for each element
    if (est_mode != estimation_mode::only_contributions)
    {   for (map<string, element_information>::iterator element_iterator = ElementInformation.begin(); element_iterator!=ElementInformation.end(); element_iterator++)
        {
            if (element_iterator->second.Role == element_information::role::element && element_iterator->second.include_in_analysis)
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

        for (map<string, element_information>::iterator element_iterator = ElementInformation.begin(); element_iterator!=ElementInformation.end(); element_iterator++)
        {
            if (element_iterator->second.Role == element_information::role::isotope && element_iterator->second.include_in_analysis)
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
            if (element_iterator->second.Role == element_information::role::element && element_iterator->second.include_in_analysis)
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
        // Source elemental profile standard deviation for each isotope
        for (map<string, element_information>::iterator element_iterator = ElementInformation.begin(); element_iterator!=ElementInformation.end(); element_iterator++)
        {
            if (element_iterator->second.Role == element_information::role::isotope && element_iterator->second.include_in_analysis)
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
        p.SetRange(0.01, 0.1);
        parameters.push_back(p);


        // Observations
        for (map<string, element_information>::iterator element_iterator = ElementInformation.begin(); element_iterator!=ElementInformation.end(); element_iterator++)
        {
            if (element_iterator->second.Role == element_information::role::element && element_iterator->second.include_in_analysis)
            {
                Observation obs;
                obs.SetName(targetsamplename + "_" + element_iterator->first);
                obs.AppendValues(0,GetElementalProfile(targetsamplename)->Val(element_iterator->first));
                observations.push_back(obs);
            }

        }
    }

    // Error Standard Deviation for isotopes
    if (est_mode != estimation_mode::source_elemental_profiles_based_on_source_data)
    {   Parameter p;
        p.SetName("Error STDev for isotopes");
        p.SetPriorDistribution(distribution_type::lognormal);
        p.SetRange(0.01, 0.1);
        parameters.push_back(p);


        // Observations
        for (map<string, element_information>::iterator element_iterator = ElementInformation.begin(); element_iterator!=ElementInformation.end(); element_iterator++)
        {
            if (element_iterator->second.Role == element_information::role::isotope && element_iterator->second.include_in_analysis)
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

CVector SourceSinkData::ObservedDataforSelectedSample_Isotope(const string &SelectedTargetSample)
{
    CVector observed_data(isotope_order.size());
    for (unsigned int i=0; i<isotope_order.size(); i++)
    {   string corresponding_element = ElementInformation[isotope_order[i]].base_element;
        if (SelectedTargetSample!="")
            observed_data[i] = (this->sample_set(TargetGroup())->Profile(SelectedTargetSample)->Val(isotope_order[i])/double(1000)+1.0)*ElementInformation[isotope_order[i]].standard_ratio*sample_set(TargetGroup())->Profile(SelectedTargetSample)->Val(corresponding_element);
        else if (selected_target_sample!="")
            observed_data[i] = (this->sample_set(TargetGroup())->Profile(selected_target_sample)->Val(isotope_order[i])/double(1000)+1.0)*ElementInformation[isotope_order[i]].standard_ratio*sample_set(TargetGroup())->Profile(SelectedTargetSample)->Val(corresponding_element);
    }
    return observed_data;
}

CVector SourceSinkData::ObservedDataforSelectedSample_Isotope_delta(const string &SelectedTargetSample)
{
    CVector observed_data(isotope_order.size());
    for (unsigned int i=0; i<isotope_order.size(); i++)
    {   string corresponding_element = ElementInformation[isotope_order[i]].base_element;
        if (SelectedTargetSample!="")
            observed_data[i] = this->sample_set(TargetGroup())->Profile(SelectedTargetSample)->Val(isotope_order[i]);
        else if (selected_target_sample!="")
            observed_data[i] = this->sample_set(TargetGroup())->Profile(selected_target_sample)->Val(isotope_order[i]);
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
        LogLikelihood -= C.num*log(error_stdev) + pow((C.Log()-C_obs.Log()).norm2(),2)/(2*pow(error_stdev,2));
    else
        LogLikelihood -= 1e10;


    return LogLikelihood;
}

double SourceSinkData::LogLikelihoodModelvsMeasured_Isotope(estimation_mode est_mode)
{
    double LogLikelihood = 0;
    CVector C;
    if (est_mode == estimation_mode::elemental_profile_and_contribution)
        C = PredictTarget_Isotope(parameter_mode::based_on_fitted_distribution);
    else
        C = PredictTarget_Isotope(parameter_mode::direct);
    CVector C_obs = ObservedDataforSelectedSample(selected_target_sample);

    if (C.min()>0)
        LogLikelihood -= C.num*log(error_stdev) + pow((C.Log()-C_obs.Log()).norm2(),2)/(2*pow(error_stdev,2));
    else
        LogLikelihood -= 1e10;

    return LogLikelihood;
}


CVector SourceSinkData::ResidualVector()
{
    CVector C = PredictTarget(parameter_mode::direct);
    CVector C_iso = PredictTarget_Isotope_delta(parameter_mode::direct);
    if (!C.is_finite())
    {
        qDebug()<<"oops!";
    }
    CVector C_obs = ObservedDataforSelectedSample(selected_target_sample);
    CVector C_obs_iso = ObservedDataforSelectedSample_Isotope_delta(selected_target_sample);
    CVector Residual = C.Log() - C_obs.Log();
    CVector Residual_isotope = C_iso - C_obs_iso;
    Residual.append(Residual_isotope);
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
    CVector_arma C_iso = PredictTarget_Isotope_delta().vec;
    CVector_arma C_obs = ObservedDataforSelectedSample(selected_target_sample).vec;
    CVector_arma C_obs_iso = ObservedDataforSelectedSample_Isotope_delta(selected_target_sample).vec;
    CVector_arma Residual = C.Log() - C_obs.Log();
    CVector_arma Residual_iso = C_iso-C_obs;
    Residual.append(Residual_iso);

    return Residual;
}

CMatrix_arma SourceSinkData::ResidualJacobian_arma()
{
    CMatrix_arma Jacobian(SourceOrder().size()-1,element_order.size()+isotope_order.size());
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
    CMatrix Jacobian(SourceOrder().size()-1,element_order.size()+isotope_order.size());
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
    CMatrix Jacobian(SourceOrder().size(),element_order.size()+isotope_order.size());
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
    for (unsigned int i=0; i<element_order.size(); i++)
    {
        observation(i)->SetPredictedValue(C[i]);
    }
    return C;
}

CVector SourceSinkData::PredictTarget_Isotope(parameter_mode param_mode)
{
    CVector C = SourceMeanMatrix_Isotopes(param_mode)*ContributionVector();
    return C;
}

CVector SourceSinkData::PredictTarget_Isotope_delta(parameter_mode param_mode)
{
    CVector C_elements = SourceMeanMatrix(param_mode)*ContributionVector();
    CVector C = SourceMeanMatrix_Isotopes(param_mode)*ContributionVector();
    for (unsigned int i=0; i<numberofisotopes; i++)
    {
        string corresponding_element = ElementInformation[isotope_order[i]].base_element;
        double predicted_corresponding_element_concentration = C_elements[lookup(element_order,corresponding_element)];
        C[i] = (C[i]/predicted_corresponding_element_concentration/ElementInformation[isotope_order[i]].standard_ratio-1.0)*1000.0;
    }
    for (unsigned int i=element_order.size(); i<element_order.size()+isotope_order.size(); i++)
    {
        observation(i)->SetPredictedValue(C[i-element_order.size()]);
    }
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

CMatrix SourceSinkData::SourceMeanMatrix_Isotopes(parameter_mode param_mode)
{
    CMatrix Y(isotope_order.size(),numberofsourcesamplesets);
    for (unsigned int isotope_counter=0; isotope_counter<isotope_order.size(); isotope_counter++)
    {
        for (unsigned int source_group_counter=0; source_group_counter<numberofsourcesamplesets; source_group_counter++)
        {
            string isotope = isotope_order[isotope_counter];
            string corresponding_element = ElementInformation[isotope].base_element;
            Elemental_Profile_Set *this_source_group = sample_set(samplesetsorder[source_group_counter]);

            if (param_mode==parameter_mode::based_on_fitted_distribution)
            {
                double mean_delta = this_source_group->GetEstimatedDistribution(isotope_order[isotope_counter])->Mean();
                double standard_ratio = ElementInformation[isotope].standard_ratio;
                double corresponding_element_content = this_source_group->GetEstimatedDistribution(corresponding_element)->Mean();
                Y[isotope_counter][source_group_counter] = (mean_delta/1000.0+1.0)*standard_ratio*corresponding_element_content;

            }
            else
            {
                double mean_delta = this_source_group->GetEstimatedDistribution(isotope_order[isotope_counter])->DataMean();
                double standard_ratio = ElementInformation[isotope].standard_ratio;
                double corresponding_element_content = this_source_group->GetEstimatedDistribution(corresponding_element)->DataMean();
                Y[isotope_counter][source_group_counter] = (mean_delta/1000.0+1.0)*standard_ratio*corresponding_element_content;

            }
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
    //Contributions
    if (i<est_mode_key_1*(numberofsourcesamplesets-1))
    {
        sample_set(samplesetsorder[i])->SetContribution(value);
        sample_set(samplesetsorder[numberofsourcesamplesets-1])->SetContribution(1-ContributionVector(false).sum());//+sample_set(samplesetsorder[numberofsourcesamplesets-1])->Contribution()
        return true;
    }
    //Element means
    else if (i<est_mode_key_1*(numberofsourcesamplesets-1)+numberofconstituents*numberofsourcesamplesets && parameter_estimation_mode==estimation_mode::elemental_profile_and_contribution)
    {
        int element_counter = (i-(numberofsourcesamplesets-1))/numberofsourcesamplesets;
        int group_counter = (i-(numberofsourcesamplesets-1))%numberofsourcesamplesets;
        GetElementDistribution(element_order[element_counter],samplesetsorder[group_counter])->SetEstimatedMu(value);
        return true;
    }
    //Isotopes means
    else if (i<est_mode_key_1*(numberofsourcesamplesets-1)+(numberofconstituents+numberofisotopes)*numberofsourcesamplesets && parameter_estimation_mode==estimation_mode::elemental_profile_and_contribution)
    {
        int isotope_counter = (i-(numberofsourcesamplesets-1)-numberofconstituents*numberofsourcesamplesets)/numberofsourcesamplesets;
        int group_counter = (i-(numberofsourcesamplesets-1-numberofconstituents*numberofsourcesamplesets))%numberofsourcesamplesets;
        GetElementDistribution(element_order[isotope_counter],samplesetsorder[group_counter])->SetEstimatedMu(value);
        return true;
    }

    //Element standard deviations
    else if (i<est_mode_key_1*(numberofsourcesamplesets-1)+(2*numberofconstituents+numberofisotopes)*numberofsourcesamplesets && parameter_estimation_mode==estimation_mode::elemental_profile_and_contribution)
    {
        if (value<0)
        {
            cout<<"stop!"<<std::endl;
        }
        int element_counter = (i-(numberofsourcesamplesets-1)-(numberofconstituents+numberofisotopes)*numberofsourcesamplesets)/numberofsourcesamplesets;
        int group_counter = (i-(numberofsourcesamplesets-1)-(numberofconstituents+numberofisotopes)*numberofsourcesamplesets)%numberofsourcesamplesets;
        GetElementDistribution(element_order[element_counter],samplesetsorder[group_counter])->SetEstimatedSigma(value);
        return true;
    }
    //Isotope standard deviations
    else if (i<est_mode_key_1*(numberofsourcesamplesets-1)+(2*numberofconstituents+2*numberofisotopes)*numberofsourcesamplesets && parameter_estimation_mode==estimation_mode::elemental_profile_and_contribution)
    {
        if (value<0)
        {
            cout<<"stop!"<<std::endl;
        }
        int isotope_counter = (i-(numberofsourcesamplesets-1)-(2*numberofconstituents+numberofisotopes)*numberofsourcesamplesets)/numberofsourcesamplesets;
        int group_counter = (i-(numberofsourcesamplesets-1)-(2*numberofconstituents+numberofisotopes)*numberofsourcesamplesets)%numberofsourcesamplesets;
        GetElementDistribution(element_order[isotope_counter],samplesetsorder[group_counter])->SetEstimatedSigma(value);
        return true;
    }

    //Observed standard deviation
    else if (i==est_mode_key_1*(numberofsourcesamplesets-1)+2*(numberofconstituents+numberofisotopes)*numberofsourcesamplesets*est_mode_key_2)
    {
        if (value<0)
        {
            cout<<"stop!"<<std::endl;
        }
        error_stdev = value;
        return true;
    }
    //Observed isotope standard deviation
    else if (i==est_mode_key_1*(numberofsourcesamplesets-1)+2*(numberofconstituents+numberofisotopes)*numberofsourcesamplesets*est_mode_key_2+1)
    {
        if (value<0)
        {
            cout<<"stop!"<<std::endl;
        }
        error_stdev_isotope = value;
        return true;
    }

    return false;
}


vector<string> SourceSinkData::ElementsToBeUsedInCMB()
{
    numberofconstituents = 0;
    constituent_order.clear();

    for (map<string, element_information>::iterator it = ElementInformation.begin(); it!=ElementInformation.end(); it++)
        if (it->second.Role == element_information::role::element)
        {   numberofconstituents ++;
            constituent_order.push_back(it->first);
        }
    return constituent_order;
}


vector<string> SourceSinkData::IsotopesToBeUsedInCMB()
{
    numberofisotopes = 0;
    isotope_order.clear();

    for (map<string, element_information>::iterator it = ElementInformation.begin(); it!=ElementInformation.end(); it++)
        if (it->second.Role == element_information::role::isotope && it->second.include_in_analysis)
        {   numberofisotopes ++;
            isotope_order.push_back(it->first);
        }
    return isotope_order;
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
        if (it->second.Role == element_information::role::element && it->second.include_in_analysis)
        {
            element_order.push_back(it->first);
        }
    for (map<string, element_information>::iterator it = ElementInformation.begin(); it!=ElementInformation.end(); it++)
        if (it->second.Role == element_information::role::isotope && it->second.include_in_analysis)
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

ResultItem SourceSinkData::GetContribution()
{
    ResultItem result_cont;
    Contribution *contribution = new Contribution();
    for (int i=0; i<SourceOrder().size(); i++)
    {
        contribution->operator[](SourceOrder()[i]) = ContributionVector()[i];
    }
    result_cont.SetName("Contributions");
    result_cont.SetResult(contribution);
    result_cont.SetType(result_type::contribution);

    return  result_cont;
}
ResultItem SourceSinkData::GetPredictedElementalProfile(parameter_mode param_mode)
{
    ResultItem result_modeled;

    Elemental_Profile *modeled_profile = new Elemental_Profile();
    CVector predicted_profile = PredictTarget(param_mode);
    vector<string> element_names = ElementOrder();
    for (int i=0; i<element_names.size(); i++)
    {
        modeled_profile->AppendElement(element_names[i],predicted_profile[i]);
    }
    result_modeled.SetName("Modeled Elemental Profile");
    result_modeled.SetResult(modeled_profile);
    result_modeled.SetType(result_type::predicted_concentration);
    return result_modeled;
}

CVector SourceSinkData::GetPredictedValues()
{
    CVector out(ObservationsCount());
    for (unsigned int i=0; i<ObservationsCount(); i++)
    {
        out[i] = observation(i)->PredictedValue();
    }
    return out;
}

ResultItem SourceSinkData::GetPredictedElementalProfile_Isotope(parameter_mode param_mode)
{
    ResultItem result_modeled;

    Elemental_Profile *modeled_profile = new Elemental_Profile();
    CVector predicted_profile = PredictTarget_Isotope_delta(param_mode);
    vector<string> isotope_names = IsotopeOrder();
    for (unsigned int i=0; i<isotope_names.size(); i++)
    {
        modeled_profile->AppendElement(isotope_names[i],predicted_profile[i]);
    }
    result_modeled.SetName("Modeled Elemental Profile for Isotopes");
    result_modeled.SetResult(modeled_profile);
    result_modeled.SetType(result_type::predicted_concentration);
    return result_modeled;
}

ResultItem SourceSinkData::GetObservedvsModeledElementalProfile(parameter_mode param_mode)
{
    ResultItem result_obs;

    Elemental_Profile* predicted = static_cast<Elemental_Profile*>(GetPredictedElementalProfile(param_mode).Result());
    Elemental_Profile* observed = static_cast<Elemental_Profile*>(GetObservedElementalProfile().Result());
    Elemental_Profile_Set* modeled_vs_observed = new Elemental_Profile_Set(); 
    modeled_vs_observed->Append_Profile("Observed", *observed);
    modeled_vs_observed->Append_Profile("Modeled", *predicted);
    
    result_obs.SetName("Observed vs Modeled Elemental Profile");
    result_obs.SetResult(modeled_vs_observed);
    result_obs.SetType(result_type::elemental_profile_set);
    return result_obs;
}

vector<ResultItem> SourceSinkData::GetMLRResults()
{
    vector<ResultItem> out;
    for (map<string,Elemental_Profile_Set>::iterator it=begin(); it!=end(); it++ )
    {
        ResultItem profile_result = it->second.GetRegressions();
        profile_result.SetShowTable(true);
        profile_result.SetName("OM & Size MLR for " + it->first);
        out.push_back(profile_result);
    }
    return out;
}

ResultItem SourceSinkData::GetObservedvsModeledElementalProfile_Isotope(parameter_mode param_mode)
{
    ResultItem result_obs;

    Elemental_Profile* predicted = static_cast<Elemental_Profile*>(GetPredictedElementalProfile_Isotope(param_mode).Result());
    Elemental_Profile* observed = static_cast<Elemental_Profile*>(GetObservedElementalProfile_Isotope().Result());
    Elemental_Profile_Set* modeled_vs_observed = new Elemental_Profile_Set();
    modeled_vs_observed->Append_Profile("Observed", *observed);
    modeled_vs_observed->Append_Profile("Modeled", *predicted);

    result_obs.SetName("Observed vs Modeled Elemental Profile for Isotopes");
    result_obs.SetResult(modeled_vs_observed);
    result_obs.SetType(result_type::elemental_profile_set);
    return result_obs;
}

ResultItem SourceSinkData::GetObservedElementalProfile()
{
    ResultItem result_obs;

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
    result_obs.SetName("Observed Elemental Profile");
    result_obs.SetResult(obs_profile);
    result_obs.SetType(result_type::predicted_concentration);
    return result_obs;
}

ResultItem SourceSinkData::GetObservedElementalProfile_Isotope()
{
    ResultItem result_obs;

    Elemental_Profile* obs_profile = new Elemental_Profile();
    CVector observed_profile = ObservedDataforSelectedSample_Isotope_delta(selected_target_sample);
    vector<string> isotope_names = IsotopeOrder();
    for (unsigned int i = 0; i < isotope_names.size(); i++)
    {
        obs_profile->AppendElement(isotope_names[i], observed_profile[i]);
    }

    result_obs.SetName("Observed Elemental Profile for Isotopes");
    result_obs.SetResult(obs_profile);
    result_obs.SetType(result_type::predicted_concentration);
    return result_obs;
}

ResultItem SourceSinkData::GetCalculatedElementMeans()
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
    ResultItem resitem;
    resitem.SetName("Calculated mean elemental contents");
    resitem.SetType(result_type::elemental_profile_set);
    resitem.SetResult(profile_set);
    return resitem;
    
}
ResultItem SourceSinkData::GetCalculatedElementStandardDev()
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
    ResultItem resitem;
    resitem.SetName("Calculated elemental contents standard deviations");
    resitem.SetType(result_type::elemental_profile_set);
    resitem.SetResult(profile_set);
    return resitem;
}

vector<ResultItem> SourceSinkData::GetSourceProfiles()
{
    vector<ResultItem> out;
    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != end(); it++)
    {
        if (it->first != target_group)
        {
            ResultItem result_item;
            result_item.SetName("Elemental Profiles for " + it->first);
            result_item.SetShowAsString(true);
            result_item.SetShowTable(true);
            result_item.SetShowGraph(true);
            result_item.SetType(result_type::elemental_profile_set);

            Elemental_Profile_Set* profile_set = new Elemental_Profile_Set();
            *profile_set = it->second;
            result_item.SetResult(profile_set);
            out.push_back(result_item);

        }
    }
    return out;
}
ResultItem SourceSinkData::GetCalculatedElementMu()
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
    ResultItem resitem;
    resitem.SetName("Calculated geometrical mean of elemental contents");
    resitem.SetType(result_type::elemental_profile_set);
    resitem.SetResult(profile_set);
    return resitem;
}
ResultItem SourceSinkData::GetEstimatedElementMu()
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
    ResultItem resitem;
    resitem.SetName("Infered geometrical mean of elemental contents");
    resitem.SetType(result_type::elemental_profile_set);
    resitem.SetResult(profile_set);
    return resitem;
}

ResultItem SourceSinkData::GetEstimatedElementMean()
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
    ResultItem resitem;
    resitem.SetName("Infered mean of elemental contents");
    resitem.SetType(result_type::elemental_profile_set);
    resitem.SetResult(profile_set);
    return resitem;
}

ResultItem SourceSinkData::GetEstimatedElementSigma()
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
    ResultItem resitem;
    resitem.SetName("Infered sigma elemental contents");
    resitem.SetType(result_type::elemental_profile_set);
    resitem.SetResult(profile_set);
    return resitem;
}

bool SourceSinkData::WriteElementInformationToFile(QFile *file)
{
    file->write("***\n");
    file->write("Element Information\n");
    for (map<string,element_information>::iterator it = ElementInformation.begin(); it!=ElementInformation.end(); it++)
        file->write(QString::fromStdString(it->first).toUtf8()+ "\t" + Role(it->second.Role).toUtf8()+"\n");

    return true;
}

QJsonObject SourceSinkData::ElementInformationToJsonObject()
{
    QJsonObject json_object;
    
    for (map<string, element_information>::iterator it = ElementInformation.begin(); it != ElementInformation.end(); it++)
    {
        QJsonObject elem_info_json_obj;
        elem_info_json_obj["Role"] = Role(it->second.Role);
        elem_info_json_obj["Standard Ratio"] = it->second.standard_ratio;
        elem_info_json_obj["Base Element"] = QString::fromStdString(it->second.base_element);
        elem_info_json_obj["Include"] = it->second.include_in_analysis;
        json_object[QString::fromStdString(it->first)] = elem_info_json_obj;

    }


    return json_object;
}

bool SourceSinkData::ReadElementInformationfromJsonObject(const QJsonObject &jsonobject)
{
    ElementInformation.clear();
    for(QString key: jsonobject.keys() ) {
        element_information elem_info;
        elem_info.Role = Role(jsonobject[key].toObject()["Role"].toString());
        elem_info.standard_ratio = jsonobject[key].toObject()["Standard Ratio"].toDouble();
        elem_info.base_element = jsonobject[key].toObject()["Base Element"].toString().toStdString();
        elem_info.include_in_analysis = jsonobject[key].toObject()["Include"].toBool();
        ElementInformation[key.toStdString()] = elem_info;
    }

    return true;
}

bool SourceSinkData::ReadElementDatafromJsonObject(const QJsonObject &jsonobject)
{
    clear();
    for(QString key: jsonobject.keys() ) {
        Elemental_Profile_Set elemental_profile_set;
        elemental_profile_set.ReadFromJsonObject(jsonobject[key].toObject());
        operator[](key.toStdString()) = elemental_profile_set;
    }

    return true;
}

QJsonObject SourceSinkData::ElementDataToJsonObject()
{
    QJsonObject json_object;
    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != end(); it++)
    {
        json_object[QString::fromStdString(it->first)] = it->second.toJsonObject();
    }
    return json_object;
}


bool SourceSinkData::WriteDataToFile(QFile *file)
{
    file->write("***\n");
    file->write("Elemental Profiles\n");
    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it!=end(); it++)
    {
        file->write("**\n");
        file->write(QString::fromStdString(it->first+"\n").toUtf8());
        it->second.writetofile(file);
    }
    return true;
}

bool SourceSinkData::WriteToFile(QFile *file)
{
    return true;
}

bool SourceSinkData::ReadFromFile(QFile *fil)
{
    QJsonObject jsondoc = QJsonDocument().fromJson(fil->readAll()).object();
    ReadElementDatafromJsonObject(jsondoc["Element Data"].toObject());
    ReadElementInformationfromJsonObject(jsondoc["Element Information"].toObject());
    target_group = jsondoc["Target Group"].toString().toStdString();
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

bool SourceSinkData::Perform_Regression_vs_om_size(const string &om, const string &d, regression_form form)
{
    omconstituent = om;
    sizeconsituent = d;
    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it!=end(); it++)
    {
        it->second.SetRegression(om,d,form);
    }
    return true;
}

CMBVector SourceSinkData::DiscriminantFunctionAnalysis(const string &source1,const string &source2)
{
    CMBVector mean1 = at(source1).ElementMeans();
    CMBVector mean2 = at(source2).ElementMeans();
    CMBMatrix CovMatr1 = at(source1).CovarianceMatrix();
    CMBMatrix CovMatr2 = at(source2).CovarianceMatrix();
    CMBVector w = ((mean1-mean2)/(CovMatr1+CovMatr2));
    w=w/w.norm2();
    w.SetLabels(ElementNames());
    return w;
}

CMBVector SourceSinkData::DiscriminantFunctionAnalysis(const string &source1)
{
    CMBVector mean1 = at(source1).ElementMeans();
    Elemental_Profile_Set rest = TheRest(source1);
    CMBVector mean2 = rest.ElementMeans();
    CMBMatrix CovMatr1 = at(source1).CovarianceMatrix();
    CMBMatrix CovMatr2 = rest.CovarianceMatrix();
    CMBVector w = ((mean1-mean2)/(CovMatr1+CovMatr2));
    w=w/w.norm2();
    w.SetLabels(ElementNames());
    return w;
}

CMBMatrix SourceSinkData::DiscriminantFunctionAnalysis()
{
    CMBMatrix out(ElementNames().size(),this->size()-1);
    int i=0;
    for (map<string,Elemental_Profile_Set>::iterator source = begin(); source != end(); source++)
    {
        if (source->first!=target_group)
        {   CMBVector w = DiscriminantFunctionAnalysis(source->first);
            CMBMatrix elem_profile_set = source->second.toMatrix();
            string temp = elem_profile_set.ToString();
            out.matr[i] = w;
            out.SetRowLabel(i,source->first);
            i++;
        }
    }
    out.SetColumnLabels(ElementNames());
    return out;
}

CMBVector SourceSinkData::DFATransformed(const CMBVector &eigenvector, const string &sourcegroup)
{
    CMBVector out(operator[](sourcegroup).size());
    int i=0;
    for (map<string,Elemental_Profile>::iterator sample = operator[](sourcegroup).begin(); sample != operator[](sourcegroup).end(); sample++)
    {
        out[i] = sample->second.DotProduct(eigenvector);
        out.SetLabel(i,sample->first);
        i++;
    }

    return out;
}

Elemental_Profile_Set SourceSinkData::TheRest(const string &source)
{
    Elemental_Profile_Set out;
    for (map<string,Elemental_Profile_Set>::iterator profile_set = begin(); profile_set!=end(); profile_set++)
    {
        if (profile_set->first!=source && profile_set->first!=target_group)
            for (map<string, Elemental_Profile>::iterator profile=profile_set->second.begin(); profile!=profile_set->second.end(); profile++ )
            {
                out.Append_Profile(profile->first, profile->second);

            }
    }
    return out;
}


CMBVector SourceSinkData::BracketTest(const string &target_sample)
{

    vector<string> element_names = ElementNames();
    CMBVector out(element_names.size());
    CVector maxs(element_names.size());
    CVector mins(element_names.size());
    maxs.SetAllValues(1.0);
    mins.SetAllValues(1.0);
    for (map<string,Elemental_Profile_Set>::iterator it = begin(); it!=end(); it++)
    {
        for (unsigned int i=0; i<element_names.size(); i++)
        {   out.SetLabel(i,element_names[i]);
            if (at(target_group).Profile(target_sample)->at(element_names[i])<it->second.ElementalDistribution(element_names[i])->max())
            {
                    maxs[i]=0;
            }
            if (at(target_group).Profile(target_sample)->at(element_names[i])>it->second.ElementalDistribution(element_names[i])->min())
            {
                    mins[i]=0;
            }

        }
    }

    for (unsigned int i=0; i<element_names.size(); i++)
    {
        out[i] = max(maxs[i],mins[i]);
    }
    return out;
}


SourceSinkData SourceSinkData::BoxCoxTransformed()
{
    SourceSinkData out(*this);
    for (map<string,Elemental_Profile_Set>::iterator it=begin(); it!=end(); it++)
    {
        if (it->first!=target_group)
        {
            out[it->first] = it->second.BocCoxTransformed();
        }
    }
    out.PopulateElementDistributions();
    out.AssignAllDistributions();

    return out;
}
