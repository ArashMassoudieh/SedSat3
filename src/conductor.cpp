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
        GA->filenames.pathname = workingfolder.toStdString() + "/";
        GA->SetRunTimeWindow(rtw);
        GA->SetProperties(arguments);
        GA->InitiatePopulation();
        GA->optimize();
        result_item result_cont = GA->Model_out.GetContribution();
        results.Append(result_cont);

        result_item result_calculated_means = GA->Model_out.GetCalculatedElementMeans();
        results.Append(result_calculated_means);

        result_item result_estimated_means = GA->Model_out.GetEstimatedElementMean(); 
        results.Append(result_estimated_means);

        result_item result_modeled_vs_measured = GA->Model_out.GetObservedvsModeledElementalProfile();
        results.Append(result_modeled_vs_measured);

    }
    if (command == "GA (fixed elemental contribution)")
    {
        if (GA!=nullptr) delete GA;
        ProgressWindow *rtw = new ProgressWindow();
        rtw->show();
        Data()->InitializeParametersObservations(arguments["Sample"],estimation_mode::only_contributions);
        Data()->SetParameterEstimationMode(estimation_mode::only_contributions);
        GA = new CGA<SourceSinkData>(Data());
        GA->filenames.pathname = workingfolder.toStdString() + "/";
        GA->SetRunTimeWindow(rtw);
        GA->SetProperties(arguments);
        GA->InitiatePopulation();
        GA->optimize();
        result_item result_cont = GA->Model_out.GetContribution();
        results.Append(result_cont);

        result_item result_modeled_vs_measured = GA->Model_out.GetObservedvsModeledElementalProfile(parameter_mode::direct);
        results.Append(result_modeled_vs_measured);

    }

    if (command == "Levenberg-Marquardt")
    {
        ProgressWindow* rtw = new ProgressWindow();
        rtw->show();
        Data()->InitializeParametersObservations(arguments["Sample"]);
        Data()->SetProgressWindow(rtw);
        if (arguments["Softmax transformation"]=="true")
            Data()->SolveLevenBerg_Marquardt(transformation::softmax);
        else
            Data()->SolveLevenBerg_Marquardt(transformation::linear);
        result_item result_cont = Data()->GetContribution();
        results.Append(result_cont);

        result_item result_modeled = Data()->GetPredictedElementalProfile(parameter_mode::direct);
        results.Append(result_modeled);

        result_item result_modeled_vs_measured = Data()->GetObservedvsModeledElementalProfile();
        results.Append(result_modeled_vs_measured);

    }
    return true;
}
