#ifndef CMBMATRIX_H
#define CMBMATRIX_H

#include "Matrix.h"
#include "interface.h"

class CMBMatrix : public CMatrix, Interface
{
public:
    CMBMatrix();
    CMBMatrix(int n);
    CMBMatrix(int n, int m);
    CMBMatrix(const CMBMatrix& mp);
    CMBMatrix& operator=(const CMBMatrix &mp);
    QJsonObject toJsonObject() override;
    bool ReadFromJsonObject(const QJsonObject &jsonobject) override;
};

#endif // CMBMATRIX_H
