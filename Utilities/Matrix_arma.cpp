// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

// Matrix.cpp: implementation of the CMatrix_arma class.
//
//////////////////////////////////////////////////////////////////////

#include "Matrix_arma.h"
#include "Vector_arma.h"
#include "Matrix.h"
#include "math.h"
#include <iostream>
#define ARMA_DONT_PRINT_ERRORS
#include "armadillo"
//#include "StringOP.h"
//#include "qstring.h"
//#include "qmap.h"
//#include "qvariant.h"
#include <vector>
#include "Vector.h"
// SuperLU is used only if the installed Armadillo was configured with it
// (this used to force it on, which failed to link where SuperLU is absent)

using namespace arma;


//using namespace std;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMatrix_arma::CMatrix_arma(int m, int n)
{
    numrows = m;
    numcols = n;
    arma::mat::resize(m, n);
    fill(fill::zeros);

}

CMatrix_arma::CMatrix_arma()
{
    numrows = 0;
    numcols = 0;
}

CMatrix_arma::CMatrix_arma(int m):arma::mat(m,m)
{
    numrows = m;
    numcols = m;
    fill(fill::zeros);
}

CMatrix_arma::CMatrix_arma(const CMatrix_arma &m):arma::mat(m)
{
    numrows = m.numrows;
    numcols = m.numcols;

}

CMatrix_arma CMatrix_arma::Diag(int n)
{
    CMatrix_arma M(n);

    for (int i = 0;i<M.getnumrows(); ++i)
    {
        M(i,i) = 1;
    }
    return M;
}

CMatrix_arma::CMatrix_arma(const CMatrix& m)
{
    numrows = m.getnumrows();
    numcols = m.getnumcols();
    mat::resize(numrows, numcols);
    for (int i = 0; i < numrows; ++i)
        for (int j = 0; j < numcols; ++j)
            at(i,j) = m.matr[i][j];

}

CMatrix_arma::CMatrix_arma(const mat& m):arma::mat(m)
{
    setnumcolrows();
}



CMatrix_arma::CMatrix_arma(const CVector_arma &v)
{
    numrows = v.num();
    numcols = 1;
    // This was "arma::mat(numrows,1);", which built and discarded a
    // temporary, leaving this matrix empty for the unchecked writes below
    set_size(numrows, 1);

    for (int i=0; i<numrows; ++i) at(i,0) = v.vect(i);
}


CMatrix_arma::~CMatrix_arma()
{
    clear();
}

double& CMatrix_arma::get(int i, int j)
{
    return at(i, j);
}

double& CMatrix_arma::operator()(int i, int j)
{
    return get(i, j);
}

std::vector<double*> CMatrix_arma::get(int i)
{
    std::vector<double*> v;
    for (int j = 0; j < numrows; j++)
        v[j] = &at(i, j);

    return v;
}

int CMatrix_arma::getnumrows() const {return numrows;};
int CMatrix_arma::getnumcols() const {return numcols;};

CMatrix_arma& CMatrix_arma::operator=(const CMatrix_arma &m)
{

    numcols = m.numcols;
    numrows = m.numrows;
    mat::operator=(m);

    return *this;
}

CMatrix_arma& CMatrix_arma::operator+=(const CMatrix_arma &m)
{
    mat::operator+=(m);
    setnumcolrows();
    return *this;
}

CMatrix_arma& CMatrix_arma::operator-=(const CMatrix_arma &m)
{
    mat::operator-=(m);
    setnumcolrows();
    return *this;
}

void CMatrix_arma::Print(FILE *FIL)
{
    for (int i=0; i<numrows; i++)
    {	for (int j=0; j<numcols; j++)
            fprintf(FIL, "%le ", at(i,j));
        fprintf(FIL, "\n");
    }
    fclose(FIL);

}

CMatrix_arma operator+(const CMatrix_arma &m1, const CMatrix_arma &m2)
{
    CMatrix_arma mt = m1;
    mt += m2;
    return mt;
}

CMatrix_arma operator-(const CMatrix_arma &m1, const CMatrix_arma &m2)
{
    CMatrix_arma mt = m1;
    mt -= m2;
    return mt;
}

CMatrix_arma mult(const CMatrix_arma &m1, const CMatrix_arma &m2)
{
    arma::mat m = arma::mat(m1)*arma::mat(m2);
    CMatrix_arma M = m;
    M.setnumcolrows();
    return M;
}

