#include "elemental_profile_set.h"
#include "iostream"
#include <gsl/gsl_statistics_double.h>


using namespace std;

Elemental_Profile_Set::Elemental_Profile_Set() :map<string, Elemental_Profile>(), Interface()
{

}

Elemental_Profile_Set::Elemental_Profile_Set(const Elemental_Profile_Set& mp) :map<string, Elemental_Profile>(mp), Interface()
{
    element_distributions = mp.element_distributions;
    contribution = mp.contribution;
}

Elemental_Profile_Set& Elemental_Profile_Set::operator=(const Elemental_Profile_Set &mp)
{
    map<string, Elemental_Profile>::operator=(mp);
    Interface::operator=(mp);
    contribution = mp.contribution;
    element_distributions = mp.element_distributions;
    return *this;
}

Elemental_Profile_Set Elemental_Profile_Set::CopyandCorrect(bool exclude_samples, bool exclude_elements, bool omnsizecorrect, const double &om, const double &psize, const map<string, element_information> *elementinfo) const
{
    Elemental_Profile_Set out;
    for (map<string,Elemental_Profile>::const_iterator it=cbegin(); it!=cend(); it++)
    {
        if (it->second.IncludedInAnalysis() || !exclude_samples)
        {
            if ((mlr_vs_om_size.size()!=0))
                out.Append_Profile(it->first, it->second.CopyandCorrect(exclude_elements,omnsizecorrect,om,psize,&mlr_vs_om_size,elementinfo));
            else
                out.Append_Profile(it->first, it->second.CopyandCorrect(exclude_elements,false,om,psize,nullptr,elementinfo));
        }
    }
    return out;
}

Elemental_Profile_Set Elemental_Profile_Set::Extract(const vector<string> &element_list) const
{
    Elemental_Profile_Set out;
    for (map<string,Elemental_Profile>::const_iterator it=cbegin(); it!=cend(); it++)
    {
        out.Append_Profile(it->first, it->second.Extract(element_list));
    }
    return out;
}

Elemental_Profile_Set Elemental_Profile_Set::CopyIncludedinAnalysis(bool applyomsizecorrection, const double &om, const double &size, map<string, element_information> *elementinfo)
{
    Elemental_Profile_Set out;
    if (applyomsizecorrection)
    {
        out = OrganicandSizeCorrect(om,size);
    }
    else
    {
        for (map<string,Elemental_Profile>::iterator it=begin(); it!=end(); it++)
            if (it->second.IncludedInAnalysis())
                out.Append_Profile(it->first, it->second);
    }

    return out;
}

void Elemental_Profile_Set::UpdateElementalDistribution()
{
    element_distributions.clear();
    for (map<string,Elemental_Profile>::iterator profile=begin(); profile!=end();profile++)
    {
        for (map<string,double>::const_iterator it=profile->second.begin(); it!=profile->second.end(); it++)
        {
            element_distributions[it->first].push_back(it->second);
        }
    }
}

Elemental_Profile *Elemental_Profile_Set::Append_Profile(const string &name, const Elemental_Profile &profile, map<string, element_information> *elementinfo)
{
    if (count(name)>0)
    {
        cout<<"Profile '" + name + "' already exists!"<<std::endl;
        return nullptr;
    }
    else
    {
        operator[](name) = profile.CopyIncluded(elementinfo);
    }
    for (map<string,double>::const_iterator it=profile.begin(); it!=profile.end(); it++)
    {
        if (elementinfo==nullptr)
            element_distributions[it->first].push_back(it->second);
        else if (elementinfo->at(it->first).include_in_analysis && elementinfo->at(it->first).Role != element_information::role::do_not_include)
            element_distributions[it->first].push_back(it->second);
    }
    return &operator[](name);
}

vector<string> Elemental_Profile_Set::SampleNames()
{
    vector<string> out;
    for (map<string,Elemental_Profile>::iterator it=begin(); it!=end(); it++)
    {
        out.push_back(it->first);
    }
    return out;
}

string Elemental_Profile_Set::ToString()
{
    string out; 
    if (size() == 0) return string(); 
    out += "Element name \t";
    for (map<string, Elemental_Profile>::iterator it = begin(); it != end(); it++)
    {
        out += it->first + "\t";
    }
    out += "\n";
    vector<string> elements = ElementNames(); 
    for (int i = 0; i < elements.size(); i++)
    {
        out += elements[i];
        for (map<string, Elemental_Profile>::iterator it = begin(); it != end(); it++)
        {
            out += "\t" + aquiutils::numbertostring(it->second.at(elements[i])); 
        }
        out += "\n";
    }
    return out; 
}

