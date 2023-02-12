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
}
ResultItem& ResultItem::operator = (const ResultItem &rhs)
{
    Interface::operator=(rhs);
    name = rhs.name;
    type = rhs.type;
    result = rhs.result;
    y_axis_mode = rhs.y_axis_mode;
    showasstring = rhs.showasstring;
    return *this;
}
