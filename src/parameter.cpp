#include "parameter.h"

Parameter::Parameter()
{
    range.resize(2);
}

double Parameter::GetVal(const string &quantity)
{

}

void Parameter::SetRange(const vector<double> &rng)
{
    if (rng.size()==2)
        range = rng;
}

double Parameter::GetRange(_range lowhigh)
{
    if (range.size()==2)
    {
        if (lowhigh ==_range::low)
            return range[0];
        else
            return range[1];
    }
    return 0;
}
void Parameter::SetRange(_range lowhigh, double value)
{
    if (range.size()!=2)
        range.resize(2);
    if (lowhigh ==_range::low)
        range[0] = value;
    else
        range[1] = value;
}
