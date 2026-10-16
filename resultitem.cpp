// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#include "resultitem.h"

ResultItem::ResultItem():Interface()
{
    YLimits.resize(2);
}

ResultItem::ResultItem(const ResultItem &rhs): Interface(rhs)
{
    name = rhs.name;
    type = rhs.type;
    result = rhs.result;
    y_axis_mode = rhs.y_axis_mode;
    x_axis_mode = rhs.x_axis_mode;
    showasstring = rhs.showasstring;
    fixYlimit = rhs.fixYlimit;
    YLimits = rhs.YLimits;
    showTable = rhs.showTable;
    showGraph = rhs.showGraph;
    showabsvalue = rhs.showabsvalue;
}
ResultItem& ResultItem::operator = (const ResultItem &rhs)
{
    if (this == &rhs)
        return *this;
    Interface::operator=(rhs);
    name = rhs.name;
    type = rhs.type;
    result = rhs.result;
    y_axis_mode = rhs.y_axis_mode;
    x_axis_mode = rhs.x_axis_mode;
    showasstring = rhs.showasstring;
    fixYlimit = rhs.fixYlimit;
    YLimits = rhs.YLimits;
    showTable = rhs.showTable;
    showGraph = rhs.showGraph;
    showabsvalue = rhs.showabsvalue;
    return *this;
}