CMatrix_arma operator*(const CMatrix_arma& m1, const CMatrix_arma& m2)
{
    arma::mat m = arma::mat(m1)*arma::mat(m2);
    CMatrix_arma M = m;
    M.setnumcolrows();
    return M;
}

CVector_arma mult(const CMatrix_arma &m1, const CVector_arma &v1)
{
    arma::vec V = m1*v1;
    CVector_arma out = V;
    return out;
}

CMatrix_arma operator*(double d, const CMatrix_arma& m1)
{
    arma::mat TrM = d*arma::mat(m1);
    CMatrix_arma out = TrM;
    out.setnumcolrows();
    return out;
}

CMatrix_arma operator+(double d, CMatrix_arma m1)
{
    CMatrix_arma TrM(m1.getnumrows(), m1.getnumcols());
    for (int i=0; i<m1.getnumrows(); i++)
        for (int j=0; j<m1.getnumcols(); j++)
            TrM.get(i,j) = d+m1.get(i,j);
    return TrM;

}

CMatrix_arma operator-(double d, CMatrix_arma m1)
{
    CMatrix_arma TrM(m1.getnumrows(), m1.getnumcols());
    for (int i=0; i<m1.getnumrows(); i++)
        for (int j=0; j<m1.getnumcols(); j++)
            TrM.get(i,j) = d-m1.get(i,j);
    return TrM;

}

CMatrix_arma operator+(CMatrix_arma m1, double d)
{
    CMatrix_arma TrM(m1.getnumrows(), m1.getnumcols());
    for (int i=0; i<m1.getnumrows(); i++)
        for (int j=0; j<m1.getnumcols(); j++)
            TrM.get(i,j) = d+m1.get(i,j);
    return TrM;

}

CMatrix_arma operator-(CMatrix_arma m1,double d)
{
    CMatrix_arma TrM(m1.getnumrows(), m1.getnumcols());
    for (int i=0; i<m1.getnumrows(); i++)
        for (int j=0; j<m1.getnumcols(); j++)
            TrM.get(i,j) = m1.get(i,j)-d;
    return TrM;

}

CMatrix_arma operator/(CMatrix_arma m1,double d)
{
    CMatrix_arma TrM(m1.getnumrows(), m1.getnumcols());
    for (int i=0; i<m1.getnumrows(); i++)
        for (int j=0; j<m1.getnumcols(); j++)
            TrM.get(i,j) = m1.get(i,j)/d;
    return TrM;
}

CMatrix_arma operator/(double d, CMatrix_arma m1)
{
    CMatrix_arma TrM(m1.getnumrows(), m1.getnumcols());
    for (int i=0; i<m1.getnumrows(); i++)
        for (int j=0; j<m1.getnumcols(); j++)
            TrM.get(i,j) = d/m1.get(i,j);
    return TrM;
}


//CVector_arma operator*(const CMatrix_arma &m, const CVector_arma &v)
//{
//    CVector_arma out = m.matr*arma::vec(v);
//}

//CVector_arma_arma gauss(CMatrix_arma, CVector_arma_arma)
//{
//}


CVector_arma operator/(CVector_arma &V, CMatrix_arma &M)
{
    CVector_arma X(M.getnumcols());
    bool status = solve(X, M, V);
    if (status == false) X = arma::vec();
    return X;
}

CVector_arma operator/(const CVector_arma &V, const CMatrix_arma &M)
{
    CVector_arma X(M.getnumcols());
    bool status = solve( X, M, V);
    if (status == false) X = arma::vec();
    return X;
}

CMatrix_arma Log(CMatrix_arma &M1)
{
    CMatrix_arma TrM(M1.getnumrows(), M1.getnumcols());
    for (int i=0; i<M1.getnumrows(); i++)
        for (int j=0; j<M1.getnumcols(); j++)
            TrM.get(i,j) = log(M1.get(i,j));
    return TrM;
}

CMatrix_arma Exp(CMatrix_arma &M1)
{
    CMatrix_arma TrM(M1.getnumrows(), M1.getnumcols());
    for (int i=0; i<M1.getnumrows(); i++)
        for (int j=0; j<M1.getnumcols(); j++)
            TrM.get(i,j) = exp(M1.get(i,j));
    return TrM;
}