bool Elemental_Profile_Set::writetofile(QFile* file)
{
    file->write(QString::fromStdString(ToString()).toUtf8());
    return 0;
}

QJsonObject Elemental_Profile_Set::toJsonObject()
{
    string out;
    QJsonObject json_object;
    if (size() == 0) return QJsonObject();
    for (map<string, Elemental_Profile>::iterator it = begin(); it != end(); it++)
    {
        json_object[QString::fromStdString(it->first)] = it->second.toJsonObject(); 
    }
    return json_object;
    
}

bool Elemental_Profile_Set::ReadFromJsonObject(const QJsonObject &jsonobject)
{
    clear();
    for(QString key: jsonobject.keys() ) {
        Elemental_Profile elemental_profile;
        elemental_profile.ReadFromJsonObject(jsonobject[key].toObject());
        Append_Profile(key.toStdString(), elemental_profile);
    }
    return true;
}

bool Elemental_Profile_Set::Read(const QStringList &strlist)
{


    return true;
}

bool Elemental_Profile_Set::ContainsElement(const string &elementname)
{
    if (size()==0)
        return false;

    bool out=true;
    for (map<string, Elemental_Profile>::iterator it = begin(); it != end(); it++)
    {
        out &= it->second.contains(elementname);

    }
    return out;

}

vector<string> Elemental_Profile_Set::ElementNames()
{
    vector<string> out; 
    if (size() == 0)
        return vector<string>(); 
    for (map<string, double>::iterator it = operator[](SampleNames()[0]).begin(); it != operator[](SampleNames()[0]).end(); it++)
        out.push_back(it->first);
    return out; 
}


Elemental_Profile *Elemental_Profile_Set::Profile(const string &name)
{
    if (count(name)==0)
    {
        cout<<"Sample '" + name + "' does not exist!"<<std::endl;
        return nullptr;
    }
    else
        return &operator[](name);

}

Elemental_Profile *Elemental_Profile_Set::Profile(unsigned int i)
{
    if (i>=size())
        return nullptr;
    else
    {
        map<string,Elemental_Profile>::iterator it=begin();
        for (int ii=0; ii<i; ii++)
            it++;
        return &it->second;
    }

}

Elemental_Profile Elemental_Profile_Set::Profile(const string &name) const
{
    if (count(name)==0)
    {
        cout<<"Sample '" + name + "' does not exist!"<<std::endl;
        return Elemental_Profile();
    }
    else
        return at(name);

}

Elemental_Profile Elemental_Profile_Set::Profile(unsigned int i) const
{
    if (i>=size())
        return Elemental_Profile();
    else
    {
        map<string,Elemental_Profile>::const_iterator it=begin();
        for (int ii=0; ii<i; ii++)
            it++;
        return it->second;
    }

}


vector<double> Elemental_Profile_Set::GetAllConcentrationsFor(const string &element_name)
{
    vector<double> out;
    if (begin()->second.count(element_name)==0)
    {
        return out;
    }
    for (map<string, Elemental_Profile>::iterator it = begin(); it != end(); it++)
    {
        out.push_back(it->second.at(element_name));
    }
    return out;
}


vector<double> Elemental_Profile_Set::GetProfileForSample(const string &sample_name)
{
    if (!Profile(sample_name))
        return vector<double>();

    return Profile(sample_name)->Vals();
}

double Elemental_Profile_Set::max()
{
    double _max = -1e12;
    for (map<string, Elemental_Profile>::iterator it = begin(); it != end(); it++)
    {
        if (it->second.max() > _max) _max = it->second.max(); 
    }
    return _max; 
}

double Elemental_Profile_Set::min()
{
    double _min = 1e12;
    for (map<string, Elemental_Profile>::iterator it = begin(); it != end(); it++)
    {
        if (it->second.min() < _min) _min = it->second.min();
    }
    return _min;
}

MultipleLinearRegressionSet Elemental_Profile_Set::regress_vs_size_OM(const string &om, const string &d,regression_form form)
{
    MultipleLinearRegressionSet out;
    vector<string> elementnames = ElementNames();
    for (unsigned int i=0; i<elementnames.size(); i++)
    {
        out[elementnames[i]] = regress_vs_size_OM(elementnames[i],om,d,form);
        out[elementnames[i]].SetDependentVariableName(elementnames[i]);
    }
    return out;
}

