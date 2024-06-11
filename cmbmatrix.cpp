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
    lowlimit = mp.lowlimit;
    highlimit = mp.highlimit;
    highlightoutsideoflimit = mp.highlightoutsideoflimit;
}

CMBMatrix::CMBMatrix(int n):CMatrix(n), Interface()
{
    columnlabels.resize(n);
    rowlabels.resize(n);
}
CMBMatrix::CMBMatrix(int n, int m):CMatrix(m,n), Interface()
{
    rowlabels.resize(m);
    columnlabels.resize(n);
}

CMBMatrix& CMBMatrix::operator=(const CMBMatrix &mp)
{
    columnlabels = mp.columnlabels;
    rowlabels = mp.rowlabels;
    lowlimit = mp.lowlimit;
    highlimit = mp.highlimit;
    highlightoutsideoflimit = mp.highlightoutsideoflimit;
    CMatrix::operator=(mp);
    return *this;
}
QJsonObject CMBMatrix::toJsonObject()
{
    QJsonObject out;
    QJsonArray matrix;
    for (int i=0; i<getnumrows(); i++)
    {
        QJsonArray row;
        for (int j=0; j<getnumcols(); j++)
            row.append(valueAt(i,j));
        matrix.append(row);
    }
    QJsonArray Row_Labels;
    for (unsigned int i=0; i<rowlabels.size(); i++)
    {
        Row_Labels.append(QString::fromStdString(rowlabels[i]));
    }
    QJsonArray Column_Labels;
    for (unsigned int i=0; i<columnlabels.size(); i++)
    {
        Column_Labels.append(QString::fromStdString(columnlabels[i]));
    }
    out["Matrix"] = matrix;
    out["RowLabels"] = Row_Labels;
    out["ColumnLabels"] = Column_Labels;
    return out;
}
bool CMBMatrix::ReadFromJsonObject(const QJsonObject &jsonobject)
{
    QJsonArray array = jsonobject["Matrix"].toArray();
    QJsonArray Row_Labels = jsonobject["RowLabels"].toArray();
    QJsonArray Column_Labels = jsonobject["ColumnLabels"].toArray();

    if (array.size()==0)
        return false;
    Resize(array.size(), array[0].toArray().size());
    rowlabels.resize(Row_Labels.size());
    columnlabels.resize(Column_Labels.size());

    for (unsigned int i=0; i<array.size(); i++)
    {
        for (unsigned int j=0; j<array[i].toArray().size(); j++)
        {
            matr[i][j] = array[i].toArray()[j].toDouble();
        }
    }

    for (unsigned int i=0; i<Row_Labels.size(); i++)
        rowlabels[i] = Row_Labels[i].toString().toStdString();

    for (unsigned int i=0; i<Column_Labels.size(); i++)
        columnlabels[i] = Column_Labels[i].toString().toStdString();

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
QTableWidget *CMBMatrix::ToTable()
{
    QTableWidget *tablewidget = new QTableWidget();
    tablewidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablewidget->setColumnCount(getnumcols());
    tablewidget->setRowCount(getnumrows());
    QStringList rowheaders;
    QStringList colheaders;

    for (int j=0; j<getnumcols(); j++)
        colheaders << QString::fromStdString(columnlabels[j]);

    for (int i=0; i<getnumrows(); i++)
    {
        rowheaders << QString::fromStdString(rowlabels[i]);
        for (int j=0; j<getnumcols(); j++)
        {   tablewidget->setItem(i,j, new QTableWidgetItem(QString::number(matr[i][j])));
            if (!boolean_values)
            {   if (highlightoutsideoflimit)
                {
                    if (matr[i][j]>highlimit || matr[i][j]<lowlimit)
                    {
                        tablewidget->item(i,j)->setForeground(QColor(Qt::red));
                    }
                }
            }
            else
            {
                if (matr[i][j]==1)
                    tablewidget->setItem(i,j, new QTableWidgetItem("Fail"));
                else
                    tablewidget->setItem(i,j, new QTableWidgetItem("Pass"));
            }
        }
    }

    tablewidget->setHorizontalHeaderLabels(colheaders);
    tablewidget->setVerticalHeaderLabels(rowheaders);
    return tablewidget;
}

double CMBMatrix::valueAt(int i, int j)
{
    return CMatrix::operator[](i).at(j);
}

CMBVector operator*(const CMBMatrix &M, const CMBVector&V)
{
    CMBVector out = M.toMatrix()*V.toVector();
    out.SetLabels(M.RowLabels());
    return out;

}

CMatrix CMBMatrix::toMatrix() const
{
    CMatrix out(getnumrows(), getnumcols());
    out.matr = matr;
    return out;
}

CMBVector CMBMatrix::GetRow(const string &rowlabel)
{
    CMBVector out(getnumcols());
    for (unsigned int i=0; i<rowlabels.size(); i++)
    {
        if (rowlabels[i]==rowlabel)
        {
            for (unsigned int j=0; j<getnumcols(); j++)
                out[j] = matr[i][j];
        }
    }
    out.SetLabels(ColumnLabels());
    return out;
}
CMBVector CMBMatrix::GetColumn(const string &columnlabel)
{
    CMBVector out(getnumrows());
    for (unsigned int i=0; i<columnlabels.size(); i++)
    {
        if (columnlabels[i]==columnlabel)
        {
            for (unsigned int j=0; j<getnumrows(); j++)
                out[j] = matr[j][i];
        }
    }
    out.SetLabels(RowLabels());
    return out;
}

QStringList CMBMatrix::RowLabelCategories()
{
    QStringList out;
    for (unsigned int i=0; i<rowlabels.size(); i++)
    {
        if (!out.contains(QString::fromStdString(rowlabels[i])))
            out<<QString::fromStdString(rowlabels[i]);
    }
    return out;
}
