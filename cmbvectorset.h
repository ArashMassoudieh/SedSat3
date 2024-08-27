#ifndef CMBVECTORSET_H
#define CMBVECTORSET_H

#include "interface.h"
#include "cmbvector.h"
#include <map>

class CMBVectorSet: public Interface,  public map<string,CMBVector>
{
public:
    CMBVectorSet();
    CMBVectorSet(const CMBVectorSet& mp);
    CMBVectorSet& operator=(const CMBVectorSet &mp);
    QJsonObject toJsonObject() override;
    bool ReadFromJsonObject(const QJsonObject &jsonobject) override;
    string ToString() override;
    QTableWidget *ToTable() override;
    bool writetofile(QFile* file) override;
    string Label(string column,int j) const;
    double valueAt(const string &columnlabel, int j ) const;
    CMBVector &GetColumn(const string columnlabel);
    void Append(const string &columnlabel,const CMBVector &vectorset);
    unsigned int MaxSize() const;
    double max() const;
    double min() const;
private:


};

#endif // CMBVECTORSET_H
