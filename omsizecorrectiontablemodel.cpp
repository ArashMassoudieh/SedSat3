#include "omsizecorrectiontablemodel.h"

OmSizeCorrectionTableModel::OmSizeCorrectionTableModel(MultipleLinearRegressionSet *_mlrset, QObject *parent) : QAbstractTableModel(parent)
{
    MLRset = _mlrset;
}
int OmSizeCorrectionTableModel::rowCount(const QModelIndex &index) const
{
    return MLRset->size();
}
int OmSizeCorrectionTableModel::columnCount(const QModelIndex &index) const
{
    return 8;
}
QVariant OmSizeCorrectionTableModel::data(const QModelIndex &index, int role) const
{
    MultipleLinearRegression *mlr = &MLRset->operator[](MLRset->Key(index.row()));
    if (role == Qt::DisplayRole)
    {   if (index.column()==0)
        {
            return QString::fromStdString(MLRset->Key(index.row()));
        }
        if (index.column()==1)
        {
            return mlr->CoefficientsIntercept()[0];
        }
        if (index.column()==2)
        {
            return mlr->CoefficientsIntercept()[1];
        }
        if (index.column()==3)
        {
            return mlr->P_Value()[0];
        }
        if (index.column()==4)
        {
            if (mlr->Effective(0))
                return "Yes";
            else
                return "No";
        }
        if (index.column()==5)
        {
            return mlr->CoefficientsIntercept()[2];
        }
        if (index.column()==6)
        {
            return mlr->P_Value()[1];
        }
        if (index.column()==7)
        {
            if (mlr->Effective(1))
                return "Yes";
            else
                return "No";
        }
    }
    if (role == Qt::ItemIsEnabled)
    {
        if (index.column()==7 || index.column()==4)
            return true;
        else
            return false;
    }

    if (role == Qt::ForegroundRole)
    {
        if (index.column()==3 && mlr->P_Value()[0]<0.05)
            return QVariant::fromValue(QColor(Qt::red));
        else if (index.column()==6 && mlr->P_Value()[1]<0.05)
            return QVariant::fromValue(QColor(Qt::red));
        else
            return QVariant::fromValue(QColor(Qt::black));
    }

    return QVariant();

}
QVariant OmSizeCorrectionTableModel::headerData(int section, Qt::Orientation orientation, int role) const
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
                return "Intercept";
            }
            if (section==2)
            {
                return "Coefficient for OM";
            }
            if (section==3)
            {
                return "P-value for OM";
            }
            if (section==4)
            {
                return "Correct based on OM";
            }
            if (section==5)
            {
                return "Coefficient for size";
            }
            if (section==6)
            {
                return "P-value for size";
            }
            if (section==7)
            {
                return "Correct based on size";
            }
        }
    }
    return QVariant();
}
bool OmSizeCorrectionTableModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    MultipleLinearRegression *mlr = &MLRset->operator[](MLRset->Key(index.row()));
    if (role==Qt::EditRole)
    {
        if (index.column()==4)
        {
            if (value.toString()=="Yes" || value.toBool())
                mlr->SetEffective(0,true);
            else
                mlr->SetEffective(0,false);
        }
        if (index.column()==7)
        {
            if (value.toString()=="Yes" || value.toBool())
                mlr->SetEffective(1,true);
            else
                mlr->SetEffective(1,false);
        }

    }
    QString result = "Done";
    emit editCompleted(result);
    return true;
}
Qt::ItemFlags OmSizeCorrectionTableModel::flags(const QModelIndex & index) const
{
    if (index.column()!=4 && index.column()!=7)
        return Qt::ItemIsSelectable |  Qt::ItemIsEnabled ;
    else
        return Qt::ItemIsSelectable |  Qt::ItemFlag::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsEditable ;

}

