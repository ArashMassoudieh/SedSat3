
#pragma once
#include <vector>

//using namespace std;

class GADistribution
{
public:
    GADistribution();
    virtual ~GADistribution();
	int n;
	std::vector<double> s;
	std::vector<double> e;
    GADistribution(int nn);
    GADistribution(const GADistribution &C);
    GADistribution operator = (const GADistribution &C);
    int GetRand();
    static double GetRndUniF(double xmin, double xmax);

};

