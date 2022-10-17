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

        result_item result_modeled = GA->Model_out.GetPredictedElementalProfile();
        results.Append(result_modeled);

        result_item result_obs = GA->Model_out.GetObservedElementalProfile();
        results.Append(result_obs);

        result_item result_calculated_means = GA->Model_out.GetCalculatedElementMeans();
        results.Append(result_calculated_means);

        result_item result_estimated_means = GA->Model_out.GetEstimatedElementMean(); 
        results.Append(result_estimated_means);

        result_item result_modeled_vs_measured = GA->Model_out.GetObservedvsModeledElementalProfile();
        results.Append(result_modeled_vs_measured);

    }
    if (command == "Levenberg-Marquardt")
    {
        ProgressWindow* rtw = new ProgressWindow();
        rtw->show();
        Data()->InitializeParametersObservations(arguments["Sample"]);
        Data()->InitializeContributionsRandomly();
        qDebug()<<QString::fromStdString(Data()->ContributionVector().toString());
        CVector V = Data()->ResidualVector();
        qDebug()<<QString::fromStdString(V.toString());
        CMatrix M = Data()->ResidualJacobian();
        qDebug()<<QString::fromStdString(M.toString()[0]);
        CMatrix JTJ = M*Transpose(M);
        CVector J_epsilon = M*V;
        qDebug()<<QString::fromStdString(J_epsilon.toString());
        CVector dx = J_epsilon/JTJ;
        qDebug()<<QString::fromStdString(dx.toString());
        qDebug() << "Matrix and Vector";
    }
    return true;
}
