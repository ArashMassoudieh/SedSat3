#ifndef ELEMENTSTABLEMODEL_H
#define ELEMENTSTABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include <sourcesinkdata.h>

class ElementTableModel: public QAbstractTableModel
{
private:
    Q_OBJECT

public:
    ElementTableModel(SourceSinkData *Data, QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex & index) const override;
    SourceSinkData *Data;
private:

signals:
     void editCompleted(const QString &);
};

#endif // ELEMENTSTABLEMODEL_H