CMatrix_arma Sqrt(CMatrix_arma &M1)
{
    CMatrix_arma TrM(M1.getnumrows(), M1.getnumcols());
    for (int i=0; i<M1.getnumrows(); i++)
        for (int j=0; j<M1.getnumcols(); j++)
            TrM.get(i,j) = sqrt(M1.get(i,j));
    return TrM;
}




CMatrix_arma Invert(const CMatrix_arma &M1)
{
    CMatrix_arma InvM(M1.getnumcols(), M1.getnumcols());
    InvM = inv(M1);
    return InvM;
}


/*double det(CMatrix_arma &A)
{
    CMatrix_arma D = LU_decomposition(A);
    double prod = 1;
    for (int i=0; i<A.getnumcols(); i++)
        prod *= A(i,i);

    return prod;

}
*/

double CMatrix_arma::det()
{
    return arma::det(*this);
}


CVector_arma diag(const CMatrix_arma &m)
{
    CVector_arma v(m.getnumcols());
    for (int i=0; i<m.getnumcols(); ++i)
        v[i] = m.at(i,i);
    return v;
}

CVector_arma maxelements(const CMatrix_arma &m)
{
    CVector_arma v(m.getnumcols());
    for (int i=0; i<m.getnumcols(); ++i)
    {   double maxval = -1e36;
        for (int j=0; j<m.getnumrows(); ++j)
            maxval = std::max(fabs(m.at(i,j)),maxval);
        v[i] = maxval;
    }
    return v;

}

CVector CMatrix_arma::maxelements() const
{
    CVector v(getnumcols());
    for (int i=0; i<getnumcols(); ++i)
    {   double maxval = -1e36;
        for (int j=0; j<getnumrows(); ++j)
            maxval = std::max(fabs(at(i,j)),maxval);
        v[i] = maxval;
    }
    return v;
}

CMatrix_arma operator*(const CVector_arma &v, const CMatrix_arma &m)
{
    arma::mat a = arma::mat(m)*arma::vec(v);
    CMatrix_arma out = a;
    out.setnumcolrows();
    return out;
}

CVector_arma operator*(const CMatrix_arma &m, const CVector_arma &v)
{
    arma::vec a = arma::mat(m)*arma::vec(v);
    CVector_arma out = a;
    return out;
}


CMatrix_arma oneoneprod(CMatrix_arma &m1, CMatrix_arma &m2)
{
    CMatrix_arma TrM(m1.getnumrows(), m1.getnumcols());
    for (int i=0; i<m1.getnumrows(); i++)
        for (int j=0; j<m1.getnumcols(); j++)
            TrM.get(i,j) = m1.get(i,j)*m2.get(i,j);
    return TrM;
}

void CMatrix_arma::setval(double a)
{
    arma::mat::operator=(a);
}

void CMatrix_arma::setvaldiag(double a)
{
    for (int i=0; i<getnumrows(); i++)
        at(i,i) = a;

}

void CMatrix_arma::writetofile(FILE *f)
{
    for (int i=0; i<numrows; i++)
    {	for (int j=0; j<numcols; j++)
            fprintf(f, "%le, ", at(i,j));
        fprintf(f, "\n");
    }
}

void CMatrix_arma::writetofile(const std::string& filename)
{
    FILE *f = fopen(filename.c_str(),"w");
    if (f == nullptr)
        return;  // the file could not be opened; fprintf(nullptr) would crash
    for (int i=0; i<numrows; i++)
    {	for (int j=0; j<numcols; j++)
            fprintf(f, "%le, ", at(i,j));
        fprintf(f, "\n");
    }
    fclose(f);
}


void CMatrix_arma::writetofile_app(const std::string& filename)
{
    FILE *f = fopen(filename.c_str(),"a");
    if (f == nullptr)
        return;  // the file could not be opened; fprintf(nullptr) would crash
    for (int i=0; i<numrows; i++)
    {	for (int j=0; j<numcols; j++)
            fprintf(f, "%le, ", at(i,j));
        fprintf(f, "\n");
    }
    fclose(f);
}

CMatrix_arma Transpose(CMatrix_arma &M1)
{
    arma::mat TrM = M1.t();
    CMatrix_arma out = TrM;
    out.setnumcolrows();
    return out;
}

void CMatrix_arma::print(const std::string& s)
{

    std::ofstream Afile;
    Afile.open(s+".txt");

    for (int i = 0; i<numrows; ++i)
    {
        for (int j = 0; j<numcols; ++j)
        {
            Afile << at(i,j) << ", ";
        }
        Afile << "\n";
    }
}

