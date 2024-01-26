#include "conductor.h"
#include "ProgressWindow.h"
#include "results.h"
#include "contribution.h"
#include "resultitem.h"
#include "testmcmc.h"
#include "rangeset.h"
#include <QMessageBox>

#include "mainwindow.h"

Conductor::Conductor(MainWindow* _mainwindow)
{
    mainwindow = _mainwindow;
}

bool Conductor::Execute(const string &command, map<string,string> arguments)
{
    results.clear();
    if (!CheckNegativeElements(Data()))
        return false;

    if (command == "GA")
    {
        if (GA!=nullptr) delete GA;
        ProgressWindow *rtw = new ProgressWindow(mainwindow);

        bool organicnsizecorrection;
        if (arguments["Apply size and organic matter correction"]=="true")
        {   organicnsizecorrection = true;
            if (Data()->OMandSizeConstituents()[0]=="" && Data()->OMandSizeConstituents()[1]=="")
            {
                QMessageBox::warning(mainwindow, "OpenHydroQual","Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
        }
        else
            organicnsizecorrection = false;


        SourceSinkData correctedData = Data()->Corrected(arguments["Sample"],organicnsizecorrection,Data()->GetElementInformation());
        rtw->show();
        correctedData.InitializeParametersObservations(arguments["Sample"]);
        if (!CheckNegativeElements(&correctedData))
            return false;
        GA = new CGA<SourceSinkData>(&correctedData);
        GA->filenames.pathname = workingfolder.toStdString() + "/";
        GA->SetRunTimeWindow(rtw);
        GA->SetProperties(arguments);
        GA->InitiatePopulation();
        GA->optimize();

        results.SetName("GA " + arguments["Sample"]);

        ResultItem result_cont = GA->Model_out.GetContribution();
        result_cont.SetShowTable(true);
        result_cont.SetType(result_type::contribution);
        result_cont.SetShowGraph(true);
        results.Append(result_cont);

        ResultItem result_modeled_vs_measured = GA->Model_out.GetObservedvsModeledElementalProfile();
        result_modeled_vs_measured.SetShowGraph(true);
        result_modeled_vs_measured.SetShowTable(true);
        results.Append(result_modeled_vs_measured);

        ResultItem result_modeled_vs_measured_isotope = GA->Model_out.GetObservedvsModeledElementalProfile_Isotope();
        result_modeled_vs_measured_isotope.SetShowGraph(true);
        result_modeled_vs_measured_isotope.SetShowTable(true);
        results.Append(result_modeled_vs_measured_isotope);

        ResultItem result_calculated_means = GA->Model_out.GetCalculatedElementMeans();
        result_calculated_means.SetShowTable(true);
        result_calculated_means.SetShowGraph(false);
        results.Append(result_calculated_means);

        ResultItem result_estimated_means = GA->Model_out.GetEstimatedElementMean();
        result_estimated_means.SetShowTable(true);
        result_calculated_means.SetShowGraph(false);
        results.Append(result_estimated_means);

        ResultItem result_calculated_mu = GA->Model_out.GetCalculatedElementMu();
        result_calculated_mu.SetShowTable(true);
        result_calculated_mu.SetShowGraph(false);
        results.Append(result_calculated_mu);

        ResultItem result_estimated_mu = GA->Model_out.GetEstimatedElementMu();
        result_estimated_mu.SetShowTable(true);
        result_calculated_mu.SetShowGraph(false);
        results.Append(result_estimated_mu);

        ResultItem result_calculated_sigma = GA->Model_out.GetCalculatedElementSigma();
        result_calculated_sigma.SetShowTable(true);
        result_calculated_sigma.SetShowGraph(false);
        results.Append(result_calculated_sigma);

        ResultItem result_estimated_sigma = GA->Model_out.GetEstimatedElementSigma();
        result_estimated_sigma.SetShowTable(true);
        result_estimated_sigma.SetShowGraph(false);
        results.Append(result_estimated_sigma);


    }
    if (command == "GA (fixed elemental contribution)")
    {
        if (GA!=nullptr) delete GA;
        ProgressWindow *rtw = new ProgressWindow(mainwindow);
        rtw->SetTitle("Fitness",0);
        rtw->SetYAxisTitle("Fitness",0);
        bool organicnsizecorrection;
        if (arguments["Apply size and organic matter correction"]=="true")
        {   organicnsizecorrection = true;
            if (Data()->OMandSizeConstituents()[0]=="" && Data()->OMandSizeConstituents()[1]=="")
            {
                QMessageBox::warning(mainwindow, "OpenHydroQual","Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
        }
        else
            organicnsizecorrection = false;

        SourceSinkData correctedData = Data()->Corrected(arguments["Sample"],organicnsizecorrection,Data()->GetElementInformation());
        if (!CheckNegativeElements(&correctedData))
            return false;

        rtw->show();
        correctedData.InitializeParametersObservations(arguments["Sample"],estimation_mode::only_contributions);
        correctedData.SetParameterEstimationMode(estimation_mode::only_contributions);
        GA = new CGA<SourceSinkData>(&correctedData);
        GA->filenames.pathname = workingfolder.toStdString() + "/";
        GA->SetRunTimeWindow(rtw);
        GA->SetProperties(arguments);
        GA->InitiatePopulation();
        GA->optimize();
        results.SetName("GA (fixed profile) " + arguments["Sample"]);
        ResultItem result_cont = GA->Model_out.GetContribution();
        results.Append(result_cont);

        ResultItem result_modeled_vs_measured = GA->Model_out.GetObservedvsModeledElementalProfile(parameter_mode::direct);
        results.Append(result_modeled_vs_measured);

        ResultItem result_modeled_vs_measured_isotope = GA->Model_out.GetObservedvsModeledElementalProfile_Isotope(parameter_mode::direct);
        results.Append(result_modeled_vs_measured_isotope);

    }
    if (command == "GA (disregarding targets)")
    {
        if (GA!=nullptr) delete GA;
        ProgressWindow *rtw = new ProgressWindow(mainwindow);
        rtw->show();
        Data()->InitializeParametersObservations(arguments["Sample"],estimation_mode::source_elemental_profiles_based_on_source_data);
        Data()->SetParameterEstimationMode(estimation_mode::source_elemental_profiles_based_on_source_data);
        GA = new CGA<SourceSinkData>(Data());
        GA->filenames.pathname = workingfolder.toStdString() + "/";
        GA->SetRunTimeWindow(rtw);
        GA->SetProperties(arguments);
        GA->InitiatePopulation();
        GA->optimize();

        results.SetName("GA (no targets) " + arguments["Sample"]);
        ResultItem result_calculated_means = GA->Model_out.GetCalculatedElementMeans();
        results.Append(result_calculated_means);

        ResultItem result_estimated_means = GA->Model_out.GetEstimatedElementMean();
        results.Append(result_estimated_means);

        ResultItem result_calculated_stds = GA->Model_out.GetCalculatedElementSigma();
        results.Append(result_calculated_stds);

        ResultItem result_estimated_stds = GA->Model_out.GetEstimatedElementSigma();
        results.Append(result_estimated_stds);



    }
    if (command == "Levenberg-Marquardt")
    {
        ProgressWindow* rtw = new ProgressWindow(mainwindow);

        bool organicnsizecorrection;
        if (arguments["Apply size and organic matter correction"]=="true")
        {   organicnsizecorrection = true;
            if (Data()->OMandSizeConstituents()[0]=="" && Data()->OMandSizeConstituents()[1]=="")
            {
                QMessageBox::warning(mainwindow, "OpenHydroQual","Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
        }
        else
            organicnsizecorrection = false;

        rtw->show();
        SourceSinkData correctedData = Data()->Corrected(arguments["Sample"],organicnsizecorrection,Data()->GetElementInformation());
        if (!CheckNegativeElements(&correctedData))
            return false;

        correctedData.InitializeParametersObservations(arguments["Sample"]);
        correctedData.SetProgressWindow(rtw);
        if (arguments["Softmax transformation"]=="true")
            correctedData.SolveLevenBerg_Marquardt(transformation::softmax);
        else
            correctedData.SolveLevenBerg_Marquardt(transformation::linear);

        results.SetName("LM " + arguments["Sample"]);
        ResultItem result_cont = correctedData.GetContribution();
        result_cont.SetShowTable(true);
        results.Append(result_cont);

        ResultItem result_modeled = correctedData.GetPredictedElementalProfile(parameter_mode::direct);
        result_modeled.SetShowTable(true);
        results.Append(result_modeled);

        ResultItem result_modeled_vs_measured = correctedData.GetObservedvsModeledElementalProfile();
        result_modeled_vs_measured.SetShowTable(true);
        results.Append(result_modeled_vs_measured);

        ResultItem result_modeled_vs_measured_isotope = correctedData.GetObservedvsModeledElementalProfile_Isotope(parameter_mode::direct);
        result_modeled_vs_measured_isotope.SetShowTable(true);
        results.Append(result_modeled_vs_measured_isotope);

    }
    if (command == "Levenberg-Marquardt-Batch")
    {
        ProgressWindow* rtw = new ProgressWindow(mainwindow);

        bool organicnsizecorrection;
        if (arguments["Apply size and organic matter correction"]=="true")
        {   organicnsizecorrection = true;
            if (Data()->OMandSizeConstituents()[0]=="" && Data()->OMandSizeConstituents()[1]=="")
            {
                QMessageBox::warning(mainwindow, "OpenHydroQual","Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
        }
        else
            organicnsizecorrection = false;

        rtw->show();


        Data()->SetProgressWindow(rtw);
        CMBTimeSeriesSet *contributions;
        if (arguments["Softmax transformation"]=="true")
            contributions = new CMBTimeSeriesSet(Data()->LM_Batch(transformation::softmax,organicnsizecorrection));
        else
            contributions = new CMBTimeSeriesSet(Data()->LM_Batch(transformation::linear,organicnsizecorrection));

        results.SetName("LM-Batch");

        results.SetName("Source verification for source'" + arguments["Source Group"] +"'");
        contributions->GetOptions().X_suffix = "";
        contributions->GetOptions().Y_suffix = "";
        contributions->SetOption(options_key::single_column_x, true);
        ResultItem contributions_result_item;
        contributions_result_item.SetName("Levenberg-Marquardt-Batch");
        contributions_result_item.SetResult(contributions);
        contributions_result_item.SetType(result_type::timeseries_set_all_symbol);
        contributions_result_item.SetShowAsString(true);
        contributions_result_item.SetShowTable(true);
        contributions_result_item.SetShowGraph(true);
        contributions_result_item.SetYLimit(_range::high, 1);
        contributions_result_item.SetXAxisMode(xaxis_mode::counter);
        contributions_result_item.setYAxisTitle("Contribution");
        contributions_result_item.setXAxisTitle("Sample");
        contributions_result_item.SetYLimit(_range::low, 0);
        results.Append(contributions_result_item);

    }
    if (command == "OM-Size Correct")
    {
        bool exclude_samples = (arguments["Use only selected samples"]=="true"?true:false);
        bool exclude_elements = (arguments["Use only selected elements"]=="true"?true:false);
        Data()->SetSelectedTargetSample(arguments["Sample"]);
        SourceSinkData TransformedData = Data()->CopyandCorrect(exclude_samples, exclude_elements,true,arguments["Sample"]);
        vector<ResultItem> resultitems = TransformedData.GetSourceProfiles();
        results.SetName("Corrected Elemental Profiles for Target" + arguments["Sample"]);
        for (unsigned int i=0; i<resultitems.size(); i++)
            results.Append(resultitems[i]);
    }
    if (command == "MLR")
    {
        if (arguments["Organic Matter constituent"]=="" && arguments["Particle Size constituent"]=="")
        {   QMessageBox::information(mainwindow,"Exclude Elements","At least one of Organic Matter constituent and Particle Size constituent must be selected", QMessageBox::Ok);
            return false;
        }

        results.SetName("MLR_vs_OM&Size ");
        bool exclude_samples = (arguments["Use only selected samples"]=="true"?true:false);
        SourceSinkData TransformedData = Data()->CopyandCorrect(exclude_samples, false,false);
        if (arguments["Equation"]=="Linear")
            TransformedData.Perform_Regression_vs_om_size(arguments["Organic Matter constituent"],arguments["Particle Size constituent"],regression_form::linear);
        else
            TransformedData.Perform_Regression_vs_om_size(arguments["Organic Matter constituent"],arguments["Particle Size constituent"],regression_form::power);

        Data()->SetOMandSizeConstituents(arguments["Organic Matter constituent"],arguments["Particle Size constituent"]);
        for (map<string,Elemental_Profile_Set>::iterator it=Data()->begin(); it!=Data()->end(); it++)
        {
            if (it->first != Data()->TargetGroup())
            it->second.SetRegression(TransformedData[it->first].GetExistingRegressionSet());

        }
        vector<ResultItem> regression_result = TransformedData.GetMLRResults();
        for (unsigned int i=0; i<regression_result.size(); i++)
        {
            results.Append(regression_result[i]);
        }
    }
    if (command == "CovMat")
    {
        results.SetName("Covariance Matrix for " + arguments["Source/Target group"] );
        ResultItem covMatResItem;
        covMatResItem.SetName("Covariance Matrix for " + arguments["Source/Target group"] );
        covMatResItem.SetShowTable(true);
        covMatResItem.SetType(result_type::matrix);

        CMBMatrix *covmatr = new CMBMatrix(Data()->at(arguments["Source/Target group"]).CovarianceMatrix());
        covMatResItem.SetShowGraph(false);
        covMatResItem.SetResult(covmatr);
        results.Append(covMatResItem);

    }
    if (command == "CorMat")
    {
        results.SetName("Correlation Matrix for " + arguments["Source/Target group"] );
        ResultItem corMatResItem;
        corMatResItem.SetName("Correlation Matrix" );
        corMatResItem.SetShowTable(true);
        corMatResItem.SetType(result_type::matrix);
        corMatResItem.SetShowGraph(false);
        double threshold = QString::fromStdString(arguments["Threshold"]).toDouble();
        bool exclude_samples = (arguments["Use only selected samples"]=="true"?true:false);
        bool exclude_elements = (arguments["Use only selected elements"]=="true"?true:false);
        SourceSinkData TransformedData = Data()->CopyandCorrect(exclude_samples, exclude_elements,false);
        CMBMatrix *cormatr = new CMBMatrix(TransformedData.at(arguments["Source/Target group"]).CorrelationMatrix());
        cormatr->SetLimit(_range::high,threshold);
        cormatr->SetLimit(_range::low,-threshold);
        corMatResItem.SetResult(cormatr);
        results.Append(corMatResItem);

    }
    if (command == "DFA")
    {
        ProgressWindow* rtw = new ProgressWindow(mainwindow);
        rtw->show();
        results.SetName("DFA between " + arguments["Source/Target group I"] + "&" + arguments["Source/Target group II"] );
        ResultItem DFAResItem;
        DFAResItem.SetName("DFA coefficients");
        DFAResItem.SetType(result_type::vector);
        DFAResItem.SetShowTable(true);
        DFAResItem.SetAbsoluteValue(true);
        DFAResItem.SetYAxisMode(yaxis_mode::log);
        bool exclude_samples = (arguments["Use only selected samples"]=="true"?true:false);
        bool exclude_elements = (arguments["Use only selected elements"]=="true"?true:false);
        
        SourceSinkData TransformedData = Data()->CopyandCorrect(exclude_samples, exclude_elements, false);
        if (!CheckNegativeElements(&TransformedData))
            return false;

        if (arguments["OM and Size Correct based on target sample"] != "")
        {
            if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
            {
                QMessageBox::warning(mainwindow, "OpenHydroQual", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
            TransformedData = TransformedData.Corrected(arguments["OM and Size Correct based on target sample"], true, Data()->GetElementInformation());
        }
        
        TransformedData.SetProgressWindow(rtw);
        if (arguments["Box-cox transformation"]=="true")
            TransformedData = TransformedData.BoxCoxTransformed(true);

        DFA_result_vector *dfaeigenvector = new DFA_result_vector(TransformedData.DiscriminantFunctionAnalysis(arguments["Source/Target group I"],arguments["Source/Target group II"]));
        DFAResItem.SetResult(&dfaeigenvector->eigen_vector);
        results.Append(DFAResItem);



    }
    if (command == "SDFA")
    {
        ProgressWindow* rtw = new ProgressWindow(mainwindow);
        rtw->show();
        results.SetName("Stepwise DFA between " + arguments["Source/Target group I"] + "&" + arguments["Source/Target group II"] );
        ResultItem DFASValues;
        DFASValues.SetName("S-Values");
        DFASValues.SetType(result_type::vector);
        DFASValues.SetShowTable(true);
        DFASValues.SetAbsoluteValue(true);
        DFASValues.SetYAxisMode(yaxis_mode::log);
        bool exclude_samples = (arguments["Use only selected samples"]=="true"?true:false);
        bool exclude_elements = (arguments["Use only selected elements"]=="true"?true:false);
        SourceSinkData TransformedData = Data()->CopyandCorrect(exclude_samples, exclude_elements,false);
        if (!CheckNegativeElements(&TransformedData))
            return false;

        if (arguments["OM and Size Correct based on target sample"] != "")
        {
            if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
            {
                QMessageBox::warning(mainwindow, "OpenHydroQual", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
            TransformedData = TransformedData.Corrected(arguments["OM and Size Correct based on target sample"], true, Data()->GetElementInformation());
        }

        if (arguments["Box-cox transformation"]=="true")
            TransformedData = TransformedData.BoxCoxTransformed(true);
        TransformedData.SetProgressWindow(rtw);
        CMBVector *SVector = new CMBVector(TransformedData.Stepwise_DiscriminantFunctionAnalysis(arguments["Source/Target group I"],arguments["Source/Target group II"]));
        DFASValues.SetResult(SVector);
        results.Append(DFASValues);

    }
    if (command == "SDFAM")
    {
        ProgressWindow* rtw = new ProgressWindow(mainwindow);
        rtw->show();
        results.SetName("Stepwise DFA");
        ResultItem DFASValues;
        DFASValues.SetName("S-Values");
        DFASValues.SetType(result_type::vector);
        DFASValues.SetShowTable(true);
        DFASValues.SetAbsoluteValue(true);
        DFASValues.SetYAxisMode(yaxis_mode::log);
        bool exclude_samples = (arguments["Use only selected samples"]=="true"?true:false);
        bool exclude_elements = (arguments["Use only selected elements"]=="true"?true:false);
        SourceSinkData TransformedData = Data()->CopyandCorrect(exclude_samples, exclude_elements,false);
        if (arguments["OM and Size Correct based on target sample"] != "")
        {
            if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
            {
                QMessageBox::warning(mainwindow, "OpenHydroQual", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
            TransformedData = TransformedData.Corrected(arguments["OM and Size Correct based on target sample"], true, Data()->GetElementInformation());
        }

        if (!CheckNegativeElements(&TransformedData))
            return false;

        
        
        TransformedData.SetProgressWindow(rtw);
        if (arguments["Box-cox transformation"]=="true")
            TransformedData = TransformedData.BoxCoxTransformed(true);

        CMBVector *SVector = new CMBVector(TransformedData.Stepwise_DiscriminantFunctionAnalysis());
        DFASValues.SetResult(SVector);
        results.Append(DFASValues);
    }
    if (command == "SDFAM-2")
    {
        ProgressWindow* rtw = new ProgressWindow(mainwindow);
        rtw->show();
        results.SetName("Multi-group Stepwise DFA");
        ResultItem DFASValues;
        DFASValues.SetName("S-Values");
        DFASValues.SetType(result_type::vectorset);
        DFASValues.SetShowTable(true);
        DFASValues.SetAbsoluteValue(true);
        DFASValues.SetYAxisMode(yaxis_mode::log);
        bool exclude_samples = (arguments["Use only selected samples"]=="true"?true:false);
        bool exclude_elements = (arguments["Use only selected elements"]=="true"?true:false);
        SourceSinkData TransformedData = Data()->CopyandCorrect(exclude_samples, exclude_elements,false);
        if (arguments["OM and Size Correct based on target sample"] != "")
        {
            if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
            {
                QMessageBox::warning(mainwindow, "OpenHydroQual", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
            TransformedData = TransformedData.Corrected(arguments["OM and Size Correct based on target sample"], true, Data()->GetElementInformation());
        }

        if (!CheckNegativeElements(&TransformedData))
            return false;

        TransformedData.SetProgressWindow(rtw);
        if (arguments["Box-cox transformation"]=="true")
            TransformedData = TransformedData.BoxCoxTransformed(true);

        CMBVectorSet *SVector = new CMBVectorSet(TransformedData.Stepwise_DiscriminantFunctionAnalysis_MoreInfo());
        DFASValues.SetResult(SVector);
        results.Append(DFASValues);

    }
    if (command == "DFAM")
    {
        results.SetName("Multigroup DFA Analysis" );
        ResultItem DFAResItem;
        DFAResItem.SetName("Normal to the discriminant hyperplane vector");
        DFAResItem.SetType(result_type::matrix);
        DFAResItem.SetShowTable(true);
        DFAResItem.SetShowGraph(true);
        DFAResItem.SetAbsoluteValue(true);
        DFAResItem.SetYAxisMode(yaxis_mode::log);
        bool exclude_samples = (arguments["Use only selected samples"]=="true"?true:false);
        bool exclude_elements = (arguments["Use only selected elements"]=="true"?true:false);
        SourceSinkData TransformedData = Data()->CopyandCorrect(exclude_samples, exclude_elements,false);
        if (arguments["OM and Size Correct based on target sample"] != "")
        {
            if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
            {
                QMessageBox::warning(mainwindow, "OpenHydroQual", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
            TransformedData = TransformedData.Corrected(arguments["OM and Size Correct based on target sample"], true, Data()->GetElementInformation());
        }

        if (!CheckNegativeElements(&TransformedData))
                return false;
        if (arguments["Box-cox transformation"]=="true")
            TransformedData = TransformedData.BoxCoxTransformed(true);
        DFA_result_matrix *dfaeigenmatrix = new DFA_result_matrix(TransformedData.DiscriminantFunctionAnalysis());
        DFAResItem.SetResult(&dfaeigenmatrix->eigen_matrix);
        results.Append(DFAResItem);

        for (map<string,Elemental_Profile_Set>::iterator source1 = TransformedData.begin(); source1 != TransformedData.end(); source1++)
        {   for (map<string,Elemental_Profile_Set>::iterator source2 = std::next(source1,1); source2 != TransformedData.end(); source2++)
            {
                if (source1->first!=TransformedData.TargetGroup() && source2->first!=TransformedData.TargetGroup())
                {   ResultItem DFAResItemWeighted;
                    DFAResItemWeighted.SetName("DFA transformed between " + source1->first + " and " + source2->first);
                    DFAResItemWeighted.SetType(result_type::matrix1vs1);
                    DFAResItemWeighted.SetShowTable(true);
                    DFAResItemWeighted.SetShowGraph(true);
                    CMBVector weighted11 = TransformedData.DFATransformed(dfaeigenmatrix->eigen_matrix.GetRow(source1->first), source1->first);
                    CMBVector weighted12 = TransformedData.DFATransformed(dfaeigenmatrix->eigen_matrix.GetRow(source2->first), source1->first);
                    CMBVector weighted21 = TransformedData.DFATransformed(dfaeigenmatrix->eigen_matrix.GetRow(source1->first), source2->first);
                    CMBVector weighted22 = TransformedData.DFATransformed(dfaeigenmatrix->eigen_matrix.GetRow(source2->first), source2->first);
                    CMBMatrix *weighted_results = new CMBMatrix(2,weighted11.getsize()+weighted21.getsize());
                    for (unsigned int i=0; i<weighted11.getsize(); i++)
                    {
                        weighted_results->matr[i][0] = weighted11[i];
                        weighted_results->matr[i][1] = weighted12[i];
                        weighted_results->SetRowLabel(i,source1->first);
                    }
                    for (unsigned int i=0; i<weighted21.getsize(); i++)
                    {
                        weighted_results->matr[i+weighted11.getsize()][0] = weighted21[i];
                        weighted_results->matr[i+weighted11.getsize()][1] = weighted22[i];
                        weighted_results->SetRowLabel(i+weighted11.getsize(),source2->first);
                    }
                    weighted_results->SetColumnLabel(0,"WB_" + source1->first);
                    weighted_results->SetColumnLabel(1,"WB_" + source2->first);
                    DFAResItemWeighted.SetResult(weighted_results);
                    results.Append(DFAResItemWeighted);
                }
            }
        }


    }
    if (command == "DFA-Transformed")
    {
        results.SetName("DFA transformed between " + arguments["Source/Target group I"] + "&" + arguments["Source/Target group II"] );
        ResultItem DFAResItem;
        DFAResItem.SetName("DFA transformed");
        DFAResItem.SetType(result_type::matrix1vs1);
        DFAResItem.SetShowTable(true);
        DFAResItem.SetShowGraph(true);
        bool exclude_samples = (arguments["Use only selected samples"]=="true"?true:false);
        bool exclude_elements = (arguments["Use only selected elements"]=="true"?true:false);
        SourceSinkData TransformedData = Data()->CopyandCorrect(exclude_samples, exclude_elements,false);
        if (arguments["OM and Size Correct based on target sample"] != "")
        {
            if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
            {
                QMessageBox::warning(mainwindow, "OpenHydroQual", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
            TransformedData = TransformedData.Corrected(arguments["OM and Size Correct based on target sample"], true, Data()->GetElementInformation());
        }
        if (!CheckNegativeElements(&TransformedData))
            return false;

        DFA_result_matrix dfaeigenmatrix = TransformedData.DiscriminantFunctionAnalysis();
        CMBVector weighted11 = TransformedData.DFATransformed(dfaeigenmatrix.eigen_matrix.GetRow(arguments["Source/Target group I"]), arguments["Source/Target group I"]);
        CMBVector weighted12 = TransformedData.DFATransformed(dfaeigenmatrix.eigen_matrix.GetRow(arguments["Source/Target group II"]), arguments["Source/Target group I"]);
        CMBVector weighted21 = TransformedData.DFATransformed(dfaeigenmatrix.eigen_matrix.GetRow(arguments["Source/Target group I"]), arguments["Source/Target group II"]);
        CMBVector weighted22 = TransformedData.DFATransformed(dfaeigenmatrix.eigen_matrix.GetRow(arguments["Source/Target group II"]), arguments["Source/Target group II"]);
        CMBMatrix *weighted_results = new CMBMatrix(2,weighted11.getsize()+weighted21.getsize());
        for (unsigned int i=0; i<weighted11.getsize(); i++)
        {
            weighted_results->matr[i][0] = weighted11[i];
            weighted_results->matr[i][1] = weighted12[i];
            weighted_results->SetRowLabel(i,arguments["Source/Target group I"]);
        }
        for (unsigned int i=0; i<weighted21.getsize(); i++)
        {
            weighted_results->matr[i+weighted11.getsize()][0] = weighted21[i];
            weighted_results->matr[i+weighted11.getsize()][1] = weighted22[i];
            weighted_results->SetRowLabel(i+weighted11.getsize(),arguments["Source/Target group II"]);
        }
        weighted_results->SetColumnLabel(0,"WB_" + arguments["Source/Target group I"]);
        weighted_results->SetColumnLabel(1,"WB_" + arguments["Source/Target group II"]);
        DFAResItem.SetResult(weighted_results);
        results.Append(DFAResItem);

    }
    if (command == "KS")
    {
        results.SetName("Kolmogorov–Smirnov statististics for " + arguments["Source/Target group"] );
        ResultItem KSItem;
        KSItem.SetName("Kolmogorov–Smirnov statististics for " + arguments["Source/Target group"] );
        KSItem.SetType(result_type::vector);
        KSItem.SetYAxisMode(yaxis_mode::normal);
        distribution_type dist;
        if (arguments["Distribution"]=="Normal")
            dist = distribution_type::normal;
        else if (arguments["Distribution"]=="Lognormal")
            dist = distribution_type::lognormal;
        CMBVector *ksoutput = new CMBVector(Data()->at(arguments["Source/Target group"]).KolmogorovSmirnovStat(dist));
        KSItem.SetShowTable(true);
        KSItem.SetResult(ksoutput);
        results.Append(KSItem);
    }
    if (command == "KS-individual")
    {
        results.SetName("Kolmogorov–Smirnov statististics for constituent " +arguments["Constituent"] + " in group " + arguments["Source/Target group"]);
        ResultItem KSItem;
        KSItem.SetName("Kolmogorov–Smirnov statististics for constituent " +arguments["Constituent"] + " in group " + arguments["Source/Target group"]);
        KSItem.SetType(result_type::timeseries_set);
        distribution_type dist;
        if (arguments["Distribution"]=="Normal")
            dist = distribution_type::normal;
        else if (arguments["Distribution"]=="Lognormal")
            dist = distribution_type::lognormal;
        CMBTimeSeriesSet *ksoutput = new CMBTimeSeriesSet(Data()->at(arguments["Source/Target group"]).ElementalDistribution(arguments["Constituent"])->DataCDFnFitted(dist));
        KSItem.SetResult(ksoutput);
        KSItem.SetShowTable(false);
        results.Append(KSItem);
    }
    if (command == "CMB Bayesian")
    {
        results.SetName("MCMC results for '" +arguments["Sample"] + "'");
        ResultItem MCMC_samples;
        MCMC_samples.SetShowAsString(false);
        MCMC_samples.SetType(result_type::mcmc_samples);
        MCMC_samples.SetName("MCMC samples");
        CMBTimeSeriesSet *samples = new CMBTimeSeriesSet();

        bool organicnsizecorrection;
        if (arguments["Apply size and organic matter correction"]=="true")
        {
            if (Data()->OMandSizeConstituents()[0]=="" && Data()->OMandSizeConstituents()[1]=="")
            {
                QMessageBox::warning(mainwindow, "OpenHydroQual","Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
            organicnsizecorrection = true;

        }
        else
            organicnsizecorrection = false;

        SourceSinkData correctedData = Data()->Corrected(arguments["Sample"],organicnsizecorrection, Data()->GetElementInformation());
        if (!CheckNegativeElements(&correctedData))
            return false;
        if (MCMC!=nullptr) delete MCMC;
        MCMC = new CMCMC<SourceSinkData>();
        MCMC->Model = &correctedData;
        ProgressWindow *rtw = new ProgressWindow(mainwindow,3);
        rtw->SetTitle("Acceptance Rate",0);
        rtw->SetTitle("Purturbation Factor",1);
        rtw->SetTitle("Log posterior value",2);
        rtw->SetYAxisTitle("Acceptance Rate",0);
        rtw->SetYAxisTitle("Purturbation Factor",1);
        rtw->SetYAxisTitle("Log posterior value",2);
        rtw->show();
// Samples
        qDebug()<<2;
        correctedData.InitializeParametersObservations(arguments["Sample"]);
        MCMC->SetProperty("number_of_samples",arguments["Number of samples"]);
        MCMC->SetProperty("number_of_chains",arguments["Number of chains"]);
        MCMC->SetProperty("number_of_burnout_samples",arguments["Samples to be discarded (burnout)"]);
        MCMC->SetProperty("dissolve_chains",arguments["Dissolve Chains"]);
        qDebug()<<3;
        MCMC->initialize(samples,true);
        string folderpath;
        if (!QString::fromStdString(arguments["Samples File Name"]).contains("/"))
            folderpath = workingfolder.toStdString()+"/";
        qDebug()<<4;
        MCMC->step(QString::fromStdString(arguments["Number of chains"]).toInt(), QString::fromStdString(arguments["Number of samples"]).toInt(), folderpath + arguments["Samples File Name"], samples, rtw);
        vector<string> SourceGroupNames = correctedData.SourceGroupNames();
        samples->AppendLastContribution(SourceGroupNames.size()-1,SourceGroupNames[SourceGroupNames.size()-1]+"_Contribution");
        MCMC_samples.SetResult(samples);
        results.Append(MCMC_samples);
// Posterior distributions
        ResultItem distribution_res_item;
        CMBTimeSeriesSet *dists = new CMBTimeSeriesSet();
        *dists = samples->distribution(100,0,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
        distribution_res_item.SetName("Posterior Distributions");
        distribution_res_item.SetShowAsString(false);
        distribution_res_item.SetShowTable(true);
        distribution_res_item.SetType(result_type::distribution);
        distribution_res_item.SetResult(dists);
        results.Append(distribution_res_item);

//Posterior contribution 95% intervals
        RangeSet *contribution_credible_intervals = new RangeSet();
                for (unsigned int i=0; i<correctedData.SourceOrder().size(); i++)
        {
            Range range;
            double percentile_low = samples->BTC[i].percentile(0.025,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
            double percentile_high = samples->BTC[i].percentile(0.975,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
            double mean = samples->BTC[i].mean(QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
            double median = samples->BTC[i].percentile(0.5,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
            range.Set(_range::low,percentile_low);
            range.Set(_range::high,percentile_high);
            range.SetMean(mean);
            range.SetMedian(median);
            contribution_credible_intervals->operator[](samples->names[i]) = range;
        }
        ResultItem contribution_credible_intervals_result_item;

        contribution_credible_intervals_result_item.SetName("Source Contribution Credible Intervals");
        contribution_credible_intervals_result_item.SetShowAsString(true);
        contribution_credible_intervals_result_item.SetShowTable(true);
        contribution_credible_intervals_result_item.SetType(result_type::rangeset);
        contribution_credible_intervals_result_item.SetResult(contribution_credible_intervals);
        contribution_credible_intervals_result_item.SetYAxisMode(yaxis_mode::log);
        contribution_credible_intervals_result_item.SetYLimit(_range::high,1.0);
        results.Append(contribution_credible_intervals_result_item);

// Predicted 95% posterior distributions
        CMBTimeSeriesSet predicted_samples = MCMC->predicted;
        CMBTimeSeriesSet predicted_samples_elems;
        vector<string> ConstituentNames = correctedData.ElementOrder();
        vector<string> IsotopeNames = correctedData.IsotopeOrder();
        vector<string> AllNames = ConstituentNames;

        AllNames.insert(AllNames.end(),IsotopeNames.begin(), IsotopeNames.end());

        for (int i=0; i<predicted_samples.nvars; i++)
            predicted_samples.setname(i,AllNames[i]);



        for (int i=0; i<predicted_samples.nvars; i++)
        {
            if (correctedData.GetElementInformation(predicted_samples.names[i])->Role==element_information::role::element)
                predicted_samples_elems.append(predicted_samples[i],predicted_samples.names[i]);
        }
        ResultItem predicted_distribution_res_item;
        CMBTimeSeriesSet *predicted_dists_elems = new CMBTimeSeriesSet();

        *predicted_dists_elems = predicted_samples_elems.distribution(100,0,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
        for (int i=0; i<predicted_samples.nvars; i++)
            predicted_dists_elems->SetObservedValue(i,correctedData.observation(i)->Value());

        predicted_distribution_res_item.SetName("Posterior Predicted Constituents");
        predicted_distribution_res_item.SetShowAsString(false);
        predicted_distribution_res_item.SetShowTable(true);
        predicted_distribution_res_item.SetType(result_type::distribution_with_observed);
        predicted_distribution_res_item.SetResult(predicted_dists_elems);
        results.Append(predicted_distribution_res_item);

//predicted 95% credible intervals

        RangeSet *predicted_credible_intervals = new RangeSet();
        vector<double> percentile_low = predicted_samples.percentile(0.025,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
        vector<double> percentile_high = predicted_samples.percentile(0.975,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
        vector<double> mean = predicted_samples.mean(QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
        vector<double> median = predicted_samples.percentile(0.5,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
        for (int i=0; i<predicted_dists_elems->nvars; i++)
        {
            Range range;
            range.Set(_range::low,percentile_low[i]);
            range.Set(_range::high,percentile_high[i]);
            range.SetMean(mean[i]);
            range.SetMedian(median[i]);
            predicted_credible_intervals->operator[](predicted_dists_elems->names[i]) = range;
            predicted_credible_intervals->operator[](predicted_dists_elems->names[i]).SetValue(correctedData.observation(i)->Value());
        }
        ResultItem predicted_credible_intervals_result_item;
        predicted_credible_intervals_result_item.SetName("Predicted Samples Credible Intervals");
        predicted_credible_intervals_result_item.SetShowAsString(true);
        predicted_credible_intervals_result_item.SetShowTable(true);
        predicted_credible_intervals_result_item.SetType(result_type::rangeset_with_observed);
        predicted_credible_intervals_result_item.SetResult(predicted_credible_intervals);
        predicted_credible_intervals_result_item.SetYAxisMode(yaxis_mode::log);
        results.Append(predicted_credible_intervals_result_item);

        // Predicted 95% posterior distributions for isotopes
        CMBTimeSeriesSet predicted_samples_isotopes;

        for (int i=0; i<predicted_samples.nvars; i++)
        {
            if (correctedData.GetElementInformation(predicted_samples.names[i])->Role==element_information::role::isotope)
                predicted_samples_isotopes.append(predicted_samples[i],predicted_samples.names[i]);
        }
        ResultItem predicted_distribution_iso_res_item;
        CMBTimeSeriesSet *predicted_dists_isotopes = new CMBTimeSeriesSet();

        *predicted_dists_isotopes = predicted_samples_isotopes.distribution(100,0,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
        for (int i=0; i<predicted_samples_isotopes.nvars; i++)
            predicted_dists_isotopes->SetObservedValue(i,correctedData.observation(i+ConstituentNames.size())->Value());

        predicted_distribution_iso_res_item.SetName("Posterior Predicted Isotopes");
        predicted_distribution_iso_res_item.SetShowAsString(false);
        predicted_distribution_iso_res_item.SetShowTable(true);
        predicted_distribution_iso_res_item.SetType(result_type::distribution_with_observed);
        predicted_distribution_iso_res_item.SetResult(predicted_dists_isotopes);
        results.Append(predicted_distribution_iso_res_item);

        //predicted 95% credible intervals for isotopes

        RangeSet *predicted_credible_intervals_isotopes = new RangeSet();

        for (int i=0; i<predicted_dists_isotopes->nvars; i++)
        {
            Range range;
            range.Set(_range::low,percentile_low[i+correctedData.ElementOrder().size()]);
            range.Set(_range::high,percentile_high[i+correctedData.ElementOrder().size()]);
            range.SetMean(mean[i+correctedData.ElementOrder().size()]);
            range.SetMedian(median[i+correctedData.ElementOrder().size()]);
            predicted_credible_intervals_isotopes->operator[](predicted_dists_isotopes->names[i]) = range;
            predicted_credible_intervals_isotopes->operator[](predicted_dists_isotopes->names[i]).SetValue(correctedData.observation(i+correctedData.ElementOrder().size())->Value());
        }
        ResultItem predicted_credible_intervals_isotope_result_item;
        predicted_credible_intervals_isotope_result_item.SetName("Predicted Samples Credible Intervals for Isotopes");
        predicted_credible_intervals_isotope_result_item.SetShowAsString(true);
        predicted_credible_intervals_isotope_result_item.SetShowTable(true);
        predicted_credible_intervals_isotope_result_item.SetType(result_type::rangeset_with_observed);
        predicted_credible_intervals_isotope_result_item.SetResult(predicted_credible_intervals_isotopes);
        predicted_credible_intervals_isotope_result_item.SetYAxisMode(yaxis_mode::normal);
        results.Append(predicted_credible_intervals_isotope_result_item);

        rtw->SetProgress(1);
    }
    if (command == "Test CMB Bayesian")
    {
        results.SetName("MCMC results for testing MCMC'");
        ResultItem MCMC_samples;
        MCMC_samples.SetShowAsString(false);
        MCMC_samples.SetType(result_type::mcmc_samples);
        MCMC_samples.SetName("MCMC samples for testing MCMC'");
        CMBTimeSeriesSet *samples = new CMBTimeSeriesSet();
        CMCMC<TestMCMC> *mcmcfortesting = new CMCMC<TestMCMC>();
        if (MCMC!=nullptr) delete MCMC;
        TestMCMC testingmodel;
        mcmcfortesting->Model = &testingmodel;
        ProgressWindow *rtw = new ProgressWindow(mainwindow,3);
        rtw->SetTitle("Acceptance Rate",0);
        rtw->SetTitle("Purturbation Factor",1);
        rtw->SetTitle("Log posterior value",2);
        rtw->SetYAxisTitle("Acceptance Rate",0);
        rtw->SetYAxisTitle("Purturbation Factor",1);
        rtw->SetYAxisTitle("Log posterior value",2);
        rtw->show();
        vector<double> mins;
        vector<double> maxs;
        mins.push_back(0.1); mins.push_back(1);
        maxs.push_back(1.0); maxs.push_back(10);
        testingmodel.InitializeParametersObservations(mins,maxs);
        mcmcfortesting->SetProperty("number_of_samples",arguments["Number of samples"]);
        mcmcfortesting->SetProperty("number_of_chains",arguments["Number of chains"]);
        mcmcfortesting->SetProperty("number_of_burnout_samples",arguments["Samples to be discarded (burnout)"]);
        mcmcfortesting->initialize(samples,true);
        string folderpath;
        if (!QString::fromStdString(arguments["samples_file_name"]).contains("/"))
            folderpath = workingfolder.toStdString()+"/";
        mcmcfortesting->step(QString::fromStdString(arguments["Number of chains"]).toInt(), QString::fromStdString(arguments["Number of samples"]).toInt(), folderpath + arguments["samples_file_name"], samples, rtw);

        MCMC_samples.SetResult(samples);

        results.Append(MCMC_samples);
        ResultItem distribution_res_item;
        CMBTimeSeriesSet *dists = new CMBTimeSeriesSet();
        *dists = samples->distribution(100,0,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
        distribution_res_item.SetName("Posterior Distributions");
        distribution_res_item.SetShowAsString(false);
        distribution_res_item.SetType(result_type::distribution);
        distribution_res_item.SetResult(dists);
        results.Append(distribution_res_item);


    }

    if (command == "DF")
    {
        results.SetName("Distribution fitting results for '" +arguments["Constituent"] + "' in '" + arguments["Source/Target group"]);
        CMBTimeSeriesSet distributions;
        CMBTimeSeriesSet cummulative_distributions;
        ResultItem distribution_item;
        distribution_item.SetName("Fitted PDF for '" +arguments["Constituent"] + "' in '" + arguments["Source/Target group"]);
        distribution_item.SetShowAsString(false);
        distribution_item.SetType(result_type::timeseries_set_first_symbol);
        distribution_item.SetYAxisMode(yaxis_mode::normal);
        distribution_item.setXAxisTitle("Value");
        distribution_item.setYAxisTitle("PDF");
        ResultItem cummulative_distribution_item;
        cummulative_distribution_item.SetName("Fitted CDF for '" +arguments["Constituent"] + "' in '" + arguments["Source/Target group"]);
        cummulative_distribution_item.SetShowAsString(false);
        cummulative_distribution_item.SetType(result_type::timeseries_set_first_symbol);
        cummulative_distribution_item.SetYAxisMode(yaxis_mode::normal);
        cummulative_distribution_item.setXAxisTitle("Value");
        cummulative_distribution_item.setYAxisTitle("CDF");
        CMBTimeSeriesSet fitted_normal = Data()->at(arguments["Source/Target group"]).ElementalDistribution(arguments["Constituent"])->DistFitted(distribution_type::normal);
        CMBTimeSeriesSet fitted_lognormal = Data()->at(arguments["Source/Target group"]).ElementalDistribution(arguments["Constituent"])->DistFitted(distribution_type::lognormal);
        CMBTimeSeriesSet observed_fitted_normal_CDF = Data()->at(arguments["Source/Target group"]).ElementalDistribution(arguments["Constituent"])->DataCDFnFitted(distribution_type::normal);
        CMBTimeSeriesSet observed_fitted_lognormal_CDF = Data()->at(arguments["Source/Target group"]).ElementalDistribution(arguments["Constituent"])->DataCDFnFitted(distribution_type::lognormal);
        CMBTimeSeriesSet *PDF = new CMBTimeSeriesSet();
        PDF->append(fitted_normal["Observed"]);
        PDF->append(fitted_normal["Fitted"]);
        PDF->append(fitted_lognormal["Fitted"]);
        PDF->setname(0,"Samples");
        PDF->setname(1, "Normal");
        PDF->setname(2,"Log-normal");
        CMBTimeSeriesSet *CDF = new CMBTimeSeriesSet();
        CDF->append(observed_fitted_normal_CDF["Observed"]);
        CDF->append(observed_fitted_normal_CDF["Fitted"]);
        CDF->append(observed_fitted_lognormal_CDF["Fitted"]);
        CDF->setname(0,"Observed");
        CDF->setname(1, "Normal");
        CDF->setname(2,"Log-normal");
        distribution_item.SetResult(PDF);
        cummulative_distribution_item.SetResult(CDF);
        results.Append(distribution_item);
        results.Append(cummulative_distribution_item);

    }

    if (command == "Bracketing Analysis")
    {
        results.SetName("Bracketing analysis for sample '" + arguments["Sample"] + "'");
        ResultItem BracketingResItem;
        BracketingResItem.SetName("Bracketing results");
        BracketingResItem.SetType(result_type::vector);
        BracketingResItem.SetShowTable(true);
        BracketingResItem.SetShowGraph(false);
        bool exclude_samples = (arguments["Use only selected samples"]=="true"?true:false);
        bool exclude_elements = (arguments["Use only selected elements"]=="true"?true:false);
        SourceSinkData TransformedData = Data()->CopyandCorrect(exclude_samples, exclude_elements,false);
        if (!CheckNegativeElements(&TransformedData))
            return false;
        CMBVector *bracketingresult = new CMBVector(TransformedData.BracketTest(arguments["Sample"]));
        bracketingresult->SetBooleanValue(true);
        BracketingResItem.SetResult(bracketingresult);
        results.Append(BracketingResItem);

    }
    if (command == "BoxCox")
    {
        results.SetName("Box-Cox parameter for '" + arguments["Source/Target group"] + "'");
        ResultItem BoxCoxResItem;
        BoxCoxResItem.SetName("Box-Cox parameters");
        BoxCoxResItem.SetType(result_type::vector);
        BoxCoxResItem.SetShowTable(true);
        BoxCoxResItem.SetShowGraph(true);
        BoxCoxResItem.SetYAxisMode(yaxis_mode::normal);
        CMBVector *boxcoxparams = new CMBVector(Data()->at(arguments["Source/Target group"]).BoxCoxParameters());
        BoxCoxResItem.SetResult(boxcoxparams);
        results.Append(BoxCoxResItem);

    }
    if (command == "Outlier")
    {
        bool exclude_samples = (arguments["Use only selected samples"] == "true" ? true : false);
        bool exclude_elements = (arguments["Use only selected elements"] == "true" ? true : false);
        SourceSinkData TransformedData = Data()->CopyandCorrect(exclude_samples, exclude_elements, false);
        if (!CheckNegativeElements(&TransformedData))
            return false;
        results.SetName("Outlier analysis for '" + arguments["Source/Target group"] + "'");
        ResultItem OutlierResItem;
        OutlierResItem.SetName("Outlier Analysis");
        OutlierResItem.SetType(result_type::matrix);
        OutlierResItem.SetShowAsString(false);
        OutlierResItem.SetShowTable(true);
        OutlierResItem.SetShowGraph(false);
        double threshold = QString::fromStdString(arguments["Threshold"]).toDouble();
        CMBMatrix *outliermatrix = new CMBMatrix(TransformedData.at(arguments["Source/Target group"]).Outlier(-threshold,threshold));
        outliermatrix->SetLimit(_range::high,threshold);
        outliermatrix->SetLimit(_range::low,-threshold);
        OutlierResItem.SetResult(outliermatrix);
        results.Append(OutlierResItem);
    }
    if (command == "EDP")
    {
        bool log = (arguments["Log Transformation"] == "true" ? true : false);

        results.SetName("Two-way element discriminant power between '" + arguments["Source/Target group I"] + "' and '" + arguments["Source/Target group II"] +"'");
        ResultItem EDPresultStd;
        EDPresultStd.SetName("Discreminant difference to standard deviation ratio");
        EDPresultStd.SetType(result_type::predicted_concentration);
        EDPresultStd.SetShowAsString(false);
        EDPresultStd.SetShowTable(true);
        EDPresultStd.SetShowGraph(true);
        Elemental_Profile *EDPProfileSet = new Elemental_Profile(Data()->DifferentiationPower(arguments["Source/Target group I"], arguments["Source/Target group II"],log));
        EDPresultStd.SetYAxisMode(yaxis_mode::normal);
        EDPresultStd.setYAxisTitle("Discrimination power");
        EDPresultStd.SetResult(EDPProfileSet);

        results.Append(EDPresultStd);

        ResultItem EDPresultPercent;
        EDPresultPercent.SetName("Discriminat fraction");
        EDPresultPercent.SetType(result_type::predicted_concentration);
        EDPresultPercent.SetShowAsString(false);
        EDPresultPercent.setYAxisTitle("Percentage discriminated");
        EDPresultPercent.SetShowTable(true);
        EDPresultPercent.SetShowGraph(true);
        Elemental_Profile *EDPProfileSetPercent = new Elemental_Profile(Data()->DifferentiationPower_Percentage(arguments["Source/Target group I"], arguments["Source/Target group II"]));
        EDPresultPercent.SetYAxisMode(yaxis_mode::normal);
        EDPresultPercent.SetYLimit(_range::high,1);
        EDPresultPercent.SetResult(EDPProfileSetPercent);
        results.Append(EDPresultPercent);

        ResultItem EDPresult_pValue;
        EDPresult_pValue.SetName("Discriminat p-value");
        EDPresult_pValue.SetType(result_type::predicted_concentration);
        EDPresult_pValue.SetShowAsString(false);
        EDPresult_pValue.setYAxisTitle("p-Value");
        EDPresult_pValue.SetShowTable(true);
        EDPresult_pValue.SetShowGraph(true);
        Elemental_Profile *EDPProfileSet_pValue = new Elemental_Profile(Data()->t_TestPValue(arguments["Source/Target group I"], arguments["Source/Target group II"],log));
        EDPresult_pValue.SetYAxisMode(yaxis_mode::normal);
        EDPresult_pValue.SetYLimit(_range::high,1);
        EDPresult_pValue.SetResult(EDPProfileSet_pValue);
        EDPProfileSet_pValue->SetLimit(_range::high, aquiutils::atof(arguments["P-value threshold"]));
        EDPProfileSet_pValue->SetLimit(_range::low, 0);
        results.Append(EDPresult_pValue);
    }
    if (command == "EDPM")
    {
        bool log = (arguments["Log Transformation"] == "true" ? true : false);
        bool include_target = (arguments["Include target samples"] == "true" ? true : false);
        results.SetName("Multi-way element discriminant power between '" + arguments["Source/Target group I"] + "' and '" + arguments["Source/Target group II"] +"'");
        ResultItem EDPresultStd;
        EDPresultStd.SetName("Multi-way discreminant difference to standard deviation ratio");
        EDPresultStd.SetType(result_type::elemental_profile_set);
        EDPresultStd.SetShowAsString(true);
        EDPresultStd.SetShowTable(true);
        EDPresultStd.SetShowGraph(true);
        Elemental_Profile_Set *EDPProfileSet = new Elemental_Profile_Set(Data()->DifferentiationPower(log,include_target));
        EDPresultStd.SetYAxisMode(yaxis_mode::normal);
        EDPresultStd.setYAxisTitle("Discrimination power");
        EDPresultStd.SetResult(EDPProfileSet);

        results.Append(EDPresultStd);

        ResultItem EDPresultPercent;
        EDPresultPercent.SetName("Multi-way discriminat fraction");
        EDPresultPercent.SetType(result_type::elemental_profile_set);
        EDPresultPercent.SetShowAsString(true);
        EDPresultPercent.setYAxisTitle("Percentage discriminated");
        EDPresultPercent.SetShowTable(true);
        EDPresultPercent.SetShowGraph(true);
        Elemental_Profile_Set *EDPProfileSetPercent = new Elemental_Profile_Set(Data()->DifferentiationPower_Percentage(include_target));
        EDPresultPercent.SetYAxisMode(yaxis_mode::normal);
        EDPresultPercent.SetYLimit(_range::high,1);
        EDPresultPercent.SetResult(EDPProfileSetPercent);
        results.Append(EDPresultPercent);

        ResultItem EDP_pValue;
        EDP_pValue.SetName("Multi-way discriminat p-value");
        EDP_pValue.SetType(result_type::elemental_profile_set);
        EDP_pValue.SetShowAsString(true);
        EDP_pValue.setYAxisTitle("p-Value");
        EDP_pValue.SetShowTable(true);
        EDP_pValue.SetShowGraph(true);
        Elemental_Profile_Set *EDPProfileSetPValue = new Elemental_Profile_Set(Data()->DifferentiationPower_P_value(include_target));
        EDP_pValue.SetYAxisMode(yaxis_mode::normal);
        EDP_pValue.SetYLimit(_range::high,1);
        EDP_pValue.SetResult(EDPProfileSetPValue);
        EDPProfileSetPValue->SetLimit(_range::high, aquiutils::atof(arguments["P-value threshold"]));
        EDPProfileSetPValue->SetLimit(_range::low, 0);
        results.Append(EDP_pValue);

    }
    if (command == "ANOVA")
    {
        bool log = (arguments["Log Transformation"] == "true" ? true : false);

        results.SetName("ANOVA analysis");
        ResultItem Anovaresults;
        Anovaresults.SetName("ANOVA");
        Anovaresults.SetType(result_type::vector);
        Anovaresults.SetShowAsString(true);
        Anovaresults.SetShowTable(true);
        Anovaresults.SetShowGraph(true);
        CMBVector *PValues = new CMBVector(Data()->ANOVA(log));

        PValues->SetLimit(_range::high, aquiutils::atof(arguments["P-value threshold"]));
        PValues->SetLimit(_range::low, 0);
        Anovaresults.SetYAxisMode(yaxis_mode::normal);
        Anovaresults.setYAxisTitle("P-value");
        Anovaresults.SetResult(PValues);

        if (arguments["Modify the included elements based on the results"] == "true")
        {
            vector<string> selected = PValues->ExtractWithinRange(0,aquiutils::atof(arguments["P-value threshold"])).Labels();
            Data()->IncludeExcludeElementsBasedOn(selected);
        }

        results.Append(Anovaresults);

    }
    if (command == "Error_Analysis")
    {

        ProgressWindow* rtw = new ProgressWindow(mainwindow);

        bool organicnsizecorrection;
        if (arguments["Apply size and organic matter correction"]=="true")
        {   organicnsizecorrection = true;
            if (Data()->OMandSizeConstituents()[0]=="" && Data()->OMandSizeConstituents()[1]=="")
            {
                QMessageBox::warning(mainwindow, "OpenHydroQual","Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
        }
        else
            organicnsizecorrection = false;

        rtw->show();

        SourceSinkData correctedData = Data()->Corrected(arguments["Sample"],organicnsizecorrection,Data()->GetElementInformation());
        correctedData.SetProgressWindow(rtw);
        if (!CheckNegativeElements(&correctedData))
            return false;

        bool softmax = false;
        if (arguments["Softmax transformation"] == "true")
            softmax = true;

        correctedData.InitializeParametersObservations(arguments["Sample"]);
        CMBTimeSeriesSet *contributions = new CMBTimeSeriesSet(correctedData.BootStrap(aquiutils::atof(arguments["Pecentage eliminated"]),aquiutils::atoi(arguments["Number of realizations"]),arguments["Sample"],softmax));
        ResultItem contributions_result_item;
        results.SetName("Error Analysis for target sample'" + arguments["Sample"] +"'");
        contributions_result_item.SetName("Error Analysis");
        contributions_result_item.SetResult(contributions);
        contributions_result_item.SetType(result_type::timeseries_set_all_symbol);
        contributions_result_item.SetShowAsString(true);
        contributions_result_item.SetShowTable(true);
        contributions_result_item.SetShowGraph(true);
        contributions_result_item.SetYLimit(_range::high, 1);
        contributions_result_item.SetXAxisMode(xaxis_mode::counter);
        contributions_result_item.setYAxisTitle("Contribution");
        contributions_result_item.setXAxisTitle("Sample");
        contributions_result_item.SetYLimit(_range::low, 0);
        results.Append(contributions_result_item);

    }
    if (command == "Source_Verify")
    {

        ProgressWindow* rtw = new ProgressWindow(mainwindow);

        bool organicnsizecorrection;
        if (arguments["Apply size and organic matter correction"]=="true")
        {   organicnsizecorrection = true;
            if (Data()->OMandSizeConstituents()[0]=="" && Data()->OMandSizeConstituents()[1]=="")
            {
                QMessageBox::warning(mainwindow, "OpenHydroQual","Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
        }
        else
            organicnsizecorrection = false;

        rtw->show();

        SourceSinkData correctedData = Data()->Corrected(arguments["Sample"],organicnsizecorrection,Data()->GetElementInformation());
        correctedData.SetProgressWindow(rtw);
        if (!CheckNegativeElements(&correctedData))
            return false;

        bool softmax = false;
        if (arguments["Softmax transformation"] == "true")
            softmax = true;

        CMBTimeSeriesSet *contributions = new CMBTimeSeriesSet(correctedData.VerifySource(arguments["Source Group"],softmax));
        ResultItem contributions_result_item;
        results.SetName("Source verification for source'" + arguments["Source Group"] +"'");
        contributions->GetOptions().X_suffix = "";
        contributions->GetOptions().Y_suffix = "";
        contributions->SetOption(options_key::single_column_x, true);
        contributions_result_item.SetName("Source Verification");
        contributions_result_item.SetResult(contributions);
        contributions_result_item.SetType(result_type::timeseries_set_all_symbol);
        contributions_result_item.SetShowAsString(true);
        contributions_result_item.SetShowTable(true);
        contributions_result_item.SetShowGraph(true);
        contributions_result_item.SetYLimit(_range::high, 1);
        contributions_result_item.SetXAxisMode(xaxis_mode::counter);
        contributions_result_item.setYAxisTitle("Contribution");
        contributions_result_item.setXAxisTitle("Source Sample");
        contributions_result_item.SetYLimit(_range::low, 0);
        results.Append(contributions_result_item);

    }
    return true;
}

bool Conductor::CheckNegativeElements(SourceSinkData *_data)
{
    if (_data==nullptr)
        _data = Data();
    vector<string> NegativeCheckResults =_data->NegativeValueCheck();

    if (NegativeCheckResults.size()>0)
    {
        QString message;
        for (unsigned int i=0; i<NegativeCheckResults.size(); i++)
        {
            message += QString::fromStdString(NegativeCheckResults[i]+"\n");
        }
        QMessageBox::warning(mainwindow, "OpenHydroQual",message, QMessageBox::Ok);
        return false;
    }
    return true;

}
