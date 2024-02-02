#include "elementstablemodel.h"

ElementTableModel::ElementTableModel(SourceSinkData *_data, QObject *parent) : QAbstractTableModel(parent)
{
    Data = _data;
}
int ElementTableModel::rowCount(const QModelIndex &index) const
{
    return Data->ElementNames().size();;
}
int ElementTableModel::columnCount(const QModelIndex &index) const
{
    return 5;
}
QVariant ElementTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {   if (index.column()==0)
        {
            return QString::fromStdString(Data->ElementNames()[index.row()]);
        }
        if (index.column()==1)
        {
            if (Data->GetElementInformation(Data->ElementNames()[index.row()])->Role == element_information::role::do_not_include)
                return "Exclude";
            if (Data->GetElementInformation(Data->ElementNames()[index.row()])->Role == element_information::role::element)
                return "Element";
            if (Data->GetElementInformation(Data->ElementNames()[index.row()])->Role == element_information::role::isotope)
                return "Isotope";
            if (Data->GetElementInformation(Data->ElementNames()[index.row()])->Role == element_information::role::particle_size)
                return "Particle Size";
            if (Data->GetElementInformation(Data->ElementNames()[index.row()])->Role == element_information::role::organic_carbon)
                return "Organic Carbon";

        }
        if (index.column()==2)
        {
            if (Data->GetElementInformation(Data->ElementNames()[index.row()])->Role == element_information::role::isotope)
                return QString::fromStdString(Data->GetElementInformation(Data->ElementNames()[index.row()])->base_element);
            else
                return "";
        }
        if (index.column()==3)
        {
            if (Data->GetElementInformation(Data->ElementNames()[index.row()])->Role == element_information::role::isotope)
                return QString::number(Data->GetElementInformation(Data->ElementNames()[index.row()])->standard_ratio);
            else
                return "";
        }
        if (index.column()==4)
        {
            if (Data->GetElementInformation(Data->ElementNames()[index.row()])->include_in_analysis)
                return "Yes";
            else
                return "No";
        }
    }
    if (role == Qt::ItemIsEnabled)
    {
        if (index.column()==2 || index.column()==3)
        {   if (Data->GetElementInformation(Data->ElementNames()[index.row()])->Role == element_information::role::isotope)
                return true;
            else
                return false;
        }
    }
    return QVariant();

}
QVariant ElementTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {   if (section==0)
            {
                return "Constituent";
            }
            if (section==1)
            {
                return "Constituent Type";
            }
            if (section==2)
            {
                return "Base Element";
            }
            if (section==3)
            {
                return "Standard isotope ratio";
            }
            if (section==4)
            {
                return "Include in analysis";
            }
        }
    }
    return QVariant();
}
bool ElementTableModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (role==Qt::EditRole)
    {

        string element = Data->ElementNames()[index.row()];
        if (index.column()==1)
        {
            if (value.toString()=="Element")
                Data->GetElementInformation(element)->Role = element_information::role::element;
            if (value.toString()=="Isotope")
                Data->GetElementInformation(element)->Role = element_information::role::isotope;
            if (value.toString()=="Exclude")
                Data->GetElementInformation(element)->Role = element_information::role::do_not_include;
            if (value.toString()=="Particle Size")
                Data->GetElementInformation(element)->Role = element_information::role::particle_size;
            if (value.toString()=="Organic Carbon")
                Data->GetElementInformation(element)->Role = element_information::role::organic_carbon;

        }
        if (index.column()==2)
        {
            Data->GetElementInformation(element)->base_element = value.toString().toStdString();
        }
        if (index.column()==3)
        {
            Data->GetElementInformation(element)->standard_ratio = value.toDouble();
        }
        if (index.column()==4)
        {
            if (value.toString()=="Yes" || value.toBool())
                Data->GetElementInformation(element)->include_in_analysis = true;
            else
                Data->GetElementInformation(element)->include_in_analysis = false;
        }

    }
    QString result = "Done";
    emit editCompleted(result);
    return true; 
}
Qt::ItemFlags ElementTableModel::flags(const QModelIndex & index) const
{
    if (index.column()!=0 && index.column()!=4)
        return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled ;
    else if (index.column()==4)
        return Qt::ItemIsSelectable |  Qt::ItemFlag::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsEditable ;
    else
        return Qt::ItemIsSelectable |  Qt::ItemIsEnabled ;
}
