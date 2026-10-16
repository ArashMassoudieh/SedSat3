// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

// Vector.cpp: implementation of the CVector_arma_arma class.
//
//////////////////////////////////////////////////////////////////////

#include "Vector_arma.h"
#include "math.h"
#include "Matrix_arma.h"
#include <cfloat>
#include "Vector.h"
//#include "Expression.h"
#include "Utilities.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVector_arma::CVector_arma()
{

}

CVector_arma::CVector_arma(int n):arma::vec(n,fill::zeros)
{

}

CVector_arma::CVector_arma(const std::vector<double> a, int n)
{
    arma::vec::operator=(a);
}

CVector_arma::CVector_arma(const double x, int n)
{
    set_size(n);
    for (int i=0; i<size(); i++)
        at(i) = x;
}

CVector_arma::CVector_arma(const double x_min, const double x_max, int n):arma::vec(n)
{
    for (int i=0; i<num(); i++)
        at(i) = x_min+(x_max-x_min)/double(n)*i;
}

CVector_arma::CVector_arma(const CVector_arma &v):arma::vec(v)

{

}

CVector_arma::CVector_arma(const std::vector<double> &v)
{
    *this = v;
}

CVector_arma::CVector_arma(const vec & v):arma::vec(v)
{

}

CVector_arma::CVector_arma(CVector & v)
{

    set_size(v.num);
    for (int i = 0; i<size(); i++)
        at(i) = v[i];
}

CVector_arma::CVector_arma(const std::vector<int> &v)
{
    set_size(v.size());
    for (int i=0; i<size(); i++)
        at(i) = v[i];
}

CVector_arma::~CVector_arma()
{
    clear();
}

double& CVector_arma::operator[](int i)
{
	double *p = 0;
    if ((i<num()) & (i>-1))
        return at(i);
	else
		return *p;
}

double CVector_arma::operator[](int i) const
{
    if ((i<num()) & (i>-1))
        return at(i);
    else
        return -999;
}

int CVector_arma::range(int i)
{
	return i;
}

CVector_arma& CVector_arma::operator=(const CVector_arma &v)
{
    arma::vec::operator=(v);
	return *this;
}


CVector_arma & CVector_arma::operator=(const CVector &v)
{

    arma::vec::operator=(v.vec);
	return *this;
}

CVector_arma& CVector_arma::operator=(const std::vector<double> &v)
{

    arma::vec::operator=(v);
	return *this;
}

CVector_arma& CVector_arma::operator=(const double &v)
{
    for (int i=0; i<num(); ++i)
        at(i) = v;
	return *this;
}

CVector_arma& CVector_arma::operator+()
{return *this;}

void CVector_arma::swap(int i, int j)
{	double tmp = this->at(range(i));
    at(i) = at(range(j));
    at(j) = tmp;

}

int CVector_arma::getsize() const {return num();}

CVector_arma& CVector_arma::operator*=(double x)
{
    for (int i=0; i<num(); ++i)
        at(i) *= x;
	return *this;

}

bool CVector_arma::haszeros() const
{
    bool out = false;
    for (int i=0; i<num(); ++i)
        if (at(i) == 0) out = true;
    return out;
}

CVector_arma& CVector_arma::operator/=(double x)
{
    for (int i=0; i<num(); ++i)
        at(i) /= x;
	return *this;

}

CVector_arma& CVector_arma::operator+=(const CVector_arma &v)
{
    for (int i=0; i<num(); ++i)
        at(i)+=v[i];
	return *this;
}

CVector_arma& CVector_arma::operator-=(const CVector_arma &v)
{
    for (int i=0; i<num(); ++i)
        at(i)-=v[i];
	return *this;
}

CVector_arma operator+(const CVector_arma &v1, const CVector_arma &v2)
{
	CVector_arma v=v1;
    v+=v2;
	return v;
}

CVector_arma operator-(const CVector_arma &v1, const CVector_arma &v2)
{
	CVector_arma v=v1;
    v=arma::vec(v1)-arma::vec(v2);
	return v;

}

double dotproduct(const CVector_arma &v1, const CVector_arma &v2)
{
    return dot(arma::vec(v1), arma::vec(v2));
}

CVector_arma& CVector_arma::operator*=(const CVector_arma& v)
{
    *this *= arma::vec(v);
	return *this;
}

arma::vec &CVector_arma::vect()
{
    return *this;
}

CVector_arma operator*(const CVector_arma &v1, const CVector_arma &v2)
{
    CVector_arma out;
    out = arma::vec(v1)*arma::vec(v2);
	return out; 
}

double norm(CVector_arma v)
{
	double sum = 0;
    for (int i=0; i<v.num(); i++)
        sum += pow(v.at(i),2);
    return sqrt(sum/v.num());
}

