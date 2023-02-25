#ifndef CMBMATRIX_H
#define CMBMATRIX_H

#include "Matrix.h"
#include "interface.h"

class CMBMatrix : public CMatrix, public Interface
{
public:
    CMBMatrix();
    CMBMatrix(int n);
    CMBMatrix(int n, int m);
    CMBMatrix(const CMBMatrix& mp);
    CMBMatrix& operator=(const CMBMatrix &mp);
    QJsonObject toJsonObject() override;
    bool ReadFromJsonObject(const QJsonObject &jsonobject) override;
    string ToString() override;
    bool writetofile(QFile* file) override;
    double valueAt(int i, int j);
    string ColumnLabel(int i) {return columnlabels[i];}
    string RowLabel(int i) {return rowlabels[i];}
    void SetColumnLabel(int i, const string &label) {columnlabels[i]=label;}
    void SetRowLabel(int i, const string &label) {rowlabels[i]=label;}
private:
    vector<string> columnlabels;
    vector<string> rowlabels;
};

#endif // CMBMATRIX_H
