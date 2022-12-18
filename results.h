#ifndef RESULTS_H
#define RESULTS_H
#include <map>
#include <interface.h>
#include <string>

using namespace std;

class QJsonObject;
class ResultItem;


class Results: public map<string,ResultItem>
{
public:
    Results();
    Results(const Results &rhs);
    Results& operator = (const Results &rhs);
    void Append(const ResultItem&);
    void SetName(const string &_name) {name = _name;}
    string GetName() {return name;}
    QJsonObject toJsonObject();
private:
    string name;

};

#endif // RESULTS_H
