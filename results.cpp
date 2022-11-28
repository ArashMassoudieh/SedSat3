#include "results.h"

Results::Results()
{

}

Results::Results(const Results &rhs): map<string, result_item>(rhs)
{
    name = rhs.name;
}
Results& Results::operator = (const Results &rhs)
{
    map<string, result_item>::operator=(rhs);
    name = rhs.name;
    return *this;
}

void Results::Append(const result_item &ritem)
{
    operator[](ritem.name) = ritem;
}
