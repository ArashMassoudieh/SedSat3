#include "results.h"

Results::Results()
{

}

Results::Results(const Results &rhs)
{
    result_items = rhs.result_items;
}
Results& Results::operator = (const Results &rhs)
{
    result_items = rhs.result_items;
    return *this;
}
