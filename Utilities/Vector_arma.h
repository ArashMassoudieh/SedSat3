// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.


#pragma once

#include <iostream>
#include <vector>
#include "armadillo"
using namespace arma;

//using namespace std;

class CMatrix_arma;
class CVector;
class SizeDist;
class CVector_arma: public arma::vec
{
private:


public:
	CVector_arma();
	CVector_arma(int);
	CVector_arma(const std::vector<double>&, int);
	CVector_arma(const std::vector<double> &v);
    CVector_arma(const vec &v);
	CVector_arma(CVector &v);
	CVector_arma(const std::vector<int> &v);
	CVector_arma(const double x, int n);
    CVector_arma(const double x_min, const double x_max, int n);
	CVector_arma(const CVector_arma&);
    arma::vec &vect();
    double &vect(int i);
    double const &vect(int i) const;
	double& operator[](int);
    double operator[](int) const;
	virtual ~CVector_arma();
    int num() const {return  size();} ;
    int range(int);
	CVector_arma& operator=(const CVector_arma&);
	CVector_arma& operator=(const CVector&);
	CVector_arma& operator=(const std::vector<double>&);
	CVector_arma& operator=(const double &v);
	CVector_arma operator=(mat);
	CVector_arma& operator+();
	void swap(int , int );
    int getsize() const;
    bool haszeros() const;
	CVector_arma& operator*=(double);
	CVector_arma& operator/=(double);
	CVector_arma& operator+=(const CVector_arma&);
	CVector_arma& operator-=(const CVector_arma&);
	CVector_arma& operator*=(const CVector_arma&);
    friend double dotproduct(const CVector_arma&, const CVector_arma&);
	friend CVector_arma mult(CMatrix_arma&, CVector_arma&);
    friend double norm(CVector_arma);
	bool operator==(double v);
	bool operator==(CVector_arma &v);
	double max();
	double min();
	double norm2();
	double sum();
	double abs_max();
	int abs_max_elems();
	CMatrix_arma T();
	CVector_arma Log();
	CVector_arma abs();
	CVector_arma H();
	void writetofile(FILE *f);
	void writetofile(const std::string& filename);
	void writetofile(std::ofstream &f);
	void writetofile_app(const std::string& filename);
	CVector_arma Exp();
	std::vector<int> Int();
	CMatrix_arma diagmat();
	CVector_arma append(const CVector_arma& V1);
	CVector_arma append(double d);
	CVector_arma sort();
	std::vector<int> lookup(double val);
	void print(const std::string& s);
	CVector_arma sub(int i, int j);
	bool is_finite();
	std::vector<int> get_nan_elements();
    std::string toString() const;
    std::vector<int> negative_elements();

};

CVector_arma GetReal(const arma::cx_vec &vx);
CVector_arma GetImg(const arma::cx_vec &vx);
CVector_arma Log(CVector_arma &);
CVector_arma Exp(CVector_arma &);
CVector_arma abs(CVector_arma &);  //works w/o reference. if const included means read only
double abs_max(CVector_arma &);
double min(CVector_arma &);
double max(CVector_arma &);
CVector_arma H(CVector_arma &);
CVector_arma operator+(const CVector_arma&, const CVector_arma&);
CVector_arma operator+(const double&, const CVector_arma&);
CVector_arma operator+(const CVector_arma&, const double&);
CVector_arma operator-(const CVector_arma&, const CVector_arma&);
CVector_arma operator-(const double&, const CVector_arma&);
CVector_arma operator*(const CVector_arma&, const CVector_arma&);
CVector_arma operator*(const double&, const CVector_arma&);
CVector_arma operator*(const CVector_arma&, const double&);
CVector_arma operator/(const CVector_arma&, const double&);
CVector_arma operator/(const CVector_arma&, const CVector_arma&);
CVector_arma operator/(const double&, const CVector_arma&);
CVector_arma zeros_ar(int i);
double avg(CVector_arma &);
