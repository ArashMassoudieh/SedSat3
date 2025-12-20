#ifndef CMBMATRIX_H
#define CMBMATRIX_H

#include "Matrix.h"
#include "interface.h"
#include "range.h"
#include "cmbvector.h"

class CMBMatrix : public CMatrix, public Interface
{
public:
    CMBMatrix();
    CMBMatrix(int n);
    CMBMatrix(int n, int m);
    CMBMatrix(const CMBMatrix& mp);
    CMatrix toMatrix() const;
    CMBMatrix& operator=(const CMBMatrix &mp);
    QJsonObject toJsonObject() const override;
    bool ReadFromJsonObject(const QJsonObject &jsonobject) override;
    string ToString() const override;
    bool writetofile(QFile* file) override;
    double valueAt(int i, int j) const;
    string ColumnLabel(int i) {return columnlabels[i];}
    string RowLabel(int i) {return rowlabels[i];}
    void SetColumnLabel(int i, const string &label) {columnlabels[i]=label;}
    void SetRowLabel(int i, const string &label) {rowlabels[i]=label;}
    void SetColumnLabels(const vector<string> &label) {columnlabels=label;}
    void SetRowLabels(const vector<string> &label) {rowlabels=label;}
    QTableWidget *ToTable() override;
    vector<string> RowLabels() const {return rowlabels;}
    vector<string> ColumnLabels() const {return columnlabels;}
    CMBVector GetRow(const string &rowlabel);
    CMBVector GetColumn(const string &rowlabel);
    QStringList RowLabelCategories();
    void SetBooleanValue(bool val) {boolean_values = val;}
private:
    vector<string> columnlabels;
    vector<string> rowlabels;
    bool boolean_values = false;

};

CMBVector operator*(const CMBMatrix &M, const CMBVector&V);

#endif // CMBMATRIX_H