CVector_arma operator*(const double &a, const CVector_arma &v)
{
    CVector_arma out = v;
    out *= a;
	return out;
}

CVector_arma operator*(const CVector_arma& v, const double &a)
{
    CVector_arma out = v;
    out *= a;
    return out;
}

CVector_arma operator/(const CVector_arma &v, const double &a)
{
    CVector_arma out = v;
    out /= a;
    return out;

}

CVector_arma operator+(const CVector_arma &v, const double &a)
{
    CVector_arma v1 = v;
    v1 += a;
	return v1;
}

CVector_arma operator+(const double &a, const CVector_arma &v)
{
    CVector_arma v1 = v;
    v1 += a;
	return v1;
}

CVector_arma operator-(const double &a, const CVector_arma &v)
{
    CVector_arma v1(v.num());
    for (int i=0; i<v.num(); i++)
		v1[i] = a - v[i];
	return v1;

}


CVector_arma operator/(const CVector_arma &v1, const CVector_arma &v2)
{
	CVector_arma x(v1.getsize());
	for (int i = 0; i<v1.getsize(); ++i)
		x[i] = v1[i]/v2[i];
	return x;
}

CVector_arma operator/(const double &a, const CVector_arma &v2)
{
	CVector_arma x(v2.getsize());
	for (int i = 0; i<v2.getsize(); ++i)
		x[i] = a/v2[i];
	return x;

}

CVector_arma zeros_ar(int i)
{
	return CVector_arma(i);
}

bool CVector_arma::operator==(double v)
{
	bool r=true;
    for (int i=0; i<num(); ++i)
        if (at(i) != v)
			r=false;
	return r;

}

bool CVector_arma::operator==(CVector_arma &v)
{
	bool r=true;
    if (num()!=v.num()) return false;
    for (int i=0; i<num(); ++i)
        if ((at(i)== v.at(i))!=true)
			r=false;
	return r;

}

bool CVector_arma::is_finite()
{
	bool r=true;
    for (int i=0; i<num(); ++i)
        if (aquiutils::isfinite(at(i))!=true)
			r=false;
	return r;
}

std::vector<int> CVector_arma::get_nan_elements()
{
	std::vector<int> out;
    for (int i = 0; i < num(); i++)
	{
        if ((at(i) == at(i)) != true || !aquiutils::isfinite(at(i)))
			out.push_back(i);
	}
	return out;
}

double CVector_arma::max()
{
    return arma::vec::max();
}

double max(CVector_arma &V)
{
	return V.max();
}

double CVector_arma::min()
{
    return arma::vec::min();
}

double min(CVector_arma &V)
{
	return V.min();
}
double CVector_arma::abs_max()
{
	double a = -1E14;
    for (int i=0;i<num(); i++)
	{
        if (fabs(at(i))>a)
            a = fabs(at(i));
	}
	return a;
}

int CVector_arma::abs_max_elems()
{
	double a = -1E14;
	int ii;
    for (int i = 0; i<num(); i++)
	{
        if (fabs(at(i)) > a)
		{
            a = fabs(at(i));
			ii = i;
		}
	}
	return ii;
}

double abs_max(CVector_arma &V)
{
	return V.abs_max();
}

double CVector_arma::norm2()
{
	double a = 0;
    for (int i=0;i<num(); i++)
	{
        a+=at(i)*at(i)/double(num());
	}
	return pow(a,0.5);

}

double &CVector_arma::vect(int i)
{
    return at(i);
}

double const &CVector_arma::vect(int i) const
{
    return at(i);
}

double CVector_arma::sum()
{
	double a = 0;
    for (int i=0;i<num(); i++)
	{
		a+=vect(i);
	}
	return a;
}

CMatrix_arma CVector_arma::T()
{
    CMatrix_arma K(1,num());
    for (int i=0; i<num(); i++)
		K.get(0,i) = vect(i);
	return K;
}

CVector_arma CVector_arma::Log()
{
	CVector_arma x(getsize());
	for (int i = 0; i<getsize(); i++)
		x[i] = log(vect(i));
	return x;
}

CVector_arma Log(CVector_arma &V)
{
	return V.Log();

}

double avg(CVector_arma &V)
{
    return V.sum()/V.num();
}

CVector_arma CVector_arma::Exp()
{
	CVector_arma x(getsize());
	for (int i = 0; i<getsize(); i++)
		x[i] = exp(vect(i));
	return x;
}

std::vector<int> CVector_arma::Int()
{
	std::vector<int> x(getsize());
	for (int i = 0; i<getsize(); i++)
		x[i] = int(vect(i));
	return x;
}

CVector_arma CVector_arma::abs()
{
	CVector_arma x(getsize());
	for (int i = 0; i<getsize(); i++)
		x[i] = fabs(vect(i));
	return x;
}


