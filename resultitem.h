#ifndef RESULTITEM_H
#define RESULTITEM_H
#include <string>

enum class result_type {timeseries, contribution, distribution, distribution_set, timeseries_set, samples, predicted_concentration, elemental_profile_set};
#include "interface.h"

using namespace std;

class ResultItem: public Interface
{
public:
    ResultItem();
    Interface *Result() const {return result;}
    void SetResult(Interface *_result) {result = _result;}
    ResultItem(const ResultItem &rhs);
    ResultItem& operator = (const ResultItem &rhs);
    void SetName(const string &_name) {name = _name;}
    string Name() const {return name;}
    void SetType(const result_type &_type) {type = _type;}
    result_type Type() const {return type;}
private:
    string name;
    result_type type;
    Interface *result;

};

#endif // RESULTITEM_H
