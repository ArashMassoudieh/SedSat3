#ifndef CONDUCTOR_H
#define CONDUCTOR_H

#include <string.h>
#include <map>
#include "sourcesinkdata.h"
#include "GA.h"
#include "MCMC.h"
#include "results.h"

using namespace std;

class MainWindow;

class Conductor
{
public:
    Conductor(MainWindow *mainwindow);

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
    bool CheckNegativeElements(SourceSinkData *data=nullptr);
private:
    SourceSinkData *data;
    CGA<SourceSinkData> *GA = nullptr;
    CMCMC<SourceSinkData> *MCMC = nullptr;
    Results results;
    QString workingfolder;
    MainWindow *mainwindow;
};

#endif // CONDUCTOR_H
