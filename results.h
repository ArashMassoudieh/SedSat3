#ifndef RESULTS_H
#define RESULTS_H
#include <map>
#include <interface.h>
#include <string>

using namespace std;

enum class result_type {timeseries, contribution, distribution, distribution_set, timeseries_set, samples, predicted_concentration, elemental_profile_set};

struct result_item
{
    string name;
    result_type type;
    Interface *result;

};

class Results: public map<string,result_item>
{
public:
    Results();
    Results(const Results &rhs);
    Results& operator = (const Results &rhs);
    void Append(const result_item&);
private:

};

#endif // RESULTS_H
