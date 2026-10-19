// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#include "elementstablemodel.h"
#include "numericparsing.h"

ElementTableModel::ElementTableModel(SourceSinkData *_data, QObject *parent) : QAbstractTableModel(parent)
{
    Data = _data;
}
int ElementTableModel::rowCount(const QModelIndex &/*index*/) const
{
    return Data->GetElementNames().size();;
}
int ElementTableModel::columnCount(const QModelIndex &/*index*/) const
{
    return 5;
}
QVariant ElementTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {   if (index.column()==0)
        {
            return QString::fromStdString(Data->GetElementNames()[index.row()]);
        }
        if (index.column()==1)
        {
            if (Data->GetElementInformation(Data->GetElementNames()[index.row()])->Role == element_information::role::do_not_include)
                return "Exclude";
            if (Data->GetElementInformation(Data->GetElementNames()[index.row()])->Role == element_information::role::element)
                return "Element";
            if (Data->GetElementInformation(Data->GetElementNames()[index.row()])->Role == element_information::role::isotope)
                return "Isotope";
            if (Data->GetElementInformation(Data->GetElementNames()[index.row()])->Role == element_information::role::particle_size)
                return "Particle Size";
            if (Data->GetElementInformation(Data->GetElementNames()[index.row()])->Role == element_information::role::organic_carbon)
                return "Organic Carbon";

        }
        if (index.column()==2)
        {
            if (Data->GetElementInformation(Data->GetElementNames()[index.row()])->Role == element_information::role::isotope)
                return QString::fromStdString(Data->GetElementInformation(Data->GetElementNames()[index.row()])->base_element);
            else
                return "";
        }
        if (index.column()==3)
        {
            if (Data->GetElementInformation(Data->GetElementNames()[index.row()])->Role == element_information::role::isotope)
                return QString::number(Data->GetElementInformation(Data->GetElementNames()[index.row()])->standard_ratio);
            else
                return "";
        }
        if (index.column()==4)
        {
            if (Data->GetElementInformation(Data->GetElementNames()[index.row()])->include_in_analysis)
                return "Yes";
            else
                return "No";
        }
    }
    if (role == Qt::ItemIsEnabled)
    {
        if (index.column()==2 || index.column()==3)
        {   if (Data->GetElementInformation(Data->GetElementNames()[index.row()])->Role == element_information::role::isotope)
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

        string element = Data->GetElementNames()[index.row()];
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
            // Refuse an edit that is not a finite number, leaving the old value
            double ratio = 0;
            QString reason;
            if (!parseNumericValue(value, ratio, reason))
                return false;
            Data->GetElementInformation(element)->standard_ratio = ratio;
        }
        if (index.column()==4)
        {
            // QVariant("No").toBool() is true, so text is compared explicitly
            const bool include = (value.typeId() == QMetaType::Bool)
                ? value.toBool()
                : value.toString().compare("Yes", Qt::CaseInsensitive) == 0 || value.toString() == "true" || value.toString() == "1";
            Data->GetElementInformation(element)->include_in_analysis = include;
        }

    }
    QString result = "Done";
    emit dataChanged(index, index);  // so the view shows the change at once
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