CVector_arma Exp(CVector_arma &V)
{
	return V.Exp();

}

CVector_arma abs(CVector_arma &V)
{
	return V.abs();
}

void CVector_arma::writetofile(FILE *f)
{
    for (int i=0; i<num(); i++)
		fprintf(f, "%le, ", vect(i));
	fprintf(f, "\n");
}

void CVector_arma::writetofile(std::ofstream &f)
{
    for (int i=0; i<num()-1; i++)
		f<<vect(i)<<",";
    f<<vect(num()-1)<<std::endl;

}

void CVector_arma::writetofile(std::string filename)
{
	FILE *f = fopen(filename.c_str(),"w");
	writetofile(f);
	fclose(f);
}

void CVector_arma::writetofile_app(std::string filename)
{
	FILE *f = fopen(filename.c_str(),"a");
	writetofile(f);
	fclose(f);
}

CMatrix_arma CVector_arma::diagmat()
{
    CMatrix_arma A(num(),num());
    for (int i=0; i<num(); i++)
        A.at(i,i) = vect(i);

	return A;

}

CVector_arma CVector_arma::append(const CVector_arma& V1)
{
    set_size(num() + V1.num());

    for (int i=0; i<V1.num(); i++)
        vect(i+num()) = V1.at(i);

	return *this;
}

CVector_arma CVector_arma::append(double d)
{
    set_size(num() + 1);
    at(num()) = d;
	return *this;
}

/*CVector_arma CVector_arma::sort()
{
	vec = QSort(vec);
	return *this;
}*/

/*CVector_arma combinesort(const CVector_arma& V1, const CVector_arma &V2)
{
	CVector_arma V3 = V1;
	CVector_arma V = V3.append(V2);
	return V.sort();

}*/

/*CVector_arma combinesort_s(const CVector_arma& V1, const CVector_arma &V2)
{
	int n1=0;
	int n2=0;
	CVector_arma V3;
	for (int i=0; i<V1.num+V2.num; i++)
	{
		if (n2==V2.num)
		{	V3.append(V1.vec[n1]);
			n1++;
		}
		else if (n1==V1.num)
		{	V3.append(V2.vec[n2]);
			n2++;
		}
		else
		{
			if (V1.vec[n1]>V2.vec[n2])
			{	V3.append(V2.vec[n2]);
				n2++;
			}
			else
			{	V3.append(V1.vec[n1]);
				n1++;
			}
		}
	}

	return V3;

}*/


std::vector<int> CVector_arma::lookup(double val)
{
	std::vector<int> res;
    for (int i=0; i<num(); i++)
		if (vect(i) == val)
			res.push_back(i);
	return res;
}

CVector_arma H(CVector_arma &V)
{
    CVector_arma x(V.num());
    for (int i = 0; i<V.num(); i++)
		x[i] = (fabs(V[i])+V[i])/2.0;
	return x;
}

CVector_arma operator*(CVector_arma &v, const double &d)
{
	return d*v;

}

void CVector_arma::print(std::string s)
{
	std::ofstream Afile;
	Afile.open(s);

    for (int i=0; i<num(); ++i)
		Afile << vect(i) << endl;

	Afile.close();

}

CVector_arma CVector_arma::operator=(mat A)
{
    set_size(A.n_rows);

    for (int i = 0; i<num(); ++i)
			vect(i)=A(i,0);

	return *this;
}

CVector_arma CVector_arma::sub(int i, int j)
{
	CVector_arma C(j-i);
	for (int ii=i; ii<j; ii++)
		C[ii-i] = vect(ii);

	return C;

}

std::string CVector_arma::toString() const
{
    std::string s = "[" + aquiutils::numbertostring(int(vect(0)));
    for (int i=1; i<num(); i++)
        s += "," + aquiutils::numbertostring(int(vect(i)));
    s+= "]";
    return s;

}

std::vector<int> CVector_arma::negative_elements()
{
    std::vector<int> out;
    for (int i=0; i<num(); i++)
        if (vect(i)<0)
            out.push_back(i);
    return out;

}

CVector_arma GetReal(const arma::cx_vec &vx)
{
    CVector_arma out(vx.size());
    for (int i=0; i<vx.size(); i++)
        out[i] = vx[i].real();

    return out;
}
CVector_arma GetImg(const arma::cx_vec &vx)
{
    CVector_arma out(vx.size());
    for (int i=0; i<vx.size(); i++)
        out[i] = vx[i].imag();

    return out;
}

//mat CVector_arma::operator=(const CVector_arma&V)
//{
//	mat A(num,1);

//	for (int i = 0; i<num; ++i)
//			A(i,0) = vec[i];

//	return A;
//}


