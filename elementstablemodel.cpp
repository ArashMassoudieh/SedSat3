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
    if (index.column()==0)
    {
        return QString::fromStdString(Data->ElementNames()[index.row()]);
    }
    if (index.column()==1)
    {
        return Data->GetElementInformation(Data->ElementNames()[index.row()])->isotope;
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

}
Qt::ItemFlags ElementTableModel::flags(const QModelIndex & index) const
{

}