MultipleLinearRegression Elemental_Profile_Set::regress_vs_size_OM(const string &element, const string &om, const string &d, regression_form form)
{
    MultipleLinearRegression out;
    vector<double> dependent = GetAllConcentrationsFor(element);
    vector<vector<double>> independents;
    independents.push_back(GetAllConcentrationsFor(om));
    independents.push_back(GetAllConcentrationsFor(d));
    vector<string> independent_var_names = {om,d};
    out.SetEquation(form);
    out.Regress(independents,dependent, independent_var_names);
    return out;
}

void Elemental_Profile_Set::SetRegression(const string &om, const string &d, regression_form form)
{
    mlr_vs_om_size = regress_vs_size_OM(om,d,form);
}

void Elemental_Profile_Set::SetRegression(const MultipleLinearRegressionSet *mlrset)
{
    mlr_vs_om_size = *mlrset;
}

ResultItem Elemental_Profile_Set::GetRegressions()
{
    ResultItem out;
    out.SetResult(new MultipleLinearRegressionSet(mlr_vs_om_size));
    out.SetType(result_type::mlrset);

    return out;
}

MultipleLinearRegressionSet* Elemental_Profile_Set::GetExistingRegressionSet()
{
    return &mlr_vs_om_size;
}

CMBMatrix Elemental_Profile_Set::CovarianceMatrix()
{
    CMBMatrix out(ElementNames().size());
    //double gsl_stats_covariance(const double data1[], const size_t stride1, const double data2[], const size_t stride2, const size_t n)

    gsl_matrix *X = CopytoGSLMatrix();

    vector<string> element_names = ElementNames();
    for (int i = 0; i < X->size2; i++)
    {
        out.SetColumnLabel(i,element_names[i]);
        out.SetRowLabel(i,element_names[i]);
    }


    for (int i = 0; i < X->size2; i++) {
            for (int j = i; j < X->size2; j++) {
              gsl_vector_view a = gsl_matrix_column (X, i);
              gsl_vector_view b = gsl_matrix_column (X, j);
              double cov = gsl_stats_covariance(a.vector.data, a.vector.stride,b.vector.data, b.vector.stride, a.vector.size);
              out[i][j]=cov;
              out[j][i]=cov;
            }
          }
    return out;
}

CMBMatrix Elemental_Profile_Set::CorrelationMatrix()
{
    CMBMatrix out(ElementNames().size());
    //double gsl_stats_covariance(const double data1[], const size_t stride1, const double data2[], const size_t stride2, const size_t n)

    gsl_matrix *X = CopytoGSLMatrix();

    vector<string> element_names = ElementNames();
    for (int i = 0; i < X->size2; i++)
    {
        out.SetColumnLabel(i,element_names[i]);
        out.SetRowLabel(i,element_names[i]);
    }
    for (int i = 0; i < X->size2; i++) {
            for (int j = i; j < X->size2; j++) {
              gsl_vector_view a = gsl_matrix_column (X, i);
              gsl_vector_view b = gsl_matrix_column (X, j);
              double cov = gsl_stats_correlation(a.vector.data, a.vector.stride,b.vector.data, b.vector.stride, a.vector.size);
              out[i][j]=cov;
              out[j][i]=cov;
            }
          }
    return out;
}


gsl_matrix* Elemental_Profile_Set::CopytoGSLMatrix()
{
    gsl_matrix *out;
    out = gsl_matrix_alloc (this->size(),ElementNames().size());
    int i=0;
    for (map<string, Elemental_Profile>::iterator it = begin(); it != end(); it++)
    {
        int j=0;
        for (map<string,double>::iterator element = it->second.begin(); element!=it->second.end(); element++)
        {
            gsl_matrix_set (out, i, j, element->second);
            j++;
        }
        i++;
    }
    return out;

}

CMBVector Elemental_Profile_Set::KolmogorovSmirnovStat(distribution_type dist_type)
{
    vector<string> element_names = ElementNames();
    CMBVector out(element_names.size());
    out.SetLabels(element_names);
    for (int i = 0; i < element_names.size(); i++)
    {
        out[i] = ElementalDistribution(element_names[i])->KolmogorovSmirnovStat(dist_type);
    }
    return out; 
}

CVector Elemental_Profile_Set::ElementMeans()
{
    vector<string> element_names = ElementNames();
    CVector out(element_names.size());
    for (int i=0; i<element_names.size(); i++)
    {
        out[i] = element_distributions[element_names[i]].mean();
    }
    return out;
}

