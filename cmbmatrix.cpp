#include "cmbmatrix.h"

CMBMatrix::CMBMatrix():CMatrix(),Interface()
{

}

CMBMatrix::CMBMatrix(const CMBMatrix& mp):CMatrix(mp)
{

}

CMBMatrix::CMBMatrix(int n):CMatrix(n), Interface()
{

}
CMBMatrix::CMBMatrix(int n, int m):CMatrix(m,n), Interface()
{

}

CMBMatrix& CMBMatrix::operator=(const CMBMatrix &mp)
{
    CMatrix::operator=(mp);
    return *this; 
}
QJsonObject CMBMatrix::toJsonObject()
{
    return QJsonObject(); 
}
bool CMBMatrix::ReadFromJsonObject(const QJsonObject &jsonobject)
{
    return true;
}
