#ifndef SELECTSAMPLETABLEMODEL_H
#define SELECTSAMPLETABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include <sourcesinkdata.h>


class SelectSampleTableModel: public QAbstractTableModel
{
private:
    Q_OBJECT
    string selectedSource;

public:
    SelectSampleTableModel(SourceSinkData *Data, QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex & index) const override;
    SourceSinkData *Data;
    void SetSelectedSource(const string &group) {selectedSource = group;}
    Elemental_Profile* GetProfileSet(int row);
    Elemental_Profile* GetProfileSet(int row) const;
signals:
     void editCompleted(const QString &);

};

#endif // SELECTSAMPLETABLEMODEL_H