QTableWidget *Elemental_Profile_Set::ToTable()
{
    QTableWidget *tablewidget = new QTableWidget();
    tablewidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablewidget->setColumnCount(size());
    vector<string> element_names = ElementNames();
    tablewidget->setRowCount(element_names.size());
    QStringList headers;
    QStringList constituents;
    for (int j=0; j<element_names.size(); j++)
        constituents<<QString::fromStdString(element_names[j]);
    int i=0;
    for (map<string,Elemental_Profile>::const_iterator it=cbegin(); it!=cend(); it++ )
    {
        for (int j=0; j<element_names.size(); j++)
            tablewidget->setItem(j,i, new QTableWidgetItem(QString::number(it->second.Val(element_names[j]))));
        i++;
        headers << QString::fromStdString(it->first);
    }

    tablewidget->setHorizontalHeaderLabels(headers);
    tablewidget->setVerticalHeaderLabels(constituents);
    return tablewidget;
}

Elemental_Profile_Set Elemental_Profile_Set::OrganicandSizeCorrect(const double &size, const double &om)
{
    Elemental_Profile_Set out = *this;
    out.clear();
    for (map<string,Elemental_Profile>::iterator it=begin(); it!=end(); it++)
    {
        if (it->second.IncludedInAnalysis())
            out.Append_Profile(it->first, it->second.OrganicandSizeCorrect(size,om,&mlr_vs_om_size));

    }
    return out;
}

CMBVector Elemental_Profile_Set::BoxCoxParameters()
{
    CMBVector out(element_distributions.size());
    int i=0;
    for (map<string, ConcentrationSet>::iterator it=element_distributions.begin(); it!=element_distributions.end(); it++)
    {
        out[i] = it->second.OptimalBoxCoxParam(-5,5,10);
        out.SetLabel(i,it->first);
        i++;
    }
    return out;
}

CMBMatrix Elemental_Profile_Set::Outlier()
{
    CMBVector lambdas = BoxCoxParameters();
    CMBMatrix outliermagnitude(begin()->second.size(),this->size());
    vector<double> means;
    vector<double> stds;
    vector<double> std_orig; 
    int i=0;
    for (map<string, ConcentrationSet>::iterator it=element_distributions.begin(); it!=element_distributions.end(); it++)
    {
        means.push_back(it->second.BoxCoxTransform(lambdas[i]).mean());
        stds.push_back(it->second.BoxCoxTransform(lambdas[i]).stdev());
        std_orig.push_back(it->second.stdev());
        i++;
    }

    i=0;
    for (map<string,Elemental_Profile>::iterator it=begin(); it!=end(); it++)
    {
        int j=0;
        outliermagnitude.SetRowLabel(i,it->first);
        for (map<string,double>::iterator element = it->second.begin(); element!=it->second.end(); element++)
        {
            outliermagnitude.SetColumnLabel(j,element->first);
            qDebug() << element->second << "," << (pow(element->second,lambdas[j])-1.0)/lambdas[j] << "," << means[j] << "," <<stds[j];
            outliermagnitude[i][j] = ((pow(element->second/std_orig[j], lambdas[j]) - 1.0) / lambdas[j] - means[j]) / stds[j];
            j++;
        }
        i++;
    }
    return outliermagnitude;
}

Elemental_Profile_Set Elemental_Profile_Set::BocCoxTransformed(CMBVector *lambda_vals)
{
    CMBVector lambdas;
    if (lambda_vals==nullptr)
        lambdas = BoxCoxParameters();
    else
    {
        lambdas = *lambda_vals;
    }
    Elemental_Profile_Set out(*this);


    int i=0;

    for (map<string,ConcentrationSet>::iterator it=element_distributions.begin(); it!=element_distributions.end(); it++)
    {
        ConcentrationSet boxcoxtransformed = it->second.BoxCoxTransform(lambdas[i]);
        int j=0;
        for (map<string,Elemental_Profile>::iterator profile=out.begin();profile!=out.end(); profile++)
        {   profile->second[it->first] = boxcoxtransformed[j];
            j++;
        }
        i++;
    }
    return out;
}


CMBMatrix Elemental_Profile_Set::toMatrix()
{
    vector<string> element_names = ElementNames();
    CMBMatrix out(element_names.size(),size());
    int i=0;

    for (map<string,Elemental_Profile>::iterator it=begin(); it!=end(); it++)
    {
        int j=0;
        out.SetRowLabel(i,it->first);
        for (map<string,double>::iterator element = it->second.begin(); element!=it->second.end(); element++)
        {
            out[i][j] = element->second;
            out.SetColumnLabel(j,element->first);
            j++;
        }
        i++;
    }
    return out;
}