CVector_arma solve_ar(CMatrix_arma &M, CVector_arma &V)
{

    CVector_arma ansr;
    solve(ansr, M,V);
    if (ansr.n_rows == 0) ansr = arma::vec();
    return ansr;
}

CMatrix_arma inv(const CMatrix_arma &M)
{

    CMatrix_arma A;
    bool X = inv(A, M);
    if (X) A.setnumcolrows();
    return A;
}

CMatrix_arma& CMatrix_arma::operator=(const mat &A)
{
    mat::operator=(A);
    setnumcolrows();

    return *this;
}

void write_to_file(std::vector<CMatrix_arma> M, const std::string& filename)
{
    std::ofstream Afile;
    Afile.open(filename);
    M.push_back(Average(M));
    for (unsigned int k = 0; k<M.size(); k++)
    {	for (int i = 0; i<M[k].numrows; ++i)
        {
            for (int j = 0; j<M[k].numcols; ++j)
            {
                Afile << M[k].get(i,j) << ", ";
                cout<< M[k].get(i,j) << ", ";
            }
            Afile << "\n";
        }
    Afile << "\n";
    }

}

CMatrix_arma Average(std::vector<CMatrix_arma> M)
{
    CMatrix_arma AVG(M[0].numrows, M[0].numcols);
    int n = M.size();
    for (unsigned int k = 0; k<M.size(); k++)
        for (unsigned int i = 0; i<M[k].numrows; ++i)
            for (int j = 0; j<M[k].numcols; ++j)
                AVG.get(i,j) += M[k].get(i,j)/n;
    return AVG;
}

CVector_arma CMatrix_arma::diag_ratio()
{
    CVector_arma X(numcols);
    CVector_arma maxs(numcols);
    for (int i=0; i<numcols; i++)
    {	for (int j=0; j<numrows; j++)
            if (i!=j) maxs[i] += fabs(at(i,j));
        X[i]=maxs[i]/at(i,i);
    }
    return X;
}

std::vector<std::vector<bool>> CMatrix_arma::non_posdef_elems(double tol)
{
    std::vector<std::vector<bool>> M;
    M.resize(getnumcols());

    for (int i = 0; i < getnumcols(); i++)
    {
        M[i].resize(getnumcols());
        for (int j = 0; j < getnumrows(); j++)
            if (at(i,j) / at(i,i) > tol) M[i][j] = 1;
    }
    return M;


}

CMatrix_arma CMatrix_arma::non_posdef_elems_m(double tol)
{
    CMatrix_arma M(getnumcols(), getnumrows());

    for (int i = 0; i < getnumcols(); i++)
        for (int j = 0; j < getnumrows(); j++)
            if (at(i,j) / at(i,i) > tol) M.get(i,j) = at(i,j);

    return M;


}

CMatrix_arma Identity_ar(int rows)
{
    CMatrix_arma M(rows, rows);
    for (int i = 0; i < rows; i++)
        M.get(i,i) = 1;

    return M;
}

CMatrix_arma CMatrix_arma::Preconditioner(double tol)
{
    CMatrix_arma M = non_posdef_elems_m(tol)+Identity_ar(numcols);
    for (int i = 0; i < getnumcols(); i++)
        for (int j = 0; j < getnumrows(); j++)
            if ((M.get(i,j) != 0) & (i != j))
                M.get(i,j) = -M.get(i,j);

    return M;
}
std::vector<std::string> CMatrix_arma::toString(const std::string& /*format*/, std::vector<std::string> columnHeaders, std::vector<std::string> rowHeaders) const
{
    std::vector<std::string> r;
    bool rowH = false, colH = false;
    int rowOffset = 0;
    if (columnHeaders.size() && columnHeaders.size() == numcols)
    {
        colH = true;
        rowOffset = 1;
    }
    if (rowHeaders.size() && rowHeaders.size() == numrows)
    {
        rowH = true;
    }
    r.resize(numrows + rowOffset);


    if (colH)
    {
        if (rowH) r[0] += ", ";
        for (int j = 0; j<numcols; j++)
        {
            r[0] += columnHeaders[j];
            if (j < numcols - 1) r[0] += ", ";
        }

    }
    for (int i = 0; i<numrows; i++)
    {
        if (rowH)
        {
            r[i + rowOffset] += rowHeaders[i];
            r[i + rowOffset] += ", ";
        }
        for (int j = 0; j<numcols; j++)
        {
            std::ostringstream streamObj;
            streamObj << at(i,j);
            std::string strObj = streamObj.str();
            r[i + rowOffset] += strObj;
            if (j < numcols - 1) r[i + rowOffset] += ", ";
        }
    }
    return r;
}


