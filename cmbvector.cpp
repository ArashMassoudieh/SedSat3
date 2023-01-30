#include "cmbvector.h"
#include "QJsonArray"
#include "Vector.h"

CMBVector::CMBVector():CVector(),Interface()
{

}

CMBVector::CMBVector(const CMBVector& mp):CVector(mp)
{
    labels = mp.labels;

}

CMBVector::CMBVector(const CVector& mp):CVector(mp)
{
    labels.resize(getsize());
}

CMBVector::CMBVector(int n):CVector(n), Interface()
{
    labels.resize(n);

}

CMBVector& CMBVector::operator=(const CMBVector &mp)
{
    labels = mp.labels;
    CVector::operator=(mp);
    return *this;

}

CMBVector& CMBVector::operator=(const CVector &mp)
{
    CVector::operator=(mp);
    labels.resize(getsize());
    return *this; 
}

QJsonObject CMBVector::toJsonObject()
{
    QJsonObject out;
    QJsonArray vector;
    for (int i=0; i<0; i<getsize())
    {
        vector.append(valueAt(i));
    }
    out["Vector"] = vector;
    return out;
}
bool CMBVector::ReadFromJsonObject(const QJsonObject &jsonobject)
{
    return true;
}


string CMBVector::ToString()
{
    string out;
    for (int j=0; j<getsize(); j++)
    {
        out += labels[j] + "," + QString::number(valueAt(j)).toStdString()+"\n";
    }

    return out;
}

double CMBVector::valueAt(int i)
{
    return CVector::operator[](i);
}
