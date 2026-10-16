// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.


#pragma once
#include <vector>

//using namespace std;

class CBinary  
{
public:
	CBinary();
	CBinary(int n);
    CBinary(int n, int preci);
	virtual ~CBinary();
    int nDigits = 4;
    std::vector<int> Digit;
	int precision = 3;
	CBinary(const CBinary &B);
    CBinary operator = (const CBinary &B);
    CBinary operator + (const CBinary &B);
    CBinary extract(int spoint, int epoint);
    double decode(double minrange);
    int& operator[](unsigned int i); ///< Digit i; throws std::out_of_range if i >= Digit.size()
    void show();
	bool sign = true;
    void mutate(double mu);
	
};

CBinary code(double x, double minrange, double maxrange, int precision);
void cross(CBinary &B1, CBinary &B2, int p);
void cross(CBinary &B1, CBinary &B2, std::vector<int> p);
void cross2p(CBinary &B1, CBinary &B2, int p1, int p2);

