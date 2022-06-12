#ifndef CONDUCTOR_H
#define CONDUCTOR_H

#include <string.h>
#include <map>
#include "sourcesinkdata.h"
#include "GA.h"

using namespace std;

class Conductor
{
public:
    Conductor();
    bool Execute(const string &command, map<string,string> arguments);
    SourceSinkData *Data() {return data;}
    void SetData(SourceSinkData *_data) {data = _data;}
private:
    SourceSinkData *data;
    CGA<SourceSinkData> *GA = nullptr;
};

#endif // CONDUCTOR_H
