#ifndef RESULTITEM_H
#define RESULTITEM_H
#include <string>
#include "parameter.h"

enum class result_type {timeseries, contribution, distribution, distribution_set, timeseries_set, samples, predicted_concentration, elemental_profile_set, mlrset, mlr, matrix, vector, vectorset, mcmc_samples, timeseries_set_first_symbol, timeseries_set_all_symbol, distribution_with_observed,rangeset, rangeset_with_observed, matrix1vs1, stacked_bar_chart};
enum class yaxis_mode {normal, log};
enum class xaxis_mode {real, counter};
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
    void SetXAxisMode(xaxis_mode mode) {x_axis_mode = mode; }
    yaxis_mode YAxisMode() {return y_axis_mode;}
    xaxis_mode XAxisMode() {return x_axis_mode; }
    void SetShowAsString(bool value) {showasstring = value; }
    bool ShowAsString() const {return showasstring;}
    void setXAxisTitle(const string &title) {

        Interface::SetXAsixLabel(QString::fromStdString(title));
        if (result)
        {
            result->SetXAsixLabel(QString::fromStdString(title));
        }
    }
    void setYAxisTitle(const string &title) {

        Interface::SetYAsixLabel(QString::fromStdString(title));
        if (result)
        {
            result->SetYAsixLabel(QString::fromStdString(title));
        }
    }
    string XAxisTitle() {return Interface::XAxisLabel().toStdString();}
    string YAxisTitle() {return Interface::YAxisLabel().toStdString();}
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
    Interface *result = nullptr;
    yaxis_mode y_axis_mode = yaxis_mode::log;
    xaxis_mode x_axis_mode = xaxis_mode::real;
    bool showabsvalue = false;
    bool showasstring = true;
    vector<double> YLimits;
    bool fixYlimit = false;
    bool showTable = false;
    bool showGraph = true;

};

#endif // RESULTITEM_H

