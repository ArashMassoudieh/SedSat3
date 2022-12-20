#include "results.h"
#include "resultitem.h"
#include "contribution.h"
#include "elemental_profile_set.h"

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
        out[QString::fromStdString(it->first)] = it->second.Result()->toJsonObject();
    }
    return out;

}

bool Results::ReadFromJson(const QJsonObject &jsonobject)
{
    for(QString key: jsonobject.keys() ) {

        if (key=="Contributions")
        {
            Contribution *contribution = new Contribution();
            contribution->ReadFromJsonObject(jsonobject[key].toObject());
            ResultItem res_item;
            res_item.SetName(key.toStdString());
            res_item.SetType(result_type::contribution);
            res_item.SetResult(contribution);
            operator[](key.toStdString()) = res_item;
        }
        else if (key=="Modeled Elemental Profile")
        {
            Elemental_Profile *modeled = new Elemental_Profile();
            modeled->ReadFromJsonObject(jsonobject[key].toObject());
            ResultItem res_item;
            res_item.SetName(key.toStdString());
            res_item.SetType(result_type::predicted_concentration);
            res_item.SetResult(modeled);
            operator[](key.toStdString()) = res_item;
        }
        else if (key=="Observed vs Modeled Elemental Profile")
        {
            Elemental_Profile_Set *modeled_vs_measured = new Elemental_Profile_Set();
            modeled_vs_measured->ReadFromJsonObject(jsonobject[key].toObject());
            ResultItem res_item;
            res_item.SetName(key.toStdString());
            res_item.SetType(result_type::elemental_profile_set);
            res_item.SetResult(modeled_vs_measured);
            operator[](key.toStdString()) = res_item;
        }

     }
    return true;

}
