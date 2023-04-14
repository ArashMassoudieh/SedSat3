#ifndef CMBMATRIX_H
#define CMBMATRIX_H

#include "Matrix.h"
#include "interface.h"
#include "range.h"

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
    void SetColumnLabels(const vector<string> &label) {columnlabels=label;}
    void SetRowLabels(const vector<string> &label) {rowlabels=label;}
    QTableWidget *ToTable() override;
    void SetLimit(_range lowhigh, const double &value)
    {
        if (lowhigh == _range::high)
            highlimit = value;
        else
            lowlimit = value;
        highlightoutsideoflimit = true;
    }
private:
    vector<string> columnlabels;
    vector<string> rowlabels;
    double lowlimit,highlimit;
    bool highlightoutsideoflimit=false;
};

#endif // CMBMATRIX_H