void CMatrix_arma::setnumcolrows()
{
    numcols = n_cols;
    numrows = n_rows;
}

void CMatrix_arma::setrow(int i, CVector_arma V)
{
    for (int j = 0; j < getnumcols(); j++)
        at(i, j) = V[j];

}
void CMatrix_arma::setrow(int i, CVector V)
{
    for (int j = 0; j < getnumcols(); j++)
        at(i, j) = V[j];
}
void CMatrix_arma::setcol(int i,  CVector_arma V)
{
    for (int j = 0; j < getnumrows(); j++)
        at(j, i) = V[j];
}
void CMatrix_arma::setcol(int i,  CVector V)
{
    for (int j = 0; j < getnumrows(); j++)
        at(j, i) = V[j];
}

CVector_arma CMatrix_arma::getcol(int i)
{
    CVector_arma out(getnumrows());
    for (int j = 0; j < getnumrows(); j++)
        out[j] = at(j, i);

    return out;
}

CVector_arma CMatrix_arma::getrow(int i)
{
    CVector_arma out(getnumcols());
    for (int j = 0; j < getnumcols(); j++)
        out[j] = at(i, j);

    return out;
}


CMatrix_arma normalize_diag( const CMatrix_arma &M1, const CMatrix_arma &M2)
{
    CMatrix_arma M(M1);
    CVector_arma D = diag(M2);
    for (int i = 0; i<M1.getnumcols(); i++)
    {
        for (int j=0; j<M1.getnumrows(); j++)
            M.at(i,j) = M1.at(i,j) / D[i];
    }
    return M;

}

CVector_arma normalize_diag( const CVector_arma &V, const CMatrix_arma &M2)
{
    CVector_arma M(V);

    for (int i = 0; i<V.getsize(); i++)
    {
        M[i] = V[i] / M2.at(i,i);
    }
    return M;
}

CVector_arma normalize_diag( const CVector_arma &V, const CVector_arma &D)
{
    CVector_arma M(V);

    for (int i = 0; i<V.getsize(); i++)
    {
        M[i] = V[i] / D[i];
    }
    return M;
}

CMatrix_arma normalize_max( const CMatrix_arma &M1, const CMatrix_arma &M2)
{
    CMatrix_arma M(M1);
    CVector_arma D = maxelements(M2);
    for (int i = 0; i<M1.getnumcols(); i++)
    {
        for (int j=0; j<M1.getnumrows(); j++)
            M.at(i,j) = M1.at(i,j) / D[i];
    }
    return M;

}

CVector_arma normalize_max( const CVector_arma &V, const CMatrix_arma &M2)
{
    CVector_arma M(V);
    CVector_arma D = maxelements(M2);
    for (int i = 0; i<V.getsize(); i++)
    {
        M[i] = V[i] / D[i];
    }
    return M;
}

CVector_arma normalize_max( const CVector_arma &V, const CVector_arma &D)
{
    CVector_arma M(V);

    for (int i = 0; i<V.getsize(); i++)
    {
        M[i] = V[i] / D[i];
    }
    return M;
}

void CMatrix_arma::ScaleDiagonal(double x)
{
    for (int i = 0; i < getnumcols(); i++)
    {
        at(i, i) *= x;
    }
}

CMatrix_arma CMatrix_arma::Identity(int rows)
{
    CMatrix M(rows, rows);
    for (int i = 0; i < rows; i++)
        M[i][i] = 1;

    return M;
}

CMatrix_arma GetReal(const arma::cx_mat &vx)
{
    CMatrix_arma out(vx.n_rows,vx.n_rows );
    for (int i=0; i<vx.n_rows; i++)
        for (int j=0; j<vx.n_cols; j++)
        out(i,j) = vx(i,j).real();

    return out;
}
CMatrix_arma GetImg(const arma::cx_mat &vx)
{
    CMatrix_arma out(vx.n_rows,vx.n_rows );
    for (int i=0; i<vx.n_rows; i++)
        for (int j=0; j<vx.n_cols; j++)
        out(i,j) = vx(i,j).imag();

    return out;
}
