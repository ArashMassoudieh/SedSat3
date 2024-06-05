#ifndef OBSERVATION_H
#define OBSERVATION_H

#include "BTC.h"

class Observation
{
public:
    Observation();
    Observation(const Observation &obs);
    Observation& operator=(const Observation &obs);
    void SetName(const std::string &nam) {name = nam;}
    std::string Name() {return name;}
    std::string GetName() { return name; }
    void SetValues(const CTimeSeries<double> &values);
    CTimeSeries<double> Values() {return values;}
    double Value() {
        if (values.n>0)
            return values.GetC(0);
        else
            return 0;
    };
    void AppendValues(const double &t, const double &val);
    double PredictedValue() {return predicted_value;}
    void SetPredictedValue(const double &value) {predicted_value = value;}
private:
    CTimeSeries<double> values;
    double predicted_value = 0;
    std::string name;
};

#endif // OBSERVATION_H
