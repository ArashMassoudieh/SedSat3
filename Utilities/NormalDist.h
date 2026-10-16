// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#pragma once
#include "Vector.h"
#include "Matrix.h"



class CNormalDist
{
public:
    CNormalDist(void);
    ~CNormalDist(void);
    static double unitrandom();
    static double getstdnormalrand();
    static double getnormalrand(double mu, double std);


};
CVector getnormal(CVector &mu, CMatrix &sigma);
CMatrix getnormal(int m, int n, double mu, double std);
CVector getnormal(int m, double mu, double std);
double getlognormalrand(double mu, double std);
CVector getlognormal(CVector &mu, CMatrix &sigma);
CMatrix getlognormal(int m, int n, double mu, double std);
CVector getlognormal(int m, double mu, double std);
double getnormalrand(double mu, double std);
double stdnormal_cdf(double u);
double getnormalcdf(double x, double mu = 0, double std = 1);
double stdnormal_cdf(double u);
double getstdnormalrand();
double stdnormal_inv(double p);
double unitrandom();

