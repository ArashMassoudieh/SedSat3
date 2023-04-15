#ifndef RESULTITEM_H
#define RESULTITEM_H
#include <string>
#include "parameter.h"

enum class result_type {timeseries, contribution, distribution, distribution_set, timeseries_set, samples, predicted_concentration, elemental_profile_set, mlrset, mlr, matrix, vector, mcmc_samples, timeseries_set_first_symbol, distribution_with_observed,rangeset, rangeset_with_observed, matrix1vs1};
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
    void SetAbsoluteValue(bool val) {showabsvalue = val;}
    bool AbsValue() {return showabsvalue;}
    double YLimit(_range highlow)
    {
        if (highlow == _range::high)
            return YLimits[1];
        else
            return YLimits[0];
    }
    void SetYLimit(_range highlow,const double &value)
    {
        if (highlow == _range::high)
            YLimits[1] = value;
        else
            YLimits[0] = value;
        fixYlimit = true;
    }
    bool FixedYLimit()
    {
        return fixYlimit;
    }
    void SetShowTable(bool state) {showTable = state;}
    bool ShowTable() const {return showTable;}
    void SetShowGraph(bool state) {showGraph = state;}
    bool ShowGraph() const {return showGraph;}
private:
    string name;
    result_type type;
    Interface *result;
    yaxis_mode y_axis_mode = yaxis_mode::log;
    bool showabsvalue = false;
    bool showasstring = true;
    string X_Axis_Title;
    string Y_Axis_Title;
    vector<double> YLimits;
    bool fixYlimit = false;
    bool showTable = false;
    bool showGraph = true;

};

#endif // RESULTITEM_H

