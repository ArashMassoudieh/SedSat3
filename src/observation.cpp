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
