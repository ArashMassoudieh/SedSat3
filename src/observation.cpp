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
