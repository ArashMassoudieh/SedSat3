#include "sourcesinkdata.h"
#include "iostream"

SourceSinkData::SourceSinkData()
{

}

SourceSinkData::SourceSinkData(const SourceSinkData& mp)
{
    sample_sets = mp.sample_sets;
    ElementInformation = mp.ElementInformation;
    element_distributions = mp.element_distributions;
    numberofelements = mp.numberofelements;
    numberofsourcesamplesets = mp.numberofsourcesamplesets;
    observations = mp.observations;
    outputpath = mp.outputpath;
    parameters = mp.parameters;
    target_group = mp.target_group;
    samplesetsorder = mp.samplesetsorder;
    elementorder = mp.elementorder;

}
SourceSinkData& SourceSinkData::operator=(const SourceSinkData &mp)
{
    sample_sets = mp.sample_sets;
    ElementInformation = mp.ElementInformation;
    element_distributions = mp.element_distributions;
    numberofelements = mp.numberofelements;
    numberofsourcesamplesets = mp.numberofsourcesamplesets;
    observations = mp.observations;
    outputpath = mp.outputpath;
    parameters = mp.parameters;
    target_group = mp.target_group;
    samplesetsorder = mp.samplesetsorder;
    elementorder = mp.elementorder;
    return *this;
}

Elemental_Profile_Set* SourceSinkData::AppendSampleSet(const string &name, const Elemental_Profile_Set &elemental_profile_set)
{
    if (sample_sets.count(name)==0)
        sample_sets[name] = elemental_profile_set;
    else
    {   cout<<"Sample set type '" + name + "' already exists!"<<endl;
        return nullptr;
    }

    return &sample_sets[name];
}

Elemental_Profile_Set *SourceSinkData::sample_set(const string &name)
{
    if (sample_sets.count(name)!=0)
    {
        return &sample_sets[name];
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
    for (map<string,Elemental_Profile_Set>::iterator it=sample_sets.begin(); it!=sample_sets.end(); it++)
    {
        out.push_back(it->first);
    }
    return out;
}
vector<string> SourceSinkData::ElementNames()
{
    vector<string> out;
    if (sample_sets.size()>0)
    for (map<string,double>::iterator it=sample_sets.begin()->second.begin()->second.begin(); it!=sample_sets.begin()->second.begin()->second.end(); it++)
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
    for (map<string,Elemental_Profile_Set>::iterator it=sample_sets.begin(); it!=sample_sets.end(); it++)
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
        for (map<string, Elemental_Profile_Set>::iterator it=sample_sets.begin(); it!=sample_sets.end(); it++)
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
    ElementsToBeUsedInCMB();
   if (sample_sets.size()==0)
   {
        cout<<"Data has not been loaded!"<<endl;
        return false;
   }
    numberofsourcesamplesets = sample_sets.size()-1;
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
    numberofelements = 0;
    for (map<string, element_information>::iterator it = ElementInformation.begin(); it!=ElementInformation.end(); it++)
        if (it->second.Role == element_information::role::element)
            numberofelements ++;


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
                    p.SetRange(-10, 10);
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
                    p.SetPriorDistribution(distribution_type::normal);
                    p.SetRange(-10, 10);
                    parameters.push_back(p);
                }
            }
        }
    }

// Error Standard Deviation
    Parameter p;
    p.SetName("Error STDev");
    p.SetPriorDistribution(distribution_type::lognormal);
    p.SetRange(-5, 5);
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
    CVector contributions(sample_sets.size()-2);
    for (unsigned long int i=0; i<sample_sets.size()-2; i++)
    {
        contributions[i] = parameter(i)->Value();
    }
    contributions[sample_sets.size()-2] = 1 - contributions.sum();
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
    for (unsigned int element_counter=0; element_counter<numberofelements; element_counter++)
    {
        for (unsigned int source_group_counter=0; source_group_counter<numberofsourcesamplesets; source_group_counter++)
        {
            Elemental_Profile_Set *this_source_group = sample_set(samplesetsorder[source_group_counter]);


            for (map<string,Elemental_Profile>::iterator sample = this_source_group->begin(); sample!=this_source_group->end(); sample++)
            {
                logLikelihood += this_source_group->ElementalDistribution(elementorder[element_counter])->FittedDistribution()->EvalLog(sample->second.Val(elementorder[element_counter]));
            }

        }
    }
    return logLikelihood;
}

