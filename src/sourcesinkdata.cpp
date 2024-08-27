#include "sourcesinkdata.h"
#include "iostream"
#include "NormalDist.h"
#include "qjsondocument.h"
#include "resultitem.h"
#include <gsl/gsl_cdf.h>
#include "GADistribution.h"
#include "rangeset.h"
#include "QJsonArray"
#include "QJsonValue"
#include <qdir.h>



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
    distance_coeff = mp.distance_coeff;
    tools_used = mp.tools_used;

}

SourceSinkData SourceSinkData::Corrected(const string &target, bool omnsizecorrect, map<string, element_information> *elementinfo)
{
    SourceSinkData out;
    selected_target_sample = target;
    for (map<string, Elemental_Profile_Set>::iterator it=begin(); it!=end(); it++)
    {
        vector<double> om_size;
        if (omconstituent!="")
            om_size.push_back(at(target_group)[selected_target_sample][omconstituent]);
        if (sizeconsituent!="")
            om_size.push_back(at(target_group)[selected_target_sample][sizeconsituent]);
        if (it->first!=target_group)
        {
            out[it->first] = it->second.CopyIncludedinAnalysis(omnsizecorrect,om_size,elementinfo);
        }
        else
            out[it->first] = it->second.CopyIncludedinAnalysis(false,om_size,elementinfo);
    }
    out.omconstituent = omconstituent;
    out.sizeconsituent = sizeconsituent;
    if (elementinfo)
        for (map<string,element_information>::iterator it = ElementInformation.begin(); it!=ElementInformation.end(); it++)
        {
            if (it->second.include_in_analysis && it->second.Role!=element_information::role::do_not_include && it->second.Role!=element_information::role::organic_carbon && it->second.Role != element_information::role::particle_size)
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

    vector<double> om_size;
    if (omnsizecorrect)
    {
        if (omconstituent!="")
            om_size.push_back(at(target_group).at(out.selected_target_sample).at(omconstituent));
        if (sizeconsituent!="")
            om_size.push_back(at(target_group).at(out.selected_target_sample).at(sizeconsituent));
    }
    for (map<string, Elemental_Profile_Set>::const_iterator it=cbegin(); it!=cend(); it++)
    {
        if (it->first==target_group)
        {
            out[it->first] = it->second.CopyandCorrect(false,exclude_elements,false, om_size, &ElementInformation);
        }
        else
            out[it->first] = it->second.CopyandCorrect(exclude_samples,exclude_elements,omnsizecorrect,om_size,&ElementInformation);
    }
    out.omconstituent = omconstituent;
    out.sizeconsituent = sizeconsituent;
    
    out.PopulateElementInformation(&ElementInformation);
    out.PopulateElementDistributions();
    out.AssignAllDistributions();
    return out;
}

SourceSinkData SourceSinkData::ExtractElementsOnly(bool isotopes) const
{
    SourceSinkData out;
    out.target_group = target_group;

    for (map<string, Elemental_Profile_Set>::const_iterator it=cbegin(); it!=cend(); it++)
    {
        out[it->first] = it->second.ExtractElementsOnly(&ElementInformation, isotopes);
    }
    out.omconstituent = omconstituent;
    out.sizeconsituent = sizeconsituent;

    out.PopulateElementInformation(&ElementInformation);
    out.PopulateElementDistributions();
    out.AssignAllDistributions();
    return out;
}

SourceSinkData SourceSinkData::Extract(const vector<string> &element_list) const
{
    SourceSinkData out;

    for (map<string, Elemental_Profile_Set>::const_iterator it=cbegin(); it!=cend(); it++)
    {
        if (it->first!=target_group)
        {
            out[it->first] = it->second.Extract(element_list);
        }
    }
    out.PopulateElementInformation(&ElementInformation);
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
    distance_coeff = mp.distance_coeff;
    tools_used = mp.tools_used;
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
    tools_used.clear();
    clear(); 
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

Elemental_Profile_Set SourceSinkData::ExtractData_EPS(const vector<vector<string>> &indicators)
{
    Elemental_Profile_Set extracted_data;

    for (int i=0; i<indicators.size(); i++)
    {
        Elemental_Profile elemental_profile = sample_set(indicators[i][0])->at(indicators[i][1]);
        extracted_data.Append_Profile(indicators[i][0] + "-" + indicators[i][1],elemental_profile);

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
            if (ElementInformation[element_names[i]].Role!=element_information::role::isotope )
                it->second.ElementalDistribution(element_names[i])->FittedDistribution()->parameters = it->second.ElementalDistribution(element_names[i])->EstimateParameters(FittedDistribution(element_names[i])->distribution);
            else
                it->second.ElementalDistribution(element_names[i])->FittedDistribution()->parameters = it->second.ElementalDistribution(element_names[i])->EstimateParameters(distribution_type::normal);

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

void SourceSinkData::PopulateElementInformation(const map<string,element_information> *ElementInfo)
{
    ElementInformation.clear();
    vector<string> element_names = ElementNames();
    for (unsigned int i=0; i<element_names.size(); i++)
    {
        if (ElementInfo==nullptr)
            ElementInformation[element_names[i]] = element_information();
        else
            ElementInformation[element_names[i]] = ElementInfo->at(element_names[i]);
    }

    if (target_group!="")
        numberofsourcesamplesets = size()-1;
    else
        numberofsourcesamplesets = size();
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
                        source_iterator->second.GetEstimatedDistribution(element_iterator->first)->SetType(distribution_type::normal);
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
                        p.SetRange(max(source_iterator->second.GetFittedDistribution(element_iterator->first)->parameters[1] * 0.8,0.001), max(source_iterator->second.GetFittedDistribution(element_iterator->first)->parameters[1] / 0.8,2.0));
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
                        p.SetRange(max(source_iterator->second.GetFittedDistribution(element_iterator->first)->parameters[1] * 0.8,0.001), max(source_iterator->second.GetFittedDistribution(element_iterator->first)->parameters[1] / 0.8,2.0));
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
        C = PredictTarget_Isotope_delta(parameter_mode::based_on_fitted_distribution);
    else
        C = PredictTarget_Isotope_delta(parameter_mode::direct);
    CVector C_obs = ObservedDataforSelectedSample_Isotope_delta(selected_target_sample);


        LogLikelihood -= C.num*log(error_stdev_isotope) + pow((C-C_obs).norm2(),2)/(2*pow(error_stdev_isotope,2));

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
    CMatrix srcMatrixIso = SourceMeanMatrix_Isotopes(param_mode);
    CVector contribVect = ContributionVector();
    CVector C = srcMatrixIso*contribVect;

    return C;
}

CVector SourceSinkData::PredictTarget_Isotope_delta(parameter_mode param_mode)
{
    CMatrix SourceMeanMat = SourceMeanMatrix(param_mode);
    CMatrix SourceMeanMat_Iso = SourceMeanMatrix_Isotopes(param_mode);
    CVector C_elements = SourceMeanMat*ContributionVector();
    CVector C = SourceMeanMat_Iso*ContributionVector();
    for (unsigned int i=0; i<numberofisotopes; i++)
    {
        string corresponding_element = ElementInformation[isotope_order[i]].base_element;
        double predicted_corresponding_element_concentration = C_elements[lookup(element_order,corresponding_element)];
        double ratio = C[i]/predicted_corresponding_element_concentration;
        double standard_ratio = ElementInformation[isotope_order[i]].standard_ratio;
        C[i] = (ratio/standard_ratio-1.0)*1000.0;
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
    double CLogLikelihood_Isotope = 0;
    if (est_mode != estimation_mode::only_contributions)
        YLogLikelihood = LogLikelihoodSourceElementalDistributions();
    double LogPrior = LogPriorContributions();
    if (est_mode != estimation_mode::source_elemental_profiles_based_on_source_data)
    {   CLogLikelihood = LogLikelihoodModelvsMeasured(est_mode);
        CLogLikelihood_Isotope = LogLikelihoodModelvsMeasured_Isotope(est_mode);
    }
    return YLogLikelihood + CLogLikelihood + CLogLikelihood_Isotope + LogPrior;
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
// Add a corresponding element check

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
        GetElementDistribution(isotope_order[isotope_counter],samplesetsorder[group_counter])->SetEstimatedMu(value);
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
        GetElementDistribution(isotope_order[isotope_counter],samplesetsorder[group_counter])->SetEstimatedSigma(value);
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

double SourceSinkData::GetParameterValue(unsigned int i)
{
    return parameters[i].Value();
}


bool SourceSinkData::SetParameterValue(const CVector &value)
{
    bool out = true;
    for (int i=0; i<value.num; i++)
    {
        out &=SetParameterValue(i,value[i]);
    }
    return out;
}

CVector SourceSinkData::GetParameterValue()
{
    CVector out(parameters.size());
    for (unsigned int i=0; i<parameters.size(); i++)
    {
        out[i]=parameters[i].Value();
    }
    return out;
}

CVector SourceSinkData::Gradient(const CVector &value, const estimation_mode estmode)
{

    CVector out(value.num);
    CVector base_vector = value;
    SetParameterValue(value);
    double baseValue = LogLikelihood(estmode);
    for (int i=0; i<value.num; i++)
    {
        base_vector[i]+=epsilon;
        SetParameterValue(base_vector);
        double loglikehood = LogLikelihood(estmode);
        out[i] = (loglikehood-baseValue)/epsilon;
    }
    return out/out.norm2();
}

CVector SourceSinkData::GradientUpdate(const estimation_mode estmode)
{
    CVector X = GetParameterValue();
    double baseLikelihood = LogLikelihood(estmode);
    CVector dx = Gradient(X,estmode);
    CVector X_new1 = X+distance_coeff*dx;
    SetParameterValue(X_new1);
    double newLikelihood1 = LogLikelihood(estmode);
    CVector X_new2 = X+2*distance_coeff*dx;
    SetParameterValue(X_new2);
    double newLikelihood2 = LogLikelihood(estmode);
    qDebug()<<"Distance Coeff:" << distance_coeff;
    if (distance_coeff<10e-6)
        distance_coeff=1;
    if (newLikelihood2>newLikelihood1 && newLikelihood2>baseLikelihood)
    {
        distance_coeff*=2;
        return X_new2;
    }
    else if (newLikelihood1>newLikelihood2 && newLikelihood1>baseLikelihood)
    {
        SetParameterValue(X_new1);
        return X_new1;
    }
    else
    {
        int counter = 0;
        while (baseLikelihood>newLikelihood1 || counter<5)
        {
            distance_coeff*=0.5;
            X_new1 = X+distance_coeff*dx;
            SetParameterValue(X_new1);
            newLikelihood1 = LogLikelihood(estmode);
            counter++;
        }
        if (counter<5)
        {   SetParameterValue(X_new1);
            return X_new1;
        }
        else
        {
            SetParameterValue(X);
            return X;
        }
    }

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
    for (map<string, element_information>::iterator it = ElementInformation.begin(); it!=ElementInformation.end(); it++)
        if (it->second.Role == element_information::role::organic_carbon)
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
ResultItem SourceSinkData::GetCalculatedElementSigma()
{
    Elemental_Profile_Set* profile_set = new Elemental_Profile_Set();
    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != end(); it++)
    {
        if (it->first != target_group)
        {
            Elemental_Profile element_profile;
            for (unsigned int element_counter = 0; element_counter < element_order.size(); element_counter++)
            {
                if (it->second.ElementalDistribution(element_order[element_counter])->GetEstimatedDistribution()->distribution == distribution_type::normal)
                    element_profile.AppendElement(element_order[element_counter], it->second.ElementalDistribution(element_order[element_counter])->stdev());
                else if (it->second.ElementalDistribution(element_order[element_counter])->GetEstimatedDistribution()->distribution == distribution_type::lognormal)
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
    resitem.SetName("Calculated mu parameter of elemental contents");
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
    resitem.SetName("Infered mu parameter elemental contents");
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
                double sigma = it->second.ElementalDistribution(element_order[element_counter])->EstimatedSigma();
                element_profile.AppendElement(element_order[element_counter], sigma);
            }
            profile_set->Append_Profile(it->first, element_profile);
        }
    }
    ResultItem resitem;
    resitem.SetName("Infered sigma parameter");
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

QJsonArray SourceSinkData::ToolsUsedToJsonObject()
{
    QJsonObject json_object;
    QJsonArray tools_used_json_array;
    for (list<string>::iterator it = tools_used.begin(); it != tools_used.end(); it++)
    {
        tools_used_json_array.append(QString::fromStdString(*it));
    }

    return tools_used_json_array;
}

void SourceSinkData::AddtoToolsUsed(const string &tool)
{
    if (!ToolsUsed(tool))
        tools_used.push_back(tool);
}

bool SourceSinkData::ReadToolsUsedFromJsonObject(const QJsonArray &jsonarray)
{
    foreach (const QJsonValue & value, jsonarray)
    {
        AddtoToolsUsed(value.toString().toStdString());
    }

    return true;
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
    Clear();
    QJsonObject jsondoc = QJsonDocument().fromJson(fil->readAll()).object();
    ReadElementDatafromJsonObject(jsondoc["Element Data"].toObject());
    ReadElementInformationfromJsonObject(jsondoc["Element Information"].toObject());
    ReadToolsUsedFromJsonObject(jsondoc["Tools used"].toArray());
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
    else if (rl == element_information::role::organic_carbon)
        return "OM";
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
    else if (rl == "OM")
        return element_information::role::organic_carbon;
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

void SourceSinkData::OutlierAnalysisForAll(const double &lowerthreshold, const double &upperthreshold)
{
    for (map<string,Elemental_Profile_Set>::iterator it=begin(); it!=end(); it++)
        if (it->first!=target_group && !it->second.OutlierAnalysisDone())
            it->second.Outlier(lowerthreshold,upperthreshold);
}

DFA_result SourceSinkData::DiscriminantFunctionAnalysis()
{
    DFA_result out;
    CMBVector eigen_vector = DFA_eigvector();
    out.eigen_vectors.push_back(eigen_vector);
    double p_value = DFA_P_Value();
    out.p_values = CMBVector(1); out.p_values[0] = p_value;
    return out;
}


DFA_result SourceSinkData::DiscriminantFunctionAnalysis(const string &source1, const string &source2)
{
    SourceSinkData twoSources;
    twoSources.AppendSampleSet(source1,at(source1));
    twoSources.AppendSampleSet(source2,at(source2));
    twoSources.PopulateElementInformation(&ElementInformation);
    DFA_result out;
    CMBVector eigen_vector = twoSources.DFA_eigvector();
    out.projected = twoSources.DFA_Projected();
    out.eigen_vectors.push_back(eigen_vector);
    double p_value = twoSources.DFA_P_Value();
    out.p_values = CMBVector(1); out.p_values[0] = p_value;
    return out;
}


int SourceSinkData::TotalNumberofSourceSamples() const
{
    int count = 0;
    for (map<string,Elemental_Profile_Set>::const_iterator source_group = cbegin(); source_group!=cend(); source_group++)
    {
        if (source_group->first != target_group)
        {
            count += double(source_group->second.size());
        }
    }
    return count;
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
            if (at(target_group).Profile(target_sample)->at(element_names[i])<=it->second.ElementalDistribution(element_names[i])->max())
            {
                    maxs[i]=0;
            }
            if (at(target_group).Profile(target_sample)->at(element_names[i])>=it->second.ElementalDistribution(element_names[i])->min())
            {
                    mins[i]=0;
            }

        }
    }

    for (unsigned int i=0; i<element_names.size(); i++)
    {
        out[i] = max(maxs[i],mins[i]);
        if (maxs[i]>0.5)
            at(target_group).Profile(target_sample)->AppendtoNotes(element_names[i] + " value is higher than the maximum of the sources");
        else if (mins[i]>0.5)
            at(target_group).Profile(target_sample)->AppendtoNotes(element_names[i] + " value is lower than the minimum of the sources");
    }
    return out;
}


CMBMatrix SourceSinkData::BracketTest()
{
    vector<string> element_names = ElementNames();
    CMBMatrix out(at(target_group).size(),element_names.size());
    int j=0;

    for (map<string,Elemental_Profile>::iterator sample = at(target_group).begin(); sample != at(target_group).end(); sample++ )
    {

        CMBVector bracket_vec = BracketTest(sample->first);
        for (int i=0; i<element_names.size(); i++)
        {
            out[i][j] = bracket_vec.valueAt(i);
            out.SetRowLabel(i,element_names[i]);
        }
        out.SetColumnLabel(j,sample->first);
        j++;
    }

    return out;
}





SourceSinkData SourceSinkData::BoxCoxTransformed(bool calculateeigenvectors)
{
    CMBVector lambda_vals=OptimalBoxCoxParameters();
    SourceSinkData out(*this);
    for (map<string,Elemental_Profile_Set>::iterator it=begin(); it!=end(); it++)
    {
        if (it->first!=target_group)
        {
            if (calculateeigenvectors)
                out[it->first] = it->second.BocCoxTransformed(&lambda_vals);
            else
                out[it->first] = it->second.BocCoxTransformed();
        }
    }
    out.PopulateElementDistributions();
    out.AssignAllDistributions();

    return out;
}

map<string,ConcentrationSet> SourceSinkData::ExtractConcentrationSet()
{
    map<string,ConcentrationSet> out;
    vector<string> element_names=ElementNames();
    for (unsigned int i=0; i<element_names.size();i++)
    {   ConcentrationSet concset;
        for (map<string,Elemental_Profile_Set>::iterator it=begin(); it!=end(); it++)
        {
            if (it->first!=target_group)
            {
                for (map<string,Elemental_Profile>::iterator sample=it->second.begin(); sample!=it->second.end(); sample++)
                    concset.Append(sample->second[element_names[i]]);
            }
        }
        out[element_names[i]]=concset;

    }

    return out;
}

CMBVector SourceSinkData::OptimalBoxCoxParameters()
{
    map<string,ConcentrationSet> concset = ExtractConcentrationSet();
    vector<string> element_names=ElementNames();
    CMBVector out(element_names.size());
    for (unsigned int i=0; i<element_names.size();i++)
    {
        out[i] = concset[element_names[i]].OptimalBoxCoxParam(-5,5,10);

    }
    out.SetLabels(element_names);
    return out;
}

Elemental_Profile SourceSinkData::t_TestPValue(const string &source1, const string &source2, bool log)
{
    vector<string> element_names=ElementNames();
    Elemental_Profile elemental_profile_item;
    for (unsigned int i=0; i<element_names.size();i++)
    {
        ConcentrationSet ConcentrationSet1 = *at(source1).ElementalDistribution(element_names[i]);
        ConcentrationSet ConcentrationSet2 = *at(source2).ElementalDistribution(element_names[i]);
        double std1, std2;
        double mean1, mean2;
        if (!log)
        {
            std1 = ConcentrationSet1.stdev();
            std2 = ConcentrationSet2.stdev();
            mean1 = ConcentrationSet1.mean();
            mean2 = ConcentrationSet2.mean();
        }
        else
        {
            std1 = ConcentrationSet1.stdevln();
            std2 = ConcentrationSet2.stdevln();
            mean1 = ConcentrationSet1.meanln();
            mean2 = ConcentrationSet2.meanln();
        }
        double t = (mean1-mean2)/sqrt(pow(std1,2)/ConcentrationSet1.size() + pow(std2,2)/ConcentrationSet2.size());
        double pvalueQ = gsl_cdf_tdist_Q (t, ConcentrationSet1.size() + ConcentrationSet2.size() - 2);
        pvalueQ = min(pvalueQ, 1.0-pvalueQ);
        double pvalueP = gsl_cdf_tdist_P (t, ConcentrationSet1.size() + ConcentrationSet2.size() - 2);
        pvalueP = min(pvalueP, 1.0-pvalueP);
        elemental_profile_item.AppendElement(element_names[i],pvalueQ+pvalueP);
    }
    return elemental_profile_item;

}

Elemental_Profile SourceSinkData::DifferentiationPower(const string &source1, const string &source2, bool log)
{
    vector<string> element_names=ElementNames();
    Elemental_Profile elemental_profile_item;
    for (unsigned int i=0; i<element_names.size();i++)
    {
        ConcentrationSet ConcentrationSet1 = *at(source1).ElementalDistribution(element_names[i]);
        ConcentrationSet ConcentrationSet2 = *at(source2).ElementalDistribution(element_names[i]);
        double std1, std2;
        double mean1, mean2;
        if (!log)
        {
            std1 = ConcentrationSet1.stdev();
            std2 = ConcentrationSet2.stdev();
            mean1 = ConcentrationSet1.mean();
            mean2 = ConcentrationSet2.mean();
        }
        else
        {
            std1 = ConcentrationSet1.stdevln();
            std2 = ConcentrationSet2.stdevln();
            mean1 = ConcentrationSet1.meanln();
            mean2 = ConcentrationSet2.meanln();
        }
        elemental_profile_item.AppendElement(element_names[i],2*(fabs(mean1-mean2)/(std1+std2)));
    }
    return elemental_profile_item;

}


Elemental_Profile_Set SourceSinkData::DifferentiationPower(bool log, bool include_target)
{

    Elemental_Profile_Set out;
    for (map<string,Elemental_Profile_Set>::iterator it=begin(); it!=prev(end()); it++)
    {
        if (include_target || it->first!=target_group)
        for (map<string,Elemental_Profile_Set>::iterator it2=next(it); it2!=end(); it2++)
        {
            if (include_target || it2->first!=target_group)
            {   Elemental_Profile elem_prof = DifferentiationPower(it->first, it2->first, log);
                out.Append_Profile(it->first + " and " + it2->first, elem_prof);
            }
        }
    }
    return out;
}

Elemental_Profile SourceSinkData::DifferentiationPower_Percentage(const string &source1, const string &source2)
{
    vector<string> element_names=ElementNames();
    Elemental_Profile elemental_profile_item;
    for (unsigned int i=0; i<element_names.size();i++)
    {
        ConcentrationSet ConcentrationSet1 = *at(source1).ElementalDistribution(element_names[i]);
        ConcentrationSet ConcentrationSet2 = *at(source2).ElementalDistribution(element_names[i]);
        ConcentrationSet ConcentrationSetAll = ConcentrationSet1;
        ConcentrationSetAll.Append(ConcentrationSet2);
        vector<unsigned int> Rank = ConcentrationSetAll.Rank();
        int Set1BelowLimitCount = aquiutils::CountLessThan(Rank,ConcentrationSet1.size(),ConcentrationSet1.size(),false);
        int Set1AboveLimitCount = aquiutils::CountGreaterThan(Rank,ConcentrationSet1.size(),ConcentrationSet1.size(),false);
        int Set2BelowLimitCount = aquiutils::CountLessThan(Rank,ConcentrationSet1.size(),ConcentrationSet1.size(),true);
        int Set2AboveLimitCount = aquiutils::CountGreaterThan(Rank,ConcentrationSet1.size(),ConcentrationSet1.size(),true);
        double set1fraction = double(Set1BelowLimitCount+Set2AboveLimitCount)/double(ConcentrationSetAll.size());
        double set2fraction = double(Set1AboveLimitCount+Set2BelowLimitCount)/double(ConcentrationSetAll.size());
        elemental_profile_item.AppendElement(element_names[i],max(set1fraction,set2fraction));


    }
    return elemental_profile_item;

}

Elemental_Profile_Set SourceSinkData::DifferentiationPower_Percentage(bool include_target)
{
    Elemental_Profile_Set out;

    for (map<string,Elemental_Profile_Set>::iterator it=begin(); it!=prev(end()); it++)
    {
        if (include_target || it->first!=target_group)
        for (map<string,Elemental_Profile_Set>::iterator it2=next(it); it2!=end(); it2++)
        {
            if (include_target || it2->first!=target_group)
            {   Elemental_Profile elem_prof = DifferentiationPower_Percentage(it->first, it2->first);
                out.Append_Profile(it->first + " and " + it2->first, elem_prof);
            }
        }
    }
    return out;

}

Elemental_Profile_Set SourceSinkData::DifferentiationPower_P_value(bool include_target)
{
    Elemental_Profile_Set out;

    for (map<string,Elemental_Profile_Set>::iterator it=begin(); it!=prev(end()); it++)
    {
        if (include_target || it->first!=target_group)
        for (map<string,Elemental_Profile_Set>::iterator it2=next(it); it2!=end(); it2++)
        {
            if (include_target || it2->first!=target_group)
            {   Elemental_Profile elem_prof = t_TestPValue(it->first, it2->first,false);
                out.Append_Profile(it->first + " and " + it2->first, elem_prof);
            }
        }
    }
    return out;

}

vector<string> SourceSinkData::NegativeValueCheck()
{
    vector<string> out;
    populate_constituent_orders();
    for (map<string,Elemental_Profile_Set>::iterator it=begin(); it!=prev(end()); it++)
    {
        vector<string> NegativeItems = it->second.NegativeValueCheck(element_order);
        for (unsigned int i=0; i<NegativeItems.size(); i++ )
        {
            out.push_back("There are zero or negative values for element '" + NegativeItems[i] + "' in sample group '" + it->first +"'");
        }
    }
    return out;
}

void SourceSinkData::IncludeExcludeAllElements(bool value)
{
    for(map<string, element_information>::iterator it = ElementInformation.begin(); it!=ElementInformation.end(); it++)
    {
        it->second.include_in_analysis = value;
    }
}

double SourceSinkData::GrandMean(const string &element, bool logtransformed)
{
    double sum=0;
    double count=0;
    for (map<string,Elemental_Profile_Set>::iterator it=begin(); it!=prev(end()); it++)
    {
        if (it->first!=target_group)
        {   if (!logtransformed)
            {   sum+=it->second.ElementalDistribution(element)->mean()*it->second.ElementalDistribution(element)->size();
                count += it->second.ElementalDistribution(element)->size();
            }
            else
            {   sum+=it->second.ElementalDistribution(element)->meanln()*it->second.ElementalDistribution(element)->size();
                count += it->second.ElementalDistribution(element)->size();
            }
        }
    }
    return sum/count;
}

Elemental_Profile_Set SourceSinkData::LumpAllProfileSets()
{
    Elemental_Profile_Set out;
    for (map<string,Elemental_Profile_Set>::const_iterator it=begin(); it!=prev(end()); it++)
    {
        if (it->first!=target_group)
        {
            out.Append_Profiles(it->second,nullptr);
        }
    }
    out.UpdateElementalDistribution();
    return out;
}

CMBVector SourceSinkData::ANOVA(bool logtransformed)
{
    vector<string> element_names = ElementNames();
    CMBVector p_values(element_names.size());
    p_values.SetLabels(element_names);
    for (unsigned int i=0; i<element_names.size(); i++)
    {
        ANOVA_info anova = ANOVA(element_names[i],logtransformed);
        p_values[i] = anova.p_value;
    }
    return p_values;
}

ANOVA_info SourceSinkData::ANOVA(const string &element, bool logtransformed)
{
    ANOVA_info anova;
    Elemental_Profile_Set All_ProfileSets = LumpAllProfileSets();
    if (!logtransformed)
    {   anova.SST = All_ProfileSets.ElementalDistribution(element)->SSE();
        double sum=0;
        double sum_w=0;
        for (map<string,Elemental_Profile_Set>::iterator source_group = begin(); source_group!=end(); source_group++ )
        {
            if (source_group->first != target_group)
            {   sum += pow(source_group->second.ElementalDistribution(element)->mean()-All_ProfileSets.ElementalDistribution(element)->mean(),2)*source_group->second.ElementalDistribution(element)->size();
                sum_w += source_group->second.ElementalDistribution(element)->SSE();
            }
        }
        anova.SSB = sum;
        anova.SSW = sum_w;
    }
    else
    {   anova.SST = pow(All_ProfileSets.ElementalDistribution(element)->stdevln(),2)*(All_ProfileSets.ElementalDistribution(element)->size()-1);
        double sum=0;
        double sum_w=0;
        for (map<string,Elemental_Profile_Set>::iterator source_group = begin(); source_group!=end(); source_group++ )
        {
            if (source_group->first != target_group)
            {   sum += pow(source_group->second.ElementalDistribution(element)->meanln()-All_ProfileSets.ElementalDistribution(element)->meanln(),2)*source_group->second.ElementalDistribution(element)->size();
                sum_w += source_group->second.ElementalDistribution(element)->SSE_ln();
            }
        }
        anova.SSB = sum;
        anova.SSW = sum_w;
    }
    anova.MSB = anova.SSB/(double(this->size()-2.0));
    anova.MSW = anova.SSW/(double(All_ProfileSets.ElementalDistribution(element)->size())-(this->size()-1));
    anova.F = anova.MSB/anova.MSW;
    anova.p_value = gsl_cdf_fdist_Q (anova.F, this->size()-2, All_ProfileSets.size());
    return anova;

}

void SourceSinkData::IncludeExcludeElementsBasedOn(const vector<string> elements)
{

    for (map<string,element_information>::iterator element = ElementInformation.begin(); element!=ElementInformation.end(); element++)
    {
        element->second.include_in_analysis=false;
    }
    for (unsigned int i=0; i<elements.size(); i++)
    {
        ElementInformation[elements[i]].include_in_analysis=true;
    }
}

SourceSinkData SourceSinkData::RandomlyEliminateSourceSamples(const double &percentage)
{
    SourceSinkData out;
    vector<string> samplestobeeliminated = RandomlypickSamples(percentage/100.0);
    for (map<string, Elemental_Profile_Set>::const_iterator it=cbegin(); it!=cend(); it++)
    {
        if (it->first!=target_group)
        {
            out[it->first] = it->second.EliminateSamples(samplestobeeliminated,&ElementInformation);
        }
        else
            out[it->first] = it->second;
    }
    out.omconstituent = omconstituent;
    out.sizeconsituent = sizeconsituent;
    out.target_group = target_group;
    out.PopulateElementInformation(&ElementInformation);
    out.PopulateElementDistributions();
    out.AssignAllDistributions();
    return out;
}


Elemental_Profile SourceSinkData::Sample(const string &samplename) const
{
    for (map<string, Elemental_Profile_Set>::const_iterator it=cbegin(); it!=cend(); it++)
    {
        if (it->second.count(samplename)==1)
        {
            return it->second.at(samplename);
        }
    }
    return Elemental_Profile();
}

SourceSinkData SourceSinkData::ReplaceSourceAsTarget(const string &sourcesamplename) const
{
    SourceSinkData out = *this;
    Elemental_Profile target = Sample(sourcesamplename);
    Elemental_Profile_Set targetgroup = Elemental_Profile_Set();
    targetgroup.Append_Profile(sourcesamplename,target);
    out[target_group] = targetgroup;
    out.omconstituent = omconstituent;
    out.sizeconsituent = sizeconsituent;
    out.target_group = target_group;
    out.PopulateElementInformation(&ElementInformation);
    out.PopulateElementDistributions();
    out.AssignAllDistributions();
    return out;
}

CMBTimeSeriesSet SourceSinkData::BootStrap(const double &percentage, unsigned int number_of_samples, string target_sample, bool softmax_transformation)
{
    CMBTimeSeriesSet result(numberofsourcesamplesets);

    for (unsigned int source_group_counter=0; source_group_counter<numberofsourcesamplesets; source_group_counter++)
        result.setname(source_group_counter, samplesetsorder[source_group_counter]);
    for (unsigned int i=0; i<number_of_samples; i++)
    {
        SourceSinkData bootstrappeddata = RandomlyEliminateSourceSamples(percentage);
        bootstrappeddata.InitializeParametersObservations(target_sample);
        if (rtw)
            rtw->SetProgress(double(i)/double(number_of_samples));

        if (softmax_transformation)
            bootstrappeddata.SolveLevenBerg_Marquardt(transformation::softmax);
        else
            bootstrappeddata.SolveLevenBerg_Marquardt(transformation::linear);

        result.append(i,bootstrappeddata.ContributionVector().vec);
    }
    return result;
}

bool SourceSinkData::BootStrap(Results *res, const double &percentage, unsigned int number_of_samples, string target_sample, bool softmax_transformation)
{
    CMBTimeSeriesSet* contributions = new CMBTimeSeriesSet(numberofsourcesamplesets);

    for (unsigned int source_group_counter=0; source_group_counter<numberofsourcesamplesets; source_group_counter++)
        contributions->setname(source_group_counter, samplesetsorder[source_group_counter]);
    for (unsigned int i=0; i<number_of_samples; i++)
    {
        SourceSinkData bootstrappeddata = RandomlyEliminateSourceSamples(percentage);
        bootstrappeddata.InitializeParametersObservations(target_sample);
        if (rtw)
            rtw->SetProgress(double(i)/double(number_of_samples));

        if (softmax_transformation)
            bootstrappeddata.SolveLevenBerg_Marquardt(transformation::softmax);
        else
            bootstrappeddata.SolveLevenBerg_Marquardt(transformation::linear);

        contributions->append(i,bootstrappeddata.ContributionVector().vec);
    }
    ResultItem contributions_result_item;
    res->SetName("Error Analysis for target sample'" + target_sample +"'");
    contributions_result_item.SetName("Error Analysis");
    contributions_result_item.SetResult(contributions);
    contributions_result_item.SetType(result_type::stacked_bar_chart);
    contributions_result_item.SetShowAsString(false);
    contributions_result_item.SetShowTable(true);
    if (number_of_samples<101)
        contributions_result_item.SetShowGraph(true);
    else
        contributions_result_item.SetShowGraph(false);
    contributions_result_item.SetYLimit(_range::high, 1);
    contributions_result_item.SetXAxisMode(xaxis_mode::counter);
    contributions_result_item.setYAxisTitle("Contribution");
    contributions_result_item.setXAxisTitle("Sample");
    contributions_result_item.SetYLimit(_range::low, 0);
    res->Append(contributions_result_item);

    CMBTimeSeriesSet *dists = new CMBTimeSeriesSet();
    *dists = contributions->distribution(100,0,0);
    ResultItem distribution_res_item;
    distribution_res_item.SetName("Posterior Distributions");
    distribution_res_item.SetShowAsString(false);
    distribution_res_item.SetShowTable(true);
    distribution_res_item.SetType(result_type::distribution);
    distribution_res_item.SetResult(dists);
    res->Append(distribution_res_item);

//Posterior contribution 95% intervals
    RangeSet *contribution_credible_intervals = new RangeSet();
    for (unsigned int i=0; i<SourceOrder().size(); i++)
    {
        Range range;
        double percentile_low = contributions->BTC[i].percentile(0.025);
        double percentile_high = contributions->BTC[i].percentile(0.975);
        double mean = contributions->BTC[i].mean();
        double median = contributions->BTC[i].percentile(0.5);
        range.Set(_range::low,percentile_low);
        range.Set(_range::high,percentile_high);
        range.SetMean(mean);
        range.SetMedian(median);
        contribution_credible_intervals->operator[](contributions->names[i]) = range;
    }
    ResultItem contribution_credible_intervals_result_item;

    contribution_credible_intervals_result_item.SetName("Source Contribution Credible Intervals");
    contribution_credible_intervals_result_item.SetShowAsString(true);
    contribution_credible_intervals_result_item.SetShowTable(true);
    contribution_credible_intervals_result_item.SetType(result_type::rangeset);
    contribution_credible_intervals_result_item.SetResult(contribution_credible_intervals);
    contribution_credible_intervals_result_item.SetYAxisMode(yaxis_mode::normal);
    contribution_credible_intervals_result_item.SetYLimit(_range::high,1.0);
    contribution_credible_intervals_result_item.SetYLimit(_range::low,0);
    res->Append(contribution_credible_intervals_result_item);


    return true;

}

CMBTimeSeriesSet SourceSinkData::VerifySource(const string &sourcegroup, bool softmax_transformation, bool sizeandorganiccorrect)
{
    InitializeParametersObservations(sample_set(target_group)->begin()->first);
    CMBTimeSeriesSet result(numberofsourcesamplesets);

    for (unsigned int source_group_counter=0; source_group_counter<numberofsourcesamplesets; source_group_counter++)
        result.setname(source_group_counter, samplesetsorder[source_group_counter]);
    int counter = 0;
    for (map<string,Elemental_Profile>::iterator sample = at(sourcegroup).begin(); sample!=at(sourcegroup).end(); sample++)
    {

        SourceSinkData bootstrappeddata = ReplaceSourceAsTarget(sample->first);
        SourceSinkData correctedData = bootstrappeddata.Corrected(sample->first,sizeandorganiccorrect,bootstrappeddata.GetElementInformation());
        correctedData.SetProgressWindow(rtw);
        vector<string> negative_elements = correctedData.NegativeValueCheck();

        if (negative_elements.size()==0)
        {
            bootstrappeddata.InitializeParametersObservations(sample->first);
            if (rtw)
                rtw->SetProgress(double(counter)/double(at(sourcegroup).size()));

            if (softmax_transformation)
                bootstrappeddata.SolveLevenBerg_Marquardt(transformation::softmax);
            else
                bootstrappeddata.SolveLevenBerg_Marquardt(transformation::linear);

            result.append(counter,bootstrappeddata.ContributionVector().vec);
            result.SetLabel(counter,sample->first);
            counter++;
        }
        else
        {
            for (int i=0; i<negative_elements.size(); i++)
            {
                qDebug()<<QString::fromStdString(negative_elements[i]);
            }
        }


    }
    return result;
}

CMBTimeSeriesSet SourceSinkData::LM_Batch(transformation transform, bool om_size_correction, map<string,vector<string>> &negative_elements)
{

    InitializeParametersObservations(sample_set(target_group)->begin()->first);
    CMBTimeSeriesSet result(numberofsourcesamplesets);

    for (unsigned int source_group_counter=0; source_group_counter<numberofsourcesamplesets; source_group_counter++)
        result.setname(source_group_counter, samplesetsorder[source_group_counter]);
    int counter = 0;
    for (map<string,Elemental_Profile>::iterator sample = at(target_group).begin(); sample!=at(target_group).end(); sample++)
    {
        if (sample->first != "")
        {   SourceSinkData correctedData = Corrected(sample->first,om_size_correction,GetElementInformation());
            negative_elements[sample->first] = correctedData.NegativeValueCheck();
            if (negative_elements[sample->first].size()==0)
            {   correctedData.InitializeParametersObservations(sample->first);
                if (rtw)
                {   rtw->SetProgress(double(counter)/double(at(target_group).size()));
                    rtw->SetLabel(QString::fromStdString(sample->first));
                }

                correctedData.SolveLevenBerg_Marquardt(transform);

                result.append(counter,correctedData.ContributionVector().vec);
                result.SetLabel(counter,sample->first);
                counter++;
            }
        }

    }
    rtw->SetProgress(1);
    return result;
}



vector<string> SourceSinkData::AllSourceSampleNames() const
{
    vector<string> out;
    for (map<string, Elemental_Profile_Set>::const_iterator profile_set=cbegin(); profile_set!=cend(); profile_set++)
    {
        if (profile_set->first!=target_group)
        {
            for (map<string,Elemental_Profile>::const_iterator profile = profile_set->second.cbegin(); profile!=profile_set->second.cend(); profile++)
                out.push_back(profile->first);
        }
    }
    return out;


}
vector<string> SourceSinkData::RandomlypickSamples(const double &percentage) const
{
    vector<string> allsamples = AllSourceSampleNames();
    vector<string> out;
    for (unsigned int i=0; i<allsamples.size(); i++)
    {
        if (GADistribution::GetRndUniF(0,1)<percentage)
        {
            out.push_back(allsamples[i]);
        }
    }
    return out;
}

Results SourceSinkData::MCMC(const string &sample, map<string,string> arguments, CMCMC<SourceSinkData> *mcmc, ProgressWindow *rtw, const string &workingfolder)
{
    Results results;
    results.SetName("MCMC results for '" +sample + "'");
    ResultItem MCMC_samples;
    MCMC_samples.SetShowAsString(false);
    MCMC_samples.SetType(result_type::mcmc_samples);
    MCMC_samples.SetName("MCMC samples");
    CMBTimeSeriesSet *samples = new CMBTimeSeriesSet();

    bool organicnsizecorrection;
    if (arguments["Apply size and organic matter correction"]=="true")
    {
        organicnsizecorrection = true;
    }
    else
        organicnsizecorrection = false;

    SourceSinkData correctedData = Corrected(sample,organicnsizecorrection, GetElementInformation());
    vector<string> negative_elements = correctedData.NegativeValueCheck();
    if (negative_elements.size()>0)
    {
        results.SetError("Negative elemental content in ");
        for (unsigned int i=0; i<negative_elements.size(); i++)
        {
            if (i==0)
                results.AppendError(negative_elements[i]);
            else
                results.AppendError("," + negative_elements[i]);
        }
        return results;

    }

    mcmc->Model = &correctedData;

    rtw->SetTitle("Acceptance Rate",0);
    rtw->SetTitle("Purturbation Factor",1);
    rtw->SetTitle("Log posterior value",2);
    rtw->SetYAxisTitle("Acceptance Rate",0);
    rtw->SetYAxisTitle("Purturbation Factor",1);
    rtw->SetYAxisTitle("Log posterior value",2);
    rtw->show();
// Samples
    qDebug()<<2;
    correctedData.InitializeParametersObservations(sample);
    mcmc->SetProperty("number_of_samples",arguments["Number of samples"]);
    mcmc->SetProperty("number_of_chains",arguments["Number of chains"]);
    mcmc->SetProperty("number_of_burnout_samples",arguments["Samples to be discarded (burnout)"]);
    mcmc->SetProperty("dissolve_chains",arguments["Dissolve Chains"]);
    qDebug()<<3;
    mcmc->initialize(samples,true);
    string folderpath;
    if (!QString::fromStdString(arguments["Samples File Name"]).contains("/"))
        folderpath = workingfolder+"/";
    qDebug()<<4;
    mcmc->step(QString::fromStdString(arguments["Number of chains"]).toInt(), QString::fromStdString(arguments["Number of samples"]).toInt(), folderpath + arguments["Samples File Name"], samples, rtw);
    qDebug()<<"Outside MCMC";
    vector<string> SourceGroupNames = correctedData.SourceGroupNames();
    qDebug()<<"Appending last contributions";
    samples->AppendLastContribution(SourceGroupNames.size()-1,SourceGroupNames[SourceGroupNames.size()-1]+"_contribution");
    MCMC_samples.SetResult(samples);
    results.Append(MCMC_samples);
// Posterior distributions
    qDebug()<<"Posterior distributions";
    ResultItem distribution_res_item;
    CMBTimeSeriesSet *dists = new CMBTimeSeriesSet();
    *dists = samples->distribution(100,0,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
    distribution_res_item.SetName("Posterior Distributions");
    distribution_res_item.SetShowAsString(false);
    distribution_res_item.SetShowTable(true);
    distribution_res_item.SetType(result_type::distribution);
    distribution_res_item.SetResult(dists);
    results.Append(distribution_res_item);
    qDebug()<<"Posterior distributions 95%";
//Posterior contribution 95% intervals
    RangeSet *contribution_credible_intervals = new RangeSet();
    for (unsigned int i=0; i<correctedData.SourceOrder().size(); i++)
    {
        Range range;
        double percentile_low = samples->BTC[i].percentile(0.025,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
        double percentile_high = samples->BTC[i].percentile(0.975,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
        double mean = samples->BTC[i].mean(QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
        double median = samples->BTC[i].percentile(0.5,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
        range.Set(_range::low,percentile_low);
        range.Set(_range::high,percentile_high);
        range.SetMean(mean);
        range.SetMedian(median);
        contribution_credible_intervals->operator[](samples->names[i]) = range;
    }
    ResultItem contribution_credible_intervals_result_item;

    contribution_credible_intervals_result_item.SetName("Source Contribution Credible Intervals");
    contribution_credible_intervals_result_item.SetShowAsString(true);
    contribution_credible_intervals_result_item.SetShowTable(true);
    contribution_credible_intervals_result_item.SetType(result_type::rangeset);
    contribution_credible_intervals_result_item.SetResult(contribution_credible_intervals);
    contribution_credible_intervals_result_item.SetYAxisMode(yaxis_mode::log);
    contribution_credible_intervals_result_item.SetYLimit(_range::high,1.0);
    results.Append(contribution_credible_intervals_result_item);
    qDebug()<<"Predicted distributions";
// Predicted 95% posterior distributions
    CMBTimeSeriesSet predicted_samples = mcmc->predicted;
    CMBTimeSeriesSet predicted_samples_elems;
    vector<string> ConstituentNames = correctedData.ElementOrder();
    vector<string> IsotopeNames = correctedData.IsotopeOrder();
    vector<string> AllNames = ConstituentNames;

    AllNames.insert(AllNames.end(),IsotopeNames.begin(), IsotopeNames.end());

    for (int i=0; i<predicted_samples.nvars; i++)
        predicted_samples.setname(i,AllNames[i]);



    for (int i=0; i<predicted_samples.nvars; i++)
    {
        if (correctedData.GetElementInformation(predicted_samples.names[i])->Role==element_information::role::element)
            predicted_samples_elems.append(predicted_samples[i],predicted_samples.names[i]);
    }
    ResultItem predicted_distribution_res_item;
    CMBTimeSeriesSet *predicted_dists_elems = new CMBTimeSeriesSet();

    *predicted_dists_elems = predicted_samples_elems.distribution(100,0,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
    for (int i=0; i<predicted_samples.nvars; i++)
        predicted_dists_elems->SetObservedValue(i,correctedData.observation(i)->Value());

    predicted_distribution_res_item.SetName("Posterior Predicted Constituents");
    predicted_distribution_res_item.SetShowAsString(false);
    predicted_distribution_res_item.SetShowTable(true);
    predicted_distribution_res_item.SetType(result_type::distribution_with_observed);
    predicted_distribution_res_item.SetResult(predicted_dists_elems);
    results.Append(predicted_distribution_res_item);
    qDebug()<<"Predicted distributions 95%";
//predicted 95% credible intervals

    RangeSet *predicted_credible_intervals = new RangeSet();
    vector<double> percentile_low = predicted_samples.percentile(0.025,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
    vector<double> percentile_high = predicted_samples.percentile(0.975,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
    vector<double> mean = predicted_samples.mean(QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
    vector<double> median = predicted_samples.percentile(0.5,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
    for (int i=0; i<predicted_dists_elems->nvars; i++)
    {
        Range range;
        range.Set(_range::low,percentile_low[i]);
        range.Set(_range::high,percentile_high[i]);
        range.SetMean(mean[i]);
        range.SetMedian(median[i]);
        predicted_credible_intervals->operator[](predicted_dists_elems->names[i]) = range;
        predicted_credible_intervals->operator[](predicted_dists_elems->names[i]).SetValue(correctedData.observation(i)->Value());
    }
    ResultItem predicted_credible_intervals_result_item;
    predicted_credible_intervals_result_item.SetName("Predicted Samples Credible Intervals");
    predicted_credible_intervals_result_item.SetShowAsString(true);
    predicted_credible_intervals_result_item.SetShowTable(true);
    predicted_credible_intervals_result_item.SetType(result_type::rangeset_with_observed);
    predicted_credible_intervals_result_item.SetResult(predicted_credible_intervals);
    predicted_credible_intervals_result_item.SetYAxisMode(yaxis_mode::log);
    results.Append(predicted_credible_intervals_result_item);
    qDebug()<<"Predicted isotope distributions";
    // Predicted 95% posterior distributions for isotopes
    CMBTimeSeriesSet predicted_samples_isotopes;

    for (int i=0; i<predicted_samples.nvars; i++)
    {
        if (correctedData.GetElementInformation(predicted_samples.names[i])->Role==element_information::role::isotope)
            predicted_samples_isotopes.append(predicted_samples[i],predicted_samples.names[i]);
    }
    ResultItem predicted_distribution_iso_res_item;
    CMBTimeSeriesSet *predicted_dists_isotopes = new CMBTimeSeriesSet();

    *predicted_dists_isotopes = predicted_samples_isotopes.distribution(100,0,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
    for (int i=0; i<predicted_samples_isotopes.nvars; i++)
        predicted_dists_isotopes->SetObservedValue(i,correctedData.observation(i+ConstituentNames.size())->Value());

    predicted_distribution_iso_res_item.SetName("Posterior Predicted Isotopes");
    predicted_distribution_iso_res_item.SetShowAsString(false);
    predicted_distribution_iso_res_item.SetShowTable(true);
    predicted_distribution_iso_res_item.SetType(result_type::distribution_with_observed);
    predicted_distribution_iso_res_item.SetResult(predicted_dists_isotopes);
    results.Append(predicted_distribution_iso_res_item);
    qDebug()<<"Predicted isotope distributions 95%";
    //predicted 95% credible intervals for isotopes

    RangeSet *predicted_credible_intervals_isotopes = new RangeSet();

    for (int i=0; i<predicted_dists_isotopes->nvars; i++)
    {
        Range range;
        range.Set(_range::low,percentile_low[i+correctedData.ElementOrder().size()]);
        range.Set(_range::high,percentile_high[i+correctedData.ElementOrder().size()]);
        range.SetMean(mean[i+correctedData.ElementOrder().size()]);
        range.SetMedian(median[i+correctedData.ElementOrder().size()]);
        predicted_credible_intervals_isotopes->operator[](predicted_dists_isotopes->names[i]) = range;
        predicted_credible_intervals_isotopes->operator[](predicted_dists_isotopes->names[i]).SetValue(correctedData.observation(i+correctedData.ElementOrder().size())->Value());
    }
    ResultItem predicted_credible_intervals_isotope_result_item;
    predicted_credible_intervals_isotope_result_item.SetName("Predicted Samples Credible Intervals for Isotopes");
    predicted_credible_intervals_isotope_result_item.SetShowAsString(true);
    predicted_credible_intervals_isotope_result_item.SetShowTable(true);
    predicted_credible_intervals_isotope_result_item.SetType(result_type::rangeset_with_observed);
    predicted_credible_intervals_isotope_result_item.SetResult(predicted_credible_intervals_isotopes);
    predicted_credible_intervals_isotope_result_item.SetYAxisMode(yaxis_mode::normal);
    results.Append(predicted_credible_intervals_isotope_result_item);
    qDebug()<<"Done for sample "<< QString::fromStdString(sample);
    rtw->SetProgress(1);
    return results;
}

CMBMatrix SourceSinkData::MCMC_Batch(map<string,string> arguments, CMCMC<SourceSinkData> *mcmc, ProgressWindow *rtw, const string &workingfolder)
{
    InitializeParametersObservations(at(target_group).begin()->first);
    CMBMatrix contributions(numberofsourcesamplesets*4,at(target_group).size());

    for (unsigned int i=0; i<numberofsourcesamplesets; i++)
    {
        contributions.SetColumnLabel(i*4, samplesetsorder[i] +"-"+"low");
        contributions.SetColumnLabel(i*4+1, samplesetsorder[i] +"-"+"high");
        contributions.SetColumnLabel(i*4+2, samplesetsorder[i] +"-"+"median");
        contributions.SetColumnLabel(i*4+3, samplesetsorder[i] +"-"+"mean");
    }
    int counter = 0;
    for (map<string,Elemental_Profile>::iterator sample = at(target_group).begin(); sample!=at(target_group).end(); sample++)
    {
        qDebug()<<QString::fromStdString(sample->first)<<":"<<"Creating folder";
        QDir dir(QString::fromStdString(workingfolder)+"/"+QString::fromStdString(sample->first));
        if (!dir.exists())
            dir.mkpath(".");
        contributions.SetRowLabel(counter,sample->first);
        rtw->SetLabel(QString::fromStdString(sample->first));
        qDebug()<<QString::fromStdString(sample->first)<<":"<<"Performing MCMC";
        Results results = MCMC(sample->first, arguments, mcmc, rtw, workingfolder);
        qDebug()<<QString::fromStdString(sample->first)<<":"<<"Writing Result Items";
        for (map<string,ResultItem>::iterator result_item = results.begin(); result_item!=results.end(); result_item++ )
        {
            QString file_name = dir.absolutePath()+"/"+QString::fromStdString(result_item->first)+".txt";
            QFile file(file_name);
            file.open(QIODevice::WriteOnly | QIODevice::Text);
            result_item->second.Result()->writetofile(&file);
            file.close();
        }
        for (unsigned int i=0; i<numberofsourcesamplesets; i++)
        {
            contributions[counter][4*i] = static_cast<RangeSet*>(results.at("3:Source Contribution Credible Intervals").Result())->at(samplesetsorder[i]+"_contribution").Get(_range::low);
            contributions[counter][4*i+1] = static_cast<RangeSet*>(results.at("3:Source Contribution Credible Intervals").Result())->at(samplesetsorder[i]+"_contribution").Get(_range::high);
            contributions[counter][4*i+2] = static_cast<RangeSet*>(results.at("3:Source Contribution Credible Intervals").Result())->at(samplesetsorder[i]+"_contribution").Median();
            contributions[counter][4*i+3] = static_cast<RangeSet*>(results.at("3:Source Contribution Credible Intervals").Result())->at(samplesetsorder[i]+"_contribution").Mean();
        }

        counter++;
        rtw->SetProgress2(double(counter)/double(at(target_group).size()));
        rtw->ClearGraph(0);
        rtw->ClearGraph(1);
        rtw->ClearGraph(2);
    }
    rtw->SetProgress2(1);
    return contributions;
}

bool SourceSinkData::ToolsUsed(const string &toolname)
{
    std::list<string>::iterator iter = std::find (tools_used.begin(), tools_used.end(), toolname);
    if (iter == tools_used.end())
        return false;
    else
        return true;
}

string SourceSinkData::FirstOMConstituent()
{
    for (map<string,element_information>::iterator element = ElementInformation.begin(); element!=ElementInformation.end(); element++)
    {
        if (element->second.Role == element_information::role::organic_carbon)
            return element->first;
    }
    return "";
}


string SourceSinkData::FirstSizeConstituent()
{
    for (map<string,element_information>::iterator element = ElementInformation.begin(); element!=ElementInformation.end(); element++)
    {
        if (element->second.Role == element_information::role::particle_size)
            return element->first;
    }
    return "";
}

CMatrix SourceSinkData::WithinGroupCovarianceMatrix()
{
    vector<string> elementNames = ElementNames();
    CMatrix CovMatr(elementNames.size());
    int counter = 0;
    for (map<string,Elemental_Profile_Set>::iterator source_group = begin(); source_group!=end(); source_group++)
    {
        if (source_group->first != target_group)
        {   CovMatr += (source_group->second.size()-1)*source_group->second.CovarianceMatrix();
            counter += source_group->second.size()-1;
        }
    }
    return CovMatr/double(counter);
}

CMatrix SourceSinkData::BetweenGroupCovarianceMatrix()
{
    vector<string> elementNames = ElementNames();
    CMatrix out(elementNames.size());
    double count = 0;
    for (map<string,Elemental_Profile_Set>::iterator source_group = begin(); source_group!=end(); source_group++)
    {
        if (source_group->first != target_group)
        {
            CMBVector deviation = MeanElementalContent() - MeanElementalContent(source_group->first);
            for (int i=0; i<elementNames.size(); i++)
                for (int j=0; j<elementNames.size(); j++)
                    out[i][j] = +deviation[i]*deviation[j]*source_group->second.size();
            count += source_group->second.size();
        }
    }
    return out/count;
}

CMatrix SourceSinkData::TotalScatterMatrix()
{
    vector<string> elementNames = ElementNames();
    CMatrix out(elementNames.size());
    double count = 0;
    CMBVector OverallMean = MeanElementalContent();
    for (map<string,Elemental_Profile_Set>::iterator source_group = begin(); source_group!=end(); source_group++)
    {
        if (source_group->first != target_group)
        {
            for (map<string,Elemental_Profile>::iterator elem_prof = source_group->second.begin(); elem_prof!=source_group->second.end(); elem_prof++ )
            {
                for (int i=0; i<elementNames.size(); i++)
                    for (int j=0; j<elementNames.size(); j++)
                        out[i][j] += (OverallMean[i]-elem_prof->second.at(elementNames[i]))*(OverallMean[j]-elem_prof->second.at(elementNames[j]));
            }
            count += source_group->second.size();
        }
    }
    return out/count;
}

double SourceSinkData::WilksLambda()
{
    CMatrix_arma S_w = WithinGroupCovarianceMatrix();
    CMatrix_arma S_T = TotalScatterMatrix();
    double numerator = S_w.det();
    double denumerator = S_T.det();
    return numerator/denumerator;
}

double SourceSinkData::DFA_P_Value()
{
    int element_count = ElementNames().size();
    double wilkslambda = min(WilksLambda(),1.0);
    double ChiSquared = -(TotalNumberofSourceSamples() - 1 - (element_count+(numberofsourcesamplesets-1.0)/2.0))*log(wilkslambda);
    double df;
    if (target_group!="")
        df = element_count*(numberofsourcesamplesets - 2.0);
    else
        df = element_count*(numberofsourcesamplesets - 1.0);
    double p_value = gsl_cdf_chisq_Q (ChiSquared, df);
    return p_value;
}

CMBVectorSet SourceSinkData::DFA_Projected()
{
    CMBVector eigen_vector = DFA_eigvector();
    CMBVectorSet out;
    for (map<string,Elemental_Profile_Set>::iterator source_group = begin(); source_group!=end(); source_group++)
    {
        CMBVector weighted = source_group->second.DotProduct(eigen_vector);
        out.Append(source_group->first,weighted);
    }
    return out;
}

CMBVector SourceSinkData::DFA_eigvector()
{
    CMatrix_arma S_B = BetweenGroupCovarianceMatrix();
    CMatrix_arma S_w = WithinGroupCovarianceMatrix();
    CMatrix_arma Product = S_B*inv(S_w);

    arma::vec eigval;
    arma::mat eigvec;

    eig_sym(eigval, eigvec, Product.matr);
    CVector_arma Eigvals(eigval);
    CMatrix_arma EigVecs(eigvec);

    CMBVector out;
    if (fabs(Eigvals[0])>fabs(Eigvals[Eigvals.num-1]))
        out = CVector_arma(eigvec.col(0));
    else
        out = CVector_arma(eigvec.col(Eigvals.num-1));
    vector<string> elementNames = ElementNames();
    out.SetLabels(elementNames);
    return out;
}

CMBVector SourceSinkData::MeanElementalContent(const string &group_name)
{
    CMBVector out;
    if (count(group_name)==0)
        return out;
    vector<string> elementNames = ElementNames();
    out = at(group_name).ElementMeans();
    out.SetLabels(elementNames);
    return out;
}

CMBVector SourceSinkData::MeanElementalContent()
{
    vector<string> elementNames = ElementNames();
    CMBVector out(elementNames.size());
    double count = 0;
    for (map<string,Elemental_Profile_Set>::iterator source_group = begin(); source_group!=end(); source_group++)
    {
        if (source_group->first != target_group)
        {
            out += double(source_group->second.size())*MeanElementalContent(source_group->first);
            count += double(source_group->second.size());
        }
    }
    out.SetLabels(element_order);
    return out/count;
}
