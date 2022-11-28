#ifndef CONDUCTOR_H
#define CONDUCTOR_H

#include <string.h>
#include <map>
#include "sourcesinkdata.h"
#include "GA.h"
#include "results.h"

using namespace std;

class Conductor
{
public:
    Conductor();
    bool Execute(const string &command, map<string,string> arguments);
    SourceSinkData *Data() {return data;}
    void SetData(SourceSinkData *_data)
    {
	    data = _data;
    }
    Results *GetResults() {
        return new Results(results);
    }
    void SetWorkingFolder(const QString& wf) { workingfolder = wf; }
    QString WorkingFolder() { return workingfolder; }

private:
    SourceSinkData *data;
    CGA<SourceSinkData> *GA = nullptr;
    Results results;
    QString workingfolder;
};

#endif // CONDUCTOR_H
