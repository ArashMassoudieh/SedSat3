#ifndef CMBVector_H
#define CMBVector_H

#include "Vector.h"
#include "interface.h"

class CMBVector : public CVector, public Interface
{
public:
    CMBVector();
    CMBVector(int n);
    CMBVector(const CMBVector& mp);
    CMBVector& operator=(const CMBVector &mp);
    CMBVector(const CVector& mp);
    CMBVector& operator=(const CVector &mp);
    QJsonObject toJsonObject() override;
    bool ReadFromJsonObject(const QJsonObject &jsonobject) override;
    string ToString() override;
    QTableWidget *ToTable() override;
    bool writetofile(QFile* file) override;
    double valueAt(int i);
    string Label(int i) {return labels[i];}
    vector<string> Labels() const {return labels;}
    void SetLabel(int i, const string &label) {labels[i]=label;}
    void SetLabels(const vector<string> &label) {labels=label;}

private:
    vector<string> labels;

};

#endif // CMBVector_H
