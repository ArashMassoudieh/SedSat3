#ifndef CMBVectorSetSetSET_H
#define CMBVectorSetSetSET_H

#include "interface.h"
#include "cmbvectorset.h"
#include <map>

class CMBVectorSetSet: public Interface,  public map<string,CMBVectorSet>
{
public:
    CMBVectorSetSet();
    CMBVectorSetSet(const CMBVectorSetSet& mp);
    CMBVectorSetSet& operator=(const CMBVectorSetSet &mp);
    QJsonObject toJsonObject() const override;
    bool ReadFromJsonObject(const QJsonObject &jsonobject) override;
    string ToString() const override;
    QTableWidget *ToTable() override;
    bool writetofile(QFile* file) override;
    string Label(const string &vectorset,const string &column, int j) const;
    double valueAt(const string &vectorset, const string &columnlabel, int j ) const;
    CMBVector &GetColumn(const string &vectorset, const string &columnlabel);
    void Append(const string &columnlabel,const CMBVectorSet &vectorset);
    unsigned int MaxSize() const;
    double max() const;
    double min() const;

private:


};



#endif // CMBVectorSetSetSET_H
