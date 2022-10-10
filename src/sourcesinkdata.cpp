#include "sourcesinkdata.h"
#include "iostream"

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
    return *this;
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

bool SourceSinkData::InitializeParametersObservations(const string &targetsamplename)
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
    for (map<string,Elemental_Profile_Set>::iterator it = begin(); it!=end(); it++)
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

// Count the number of elements to be used
    numberofconstituents = 0;
    for (map<string, element_information>::iterator it = ElementInformation.begin(); it!=ElementInformation.end(); it++)
        if (it->second.Role == element_information::role::element)
            numberofconstituents ++;


// Source elemental profile geometrical mean for each element

    for (map<string, element_information>::iterator element_iterator = ElementInformation.begin(); element_iterator!=ElementInformation.end(); element_iterator++)
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
                    p.SetRange(source_iterator->second.GetFittedDistribution(element_iterator->first)->parameters[0]*0.8, source_iterator->second.GetFittedDistribution(element_iterator->first)->parameters[0]*1.2);
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

// Error Standard Deviation
    Parameter p;
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

    return true;
}

CVector SourceSinkData::GetSourceContributions()
{
    CVector contributions(size()-2);
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
        return -100;
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

double SourceSinkData::LogLikelihoodModelvsMeasured()
{
    double LogLikelihood = 0;
    CVector C = PredictTarget();
    CVector C_obs = ObservedDataforSelectedSample(selected_target_sample);
    for (unsigned int i=0; i<numberofconstituents; i++)
    {
        LogLikelihood -= log(error_stdev) + pow((C.Log()-C_obs.Log()).norm2(),2)/(2*pow(error_stdev,2));
    }
    return LogLikelihood;
}

CVector SourceSinkData::PredictTarget()
{
    CVector C = SourceMeanMatrix()*ContributionVector();
    return C;
}

double SourceSinkData::GetObjectiveFunctionValue()
{
    return -LogLikelihood();
}

double SourceSinkData::LogLikelihood()
{
    double YLogLikelihood = LogLikelihoodSourceElementalDistributions();
    double CLogLikelihood = LogLikelihoodModelvsMeasured();
    return YLogLikelihood + CLogLikelihood;
}

CMatrix SourceSinkData::SourceMeanMatrix()
{
    CMatrix Y(element_order.size(),numberofsourcesamplesets);
    for (unsigned int element_counter=0; element_counter<element_order.size(); element_counter++)
    {
        for (unsigned int source_group_counter=0; source_group_counter<numberofsourcesamplesets; source_group_counter++)
        {
            Elemental_Profile_Set *this_source_group = sample_set(samplesetsorder[source_group_counter]);
            Y[element_counter][source_group_counter] = this_source_group->GetEstimatedDistribution(element_order[element_counter])->Mean();
        }
    }
    return Y;
}

CVector SourceSinkData::ContributionVector()
{
    CVector X(numberofsourcesamplesets);
    for (unsigned int source_group_counter=0; source_group_counter<numberofsourcesamplesets; source_group_counter++)
    {
        Elemental_Profile_Set *this_source_group = sample_set(samplesetsorder[source_group_counter]);
        X[source_group_counter] = this_source_group->Contribution();
    }
    return X;
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

    parameters[i].SetValue(value);
    if (i<numberofsourcesamplesets-1)
    {
        sample_set(samplesetsorder[i])->SetContribution(value);
        sample_set(samplesetsorder[numberofsourcesamplesets-1])->SetContribution(1-ContributionVector().sum()+sample_set(samplesetsorder[numberofsourcesamplesets-1])->Contribution());
    }
    else if (i<numberofsourcesamplesets-1+numberofconstituents*numberofsourcesamplesets)
    {
        int element_counter = (i-(numberofsourcesamplesets-1))/numberofsourcesamplesets;
        int group_counter = (i-(numberofsourcesamplesets-1))%numberofsourcesamplesets;
        GetElementDistribution(element_order[element_counter],samplesetsorder[group_counter])->SetEstimatedMu(value);
    }
    else if (i<numberofsourcesamplesets-1+2*numberofconstituents*numberofsourcesamplesets)
    {
        int element_counter = (i-(numberofsourcesamplesets-1)-numberofconstituents*numberofsourcesamplesets)/numberofsourcesamplesets;
        int group_counter = (i-(numberofsourcesamplesets-1)-numberofconstituents*numberofsourcesamplesets)%numberofsourcesamplesets;
        GetElementDistribution(element_order[element_counter],samplesetsorder[group_counter])->SetEstimatedSigma(value);
    }
    else if (i==numberofsourcesamplesets-1+2*numberofconstituents*numberofsourcesamplesets)
    {
        error_stdev = value;
    }

    return true;
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
result_item SourceSinkData::GetPredictedElementalProfile()
{
    result_item result_modeled;

    Elemental_Profile *modeled_profile = new Elemental_Profile();
    CVector predicted_profile = PredictTarget();
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

result_item SourceSinkData::GetObservedElementalProfile()
{
    result_item result_obs;

    Elemental_Profile *obs_profile = new Elemental_Profile();
    CVector observed_profile = ObservedDataforSelectedSample(selected_target_sample);
    vector<string> element_names = ElementOrder();
    for (int i=0; i<element_names.size(); i++)
    {
        obs_profile->AppendElement(element_names[i],observed_profile[i]);
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
