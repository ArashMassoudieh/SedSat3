#ifndef RESULTITEM_H
#define RESULTITEM_H
#include <string>

enum class result_type {timeseries, contribution, distribution, distribution_set, timeseries_set, samples, predicted_concentration, elemental_profile_set, mlrset, mlr, matrix, vector, mcmc_samples, timeseries_set_first_symbol};
enum class yaxis_mode {normal, log};
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
    void SetYAxisMode(yaxis_mode mode) {y_axis_mode = mode;}
    yaxis_mode YAxisMode() {return y_axis_mode;}
    void SetShowAsString(bool value) {showasstring = value; }
    bool ShowAsString() const {return showasstring;}
    void setXAxisTitle(const string &title) {X_Axis_Title = title;}
    void setYAxisTitle(const string &title) {Y_Axis_Title = title;}
    string XAxisTitle() {return X_Axis_Title;}
    string YAxisTitle() {return Y_Axis_Title;}
private:
    string name;
    result_type type;
    Interface *result;
    yaxis_mode y_axis_mode = yaxis_mode::log;
    bool showasstring = true;
    string X_Axis_Title;
    string Y_Axis_Title;

};

#endif // RESULTITEM_H
