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
    return 4;
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

        }
        if (index.column()==2)
        {
            return QString::fromStdString(Data->GetElementInformation(Data->ElementNames()[index.row()])->base_element);
        }
        if (index.column()==3)
        {
            return QString::number(Data->GetElementInformation(Data->ElementNames()[index.row()])->standard_ratio);
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
                return "Isotope";
            }
            if (section==2)
            {
                return "Base Element";
            }
            if (section==3)
            {
                return "Standard isotope ratio";
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

        }
        if (index.column()==2)
        {
            Data->GetElementInformation(element)->base_element = value.toString().toStdString();
        }
        if (index.column()==3)
        {
            Data->GetElementInformation(element)->standard_ratio = value.toDouble();
        }

    }
    QString result = "Done";
    emit editCompleted(result);
}
Qt::ItemFlags ElementTableModel::flags(const QModelIndex & index) const
{
    if (index.column()!=0)
        return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled ;
    else
        return Qt::ItemIsSelectable |  Qt::ItemIsEnabled ;
}
