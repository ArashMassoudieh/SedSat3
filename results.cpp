#include "results.h"
#include "resultitem.h"

Results::Results()
{

}

Results::Results(const Results &rhs): map<string, ResultItem>(rhs)
{
    name = rhs.name;
}
Results& Results::operator = (const Results &rhs)
{
    map<string, ResultItem>::operator=(rhs);
    name = rhs.name;
    return *this;
}

void Results::Append(const ResultItem &ritem)
{
    operator[](ritem.Name()) = ritem;
}

QJsonObject Results::toJsonObject()
{
    QJsonObject out;
    for (map<string,ResultItem>::iterator it = begin(); it!=end(); it++)
    {
        out[QString::fromStdString(it->first)] = it->second.toJsonObject();
    }
    return out;

}
