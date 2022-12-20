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
}
ResultItem& ResultItem::operator = (const ResultItem &rhs)
{
    Interface::operator=(rhs);
    name = rhs.name;
    type = rhs.type;
    result = rhs.result;
    y_axis_mode = rhs.y_axis_mode;
    return *this;
}
