#include "resultitem.h"

ResultItem::ResultItem():Interface()
{

}

ResultItem::ResultItem(const ResultItem &rhs): Interface(rhs)
{
    name = rhs.name;
    type = rhs.type;
    result = rhs.result;
    y_axis_mode = rhs.y_axis_mode;
    showasstring = rhs.showasstring;
    X_Axis_Title = rhs.X_Axis_Title;
    Y_Axis_Title = rhs.Y_Axis_Title;
}
ResultItem& ResultItem::operator = (const ResultItem &rhs)
{
    Interface::operator=(rhs);
    name = rhs.name;
    type = rhs.type;
    result = rhs.result;
    y_axis_mode = rhs.y_axis_mode;
    showasstring = rhs.showasstring;
    X_Axis_Title = rhs.X_Axis_Title;
    Y_Axis_Title = rhs.Y_Axis_Title;
    return *this;
}
