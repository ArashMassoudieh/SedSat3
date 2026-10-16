// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#include "observation.h"

Observation::Observation()
{

}

Observation::Observation(const Observation &obs)
{
    values = obs.values;
    name = obs.name;
}

Observation& Observation::operator=(const Observation &obs)
{
    values = obs.values;
    name = obs.name;
    return *this;
}

void Observation::SetValues(const TimeSeries<double> &_values)
{
    values = _values;
}

void Observation::AppendValues(const double &t, const double &val)
{
    values.append(t,val);
}
