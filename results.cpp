#include "results.h"

Results::Results()
{

}

Results::Results(const Results &rhs): map<string, result_item>()
{

}
Results& Results::operator = (const Results &rhs)
{
    map<string, result_item>::operator=(rhs);
    return *this;
}

void Results::Append(const result_item &ritem)
{
    operator[](ritem.name) = ritem;
}