CVector SourceSinkData::ObservedDataforSelectedSample(const string &SelectedTargetSample)
{
    CVector observed_data(numberofelements);
    for (unsigned int i=0; i<numberofelements; i++)
    {   if (SelectedTargetSample!="")
            observed_data[i] = this->sample_set(TargetGroup())->Profile(SelectedTargetSample)->Val(elementorder[i]);
        else if (selected_target_sample!="")
            observed_data[i] = this->sample_set(TargetGroup())->Profile(selected_target_sample)->Val(elementorder[i]);
    }
    return observed_data;
}

double SourceSinkData::LogLikelihoodModelvsMeasured()
{
    double LogLikelihood = 0;
    CVector C = PredictTarget();
    CVector C_obs = ObservedDataforSelectedSample();
    for (unsigned int i=0; i<numberofelements; i++)
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
    CMatrix Y(numberofelements,numberofsourcesamplesets);
    for (unsigned int element_counter=0; element_counter<numberofelements; element_counter++)
    {
        for (unsigned int source_group_counter=0; source_group_counter<numberofsourcesamplesets; source_group_counter++)
        {
            Elemental_Profile_Set *this_source_group = sample_set(samplesetsorder[source_group_counter]);
            Y[element_counter][source_group_counter] = this_source_group->GetEstimatedDistribution(elementorder[element_counter])->Mean();
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
    return &parameters[sample_sets.size()-2+element_iterator*numberofsourcesamplesets +source_iterator];
}
Parameter* SourceSinkData::ElementalContent_sigma(int element_iterator, int source_iterator)
{
    return &parameters[sample_sets.size()-2+element_iterator*numberofsourcesamplesets +source_iterator + numberofelements*numberofsourcesamplesets];
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
    else if (i<numberofsourcesamplesets-1+numberofelements*numberofsourcesamplesets)
    {
        int element_counter = (i-(numberofsourcesamplesets-1))/numberofsourcesamplesets;
        int group_counter = (i-(numberofsourcesamplesets-1))%numberofsourcesamplesets;
        GetElementDistribution(elementorder[element_counter],samplesetsorder[group_counter])->SetEstimatedMu(value);
    }
    else if (i<numberofsourcesamplesets-1+2*numberofelements*numberofsourcesamplesets)
    {
        int element_counter = (i-(numberofsourcesamplesets-1)-numberofelements*numberofsourcesamplesets)/numberofsourcesamplesets;
        int group_counter = (i-(numberofsourcesamplesets-1)-numberofelements*numberofsourcesamplesets)%numberofsourcesamplesets;
        GetElementDistribution(elementorder[element_counter],samplesetsorder[group_counter])->SetEstimatedSigma(value);
    }
    else if (i==numberofsourcesamplesets-1+2*numberofelements*numberofsourcesamplesets)
    {
        error_stdev = value;
    }

    return true;
}


vector<string> SourceSinkData::ElementsToBeUsedInCMB()
{
    numberofelements = 0;
    elementorder.clear();
    for (map<string, element_information>::iterator it = ElementInformation.begin(); it!=ElementInformation.end(); it++)
        if (it->second.Role == element_information::role::element)
        {   numberofelements ++;
            elementorder.push_back(it->first);
        }
    return elementorder;
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

    for (map<string,Elemental_Profile_Set>::const_iterator group=sample_sets.begin(); group!=sample_sets.end(); group++ )
    {
        for (map<string,Elemental_Profile>::const_iterator sample=group->second.cbegin(); sample!=group->second.cend(); sample++)
            if (sample->first == sample_name)
                return sample_set(group->first)->Profile(sample->first);
    }
    return nullptr;
}
