#ifndef OMSIZECORRECTIONTABLEMODEL_H
#define OMSIZECORRECTIONTABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include <multiplelinearregressionset.h>

class OmSizeCorrectionTableModel: public QAbstractTableModel
{
private:
    Q_OBJECT


public:
    OmSizeCorrectionTableModel(MultipleLinearRegressionSet *MLRset, QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex & index) const override;
    MultipleLinearRegressionSet *MLRset;
signals:
     void editCompleted(const QString &);

};

#endif // OMSIZECORRECTIONTABLEMODEL_H



