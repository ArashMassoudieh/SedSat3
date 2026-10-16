// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

// Matrix.h: interface for the CMatrix_arma class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Vector_arma.h"
#include <iostream>
#include <math.h>
#define ARMA_DONT_PRINT_ERRORS
#include "armadillo"
class QVariant;
//class QString;
//class QList;
//#include "QMap"
using namespace arma;
class CMatrix; 
class CVector_arma;
class CMatrix_arma: public arma::mat
{

private:
	int numrows{};
	int numcols{};

public:
    //mat matr;
	CMatrix_arma(int, int);
	CMatrix_arma(int);
	CMatrix_arma();
	CMatrix_arma(const CMatrix_arma&);
	CMatrix_arma(const CVector_arma&);
    CMatrix_arma(const CMatrix&);
    CMatrix_arma(const mat& m);
    static CMatrix_arma Diag(int n);
    //CVector_arma operator[](int);
    double& get(int i, int j);
	double& operator()(int i, int j);
    std::vector<double*> get(int i);
    int getnumrows() const;
    int getnumcols() const;
	virtual ~CMatrix_arma();
    CMatrix_arma& operator=(const CMatrix_arma&);
    CMatrix_arma& operator+=(const CMatrix_arma&);
    CMatrix_arma& operator-=(const CMatrix_arma &);
    CMatrix_arma& operator=(const mat&);
	friend void triangulate(CMatrix_arma&, CVector_arma&);
	friend void backsubst(CMatrix_arma&, CVector_arma&, CVector_arma&);
	friend CVector_arma gauss0(CMatrix_arma, CVector_arma);
    friend CVector_arma diag(const CMatrix_arma&);
    CVector maxelements() const;

    CMatrix_arma LU_decomposition();
    CMatrix_arma Cholesky_factor();
    double det();
    void Print(FILE *FIL);
    void print(std::string s);
    void setval(double a);
    void setvaldiag(double a);
    void writetofile(FILE *f);
    void writetofile(std::string filename);
    void writetofile_app(std::string filename);
	friend void write_to_file(std::vector<CMatrix_arma> M, std::string filename);
	friend CMatrix_arma Average(std::vector<CMatrix_arma> M);
    CVector_arma diag_ratio();
    std::vector<std::vector<bool> > non_posdef_elems(double tol = 1);
    CMatrix_arma non_posdef_elems_m(double tol = 1);
    CMatrix_arma Preconditioner(double tol = 1);
    std::vector<std::string> toString(std::string format = "", std::vector<std::string> columnHeaders = std::vector<std::string>(), std::vector<std::string> rowHeaders = std::vector<std::string>()) const;
	std::vector<std::string> toHtml(std::string format = "", std::vector<std::string> columnHeaders = std::vector<std::string>(), std::vector<std::string> rowHeaders = std::vector<std::string>());
    void setnumcolrows();
    void setrow(int i, CVector_arma V);
    void setrow(int i, CVector V);
    void setcol(int i, CVector_arma V);
    void setcol(int i, CVector V);
    CVector_arma getcol(int i);
    CVector_arma getrow(int i);

	void ScaleDiagonal(double x);
    static CMatrix_arma Identity(int rows);

};

CMatrix_arma GetReal(const arma::cx_mat &vx);
CMatrix_arma GetImg(const arma::cx_mat &vx);
double det(CMatrix_arma &);
CMatrix_arma Log(CMatrix_arma &M1);
CMatrix_arma Exp(CMatrix_arma &M1);
CMatrix_arma Sqrt(CMatrix_arma &M1);
CMatrix_arma operator+(const CMatrix_arma&, const CMatrix_arma&);
CMatrix_arma operator+(double, const CMatrix_arma&);
CMatrix_arma operator+(const CMatrix_arma&, double);
CMatrix_arma operator-(double d, const CMatrix_arma &m1);
CMatrix_arma operator+(const CMatrix_arma &m1, double d);
CMatrix_arma operator-(const CMatrix_arma &m1,double d);
CMatrix_arma operator/(const CMatrix_arma &m1,double d);
CMatrix_arma operator/(double d, const CMatrix_arma &m1);
CMatrix_arma operator-(const CMatrix_arma&, const CMatrix_arma&);
CMatrix_arma operator*(const CMatrix_arma&, const CMatrix_arma&);
CVector_arma operator*(const CMatrix_arma&, const CVector_arma&);
CMatrix_arma operator*(const CVector_arma&, const CMatrix_arma&);
CMatrix_arma operator*(double, CMatrix_arma);
CVector_arma operator/(CVector_arma&, CMatrix_arma&);
CVector_arma operator/(const CVector_arma &V, const CMatrix_arma &M);
CMatrix_arma Transpose(CMatrix_arma &M1);
CMatrix_arma Invert(const CMatrix_arma &M1);
CVector_arma SpareSolve(CMatrix_arma, CVector_arma);
CMatrix_arma oneoneprod(CMatrix_arma &m1, CMatrix_arma &m2);
CVector_arma solve_ar(CMatrix_arma&, CVector_arma&);
CMatrix_arma inv(const CMatrix_arma&);
CMatrix_arma normalize_diag( const CMatrix_arma&,  const CMatrix_arma&);
CVector_arma normalize_diag( const CVector_arma&,  const CMatrix_arma&);
CVector_arma normalize_diag( const CVector_arma&,  const CVector_arma&);
CMatrix_arma normalize_max( const CMatrix_arma&,  const CMatrix_arma&);
CVector_arma normalize_max( const CVector_arma&,  const CMatrix_arma&);
CVector_arma normalize_max( const CVector_arma&,  const CVector_arma&);
CMatrix_arma mult(const CMatrix_arma&, const CMatrix_arma&);
CVector_arma mult(const CMatrix_arma&, const CVector_arma&);
CVector_arma mult(const CVector_arma&, const CMatrix_arma&);
CMatrix_arma Identity_ar(int rows);
CVector_arma maxelements(const CMatrix_arma&);
CMatrix_arma Cholesky_factor(CMatrix_arma &M);
CMatrix_arma LU_decomposition(CMatrix_arma &M);

