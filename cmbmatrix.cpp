#include "cmbmatrix.h"
#include "QJsonArray"
#include "Vector.h"
#include <QFile>

CMBMatrix::CMBMatrix():CMatrix(),Interface()
{

}

CMBMatrix::CMBMatrix(const CMBMatrix& mp):CMatrix(mp)
{
    columnlabels = mp.columnlabels;
    rowlabels = mp.rowlabels;
}

CMBMatrix::CMBMatrix(int n):CMatrix(n), Interface()
{
    columnlabels.resize(n);
    rowlabels.resize(n);
}
CMBMatrix::CMBMatrix(int n, int m):CMatrix(m,n), Interface()
{
    rowlabels.resize(n);
    columnlabels.resize(m);
}

CMBMatrix& CMBMatrix::operator=(const CMBMatrix &mp)
{
    columnlabels = mp.columnlabels;
    rowlabels = mp.rowlabels;
    CMatrix::operator=(mp);
    return *this;
}
QJsonObject CMBMatrix::toJsonObject()
{
    QJsonObject out;
    QJsonArray matrix;
    for (int i=0; i<0; i<getnumrows())
    {
        QJsonArray row;
        for (int j=0; j<0; j<getnumcols())
            row.append(valueAt(i,j));
        matrix.append(row);
    }
    out["Matrix"] = matrix;
    return out;
}
bool CMBMatrix::ReadFromJsonObject(const QJsonObject &jsonobject)
{
    return true;
}

bool CMBMatrix::writetofile(QFile* file)
{
    file->write(QString::fromStdString(ToString()).toUtf8());
    return true;
}
string CMBMatrix::ToString()
{
    string out;
    for (int j=0; j<getnumcols(); j++)
    {
        out += "," + columnlabels[j];
    }
    out += "\n";
    for (int i=0; i<getnumrows(); i++)
    {
        QJsonArray row;
        out+= rowlabels[i];
        for (int j=0; j<getnumcols(); j++)
        {
            out+=",";
            out+=QString::number(valueAt(i,j)).toStdString();
        }
        out += "\n";
    }
    return out;
}

double CMBMatrix::valueAt(int i, int j)
{
    return CMatrix::operator[](i).at(j);
}
