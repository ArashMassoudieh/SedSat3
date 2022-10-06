#include "conductor.h"
#include "ProgressWindow.h"

Conductor::Conductor()
{

}

bool Conductor::Execute(const string &command, map<string,string> arguments)
{
    if (command == "GA")
    {
        if (GA!=nullptr) delete GA;
        ProgressWindow *rtw = new ProgressWindow();
        rtw->show();
        Data()->InitializeParametersObservations(arguments["Sample"]);
        GA = new CGA<SourceSinkData>(Data());
        GA->SetRunTimeWindow(rtw);
        GA->SetProperties(arguments);
        GA->InitiatePopulation();
        GA->optimize();
    }
    return true;
}
