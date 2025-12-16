#include "selectsampletablemodel.h"

SelectSampleTableModel::SelectSampleTableModel(SourceSinkData *_data, QObject *parent) : QAbstractTableModel(parent)
{
    Data = _data;
}
int SelectSampleTableModel::rowCount(const QModelIndex &index) const
{
    return Data->operator[](selectedSource).size();
}
int SelectSampleTableModel::columnCount(const QModelIndex &index) const
{
    return 3;
}
QVariant SelectSampleTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {   if (index.column()==0)
        {
            return QString::fromStdString(Data->operator[](selectedSource).SampleNames()[index.row()]);
        }
        if (index.column()==1)
        {
            if (GetProfileSet(index.row())->IsIncludedInAnalysis())
                return "Yes";
            else
                return "No";
        }
        if (index.column()==2)
        {
            return QString::fromStdString(GetProfileSet(index.row())->GetNotes());
        }

    }
    if (role == Qt::ForegroundRole)
    {
        if (GetProfileSet(index.row())->GetNotes()!="")
            return QVariant::fromValue(QColor(Qt::red));
        else
            return QVariant::fromValue(QColor(Qt::black));
    }

    return QVariant();

}

Elemental_Profile* SelectSampleTableModel::GetProfileSet(int row)
{
    return &Data->operator[](selectedSource)[Data->operator[](selectedSource).SampleNames()[row]];
}

Elemental_Profile* SelectSampleTableModel::GetProfileSet(int row) const
{
    return &Data->operator[](selectedSource)[Data->operator[](selectedSource).SampleNames()[row]];
}
QVariant SelectSampleTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {   if (section==0)
            {
                return "Sample";
            }
            if (section==1)
            {
                return "Include";
            }
            if (section==2)
            {
                return "Notes";
            }
        }
    }
    return QVariant();
}
bool SelectSampleTableModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (role==Qt::EditRole)
    {

        if (index.column()==1)
        {
            if (value.toString()=="Yes" || value.toBool())
                GetProfileSet(index.row())->SetIncludedInAnalysis(true);
            else
                GetProfileSet(index.row())->SetIncludedInAnalysis(false);
        }

    }
    QString result = "Done";
    emit editCompleted(result);
    return true;
}
Qt::ItemFlags SelectSampleTableModel::flags(const QModelIndex & index) const
{
    if (index.column()!=0)
        return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled ;
    else
        return Qt::ItemIsSelectable |  Qt::ItemIsEnabled ;
}
