#include "conductor.h"
#include "ProgressWindow.h"
#include "results.h"
#include "contribution.h"

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
        result_item result_cont = GA->Model_out.GetContribution();
        results.Append(result_cont);

        result_item result_modeled = GA->Model_out.GetPredictedElementalProfile();
        results.Append(result_modeled);

        result_item result_obs = GA->Model_out.GetObservedElementalProfile();
        results.Append(result_obs);

    }
    return true;
}
