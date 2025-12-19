#include "conductor.h"
#include "ProgressWindow.h"
#include "results.h"
#include "contribution.h"
#include "resultitem.h"
#include "testmcmc.h"
#include "rangeset.h"
#include <QMessageBox>

#include "mainwindow.h"

Conductor::Conductor(MainWindow* mainwindow)
    : data(nullptr),
    mainwindow(mainwindow)
{
}

bool Conductor::Execute(const string &command, map<string,string> arguments)
{
    results.clear();
    if (!CheckNegativeElements(Data()))
        return false;

    if (command == "GA")
    {
        ProgressWindow *rtw = new ProgressWindow(mainwindow);

        bool organicnsizecorrection;
        if (arguments["Apply size and organic matter correction"]=="true")
        {   organicnsizecorrection = true;
            if (Data()->OMandSizeConstituents()[0]=="" && Data()->OMandSizeConstituents()[1]=="")
            {
                QMessageBox::warning(mainwindow, "SedSAT3","Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
        }
        else
            organicnsizecorrection = false;


        SourceSinkData correctedData = Data()->CreateCorrectedDataset(arguments["Sample"],organicnsizecorrection,Data()->GetElementInformation());
        rtw->show();
        correctedData.InitializeParametersAndObservations(arguments["Sample"]);
        if (!CheckNegativeElements(&correctedData))
            return false;
        GA = std::make_unique<CGA<SourceSinkData>>(&correctedData);
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

        ProgressWindow *rtw = new ProgressWindow(mainwindow);
        rtw->SetTitle("Fitness",0);
        rtw->SetYAxisTitle("Fitness",0);
        bool organicnsizecorrection;
        if (arguments["Apply size and organic matter correction"]=="true")
        {   organicnsizecorrection = true;
            if (Data()->OMandSizeConstituents()[0]=="" && Data()->OMandSizeConstituents()[1]=="")
            {
                QMessageBox::warning(mainwindow, "SedSAT3","Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
        }
        else
            organicnsizecorrection = false;

        SourceSinkData correctedData = Data()->CreateCorrectedDataset(arguments["Sample"],organicnsizecorrection,Data()->GetElementInformation());
        if (!CheckNegativeElements(&correctedData))
            return false;

        rtw->show();
        correctedData.InitializeParametersAndObservations(arguments["Sample"],estimation_mode::only_contributions);
        correctedData.SetParameterEstimationMode(estimation_mode::only_contributions);
        GA = std::make_unique<CGA<SourceSinkData>>(&correctedData);
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
        
        ProgressWindow *rtw = new ProgressWindow(mainwindow);
        rtw->show();
        Data()->InitializeParametersAndObservations(arguments["Sample"],estimation_mode::source_elemental_profiles_based_on_source_data);
        Data()->SetParameterEstimationMode(estimation_mode::source_elemental_profiles_based_on_source_data);
        GA = std::make_unique<CGA<SourceSinkData>>(Data());
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
                QMessageBox::warning(mainwindow, "SedSAT3","Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
        }
        else
            organicnsizecorrection = false;

        rtw->show();
        SourceSinkData correctedData = Data()->CreateCorrectedDataset(arguments["Sample"],organicnsizecorrection,Data()->GetElementInformation());
        if (!CheckNegativeElements(&correctedData))
            return false;

        correctedData.InitializeParametersAndObservations(arguments["Sample"]);
        correctedData.SetProgressWindow(rtw);
        if (arguments["Softmax transformation"]=="true")
            correctedData.SolveLevenberg_Marquardt(transformation::softmax);
        else
            correctedData.SolveLevenberg_Marquardt(transformation::linear);

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
        ProgressWindow* rtw = new ProgressWindow(mainwindow,0 );

        bool organicnsizecorrection;
        if (arguments["Apply size and organic matter correction"]=="true")
        {   organicnsizecorrection = true;
            if (Data()->OMandSizeConstituents()[0]=="" && Data()->OMandSizeConstituents()[1]=="")
            {
                QMessageBox::warning(mainwindow, "SedSAT3","Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
        }
        else
            organicnsizecorrection = false;

        rtw->show();


        Data()->SetProgressWindow(rtw);
        CMBTimeSeriesSet *contributions;
        map<string,vector<string>> negatives_elements;
        if (arguments["Softmax transformation"]=="true")
            contributions = new CMBTimeSeriesSet(Data()->LM_Batch(transformation::softmax,organicnsizecorrection,negatives_elements));
        else
            contributions = new CMBTimeSeriesSet(Data()->LM_Batch(transformation::linear,organicnsizecorrection,negatives_elements));

        if (negatives_elements.size()>0)
        {
            CheckNegativeElements(negatives_elements);
        }

        results.SetName("LM-Batch");

        contributions->GetOptions().X_suffix = "";
        contributions->GetOptions().Y_suffix = "";
        contributions->SetOption(options_key::single_column_x, true);
        ResultItem contributions_result_item;
        contributions_result_item.SetName("Levenberg-Marquardt-Batch");
        contributions_result_item.SetResult(contributions);
        contributions_result_item.SetType(result_type::stacked_bar_chart);
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
        SourceSinkData TransformedData = Data()->CreateCorrectedAndFilteredDataset(exclude_samples, exclude_elements,true,arguments["Sample"]);
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
        SourceSinkData TransformedData = Data()->CreateCorrectedAndFilteredDataset(exclude_samples, false,false);
        if (arguments["Equation"]=="Linear")
            TransformedData.PerformRegressionVsOMAndSize(arguments["Organic Matter constituent"],arguments["Particle Size constituent"],regression_form::linear, QString::fromStdString(arguments["P-value threshold"]).toDouble());
        else
            TransformedData.PerformRegressionVsOMAndSize(arguments["Organic Matter constituent"],arguments["Particle Size constituent"],regression_form::power, QString::fromStdString(arguments["P-value threshold"]).toDouble());

        Data()->SetOMandSizeConstituents(arguments["Organic Matter constituent"],arguments["Particle Size constituent"]);
        for (map<string,Elemental_Profile_Set>::iterator it=Data()->begin(); it!=Data()->end(); it++)
        {
            if (it->first != Data()->GetTargetGroup())
            it->second.SetRegressionModels(TransformedData[it->first].GetRegressionModels());

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

        CMBMatrix *covmatr = new CMBMatrix(Data()->at(arguments["Source/Target group"]).CalculateCovarianceMatrix());
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
        corMatResItem.setTableTitle("Correlation Matrix for source group '" + arguments["Source/Target group"] + "'");
        corMatResItem.SetType(result_type::matrix);
        corMatResItem.SetShowGraph(false);
        double threshold = QString::fromStdString(arguments["Threshold"]).toDouble();
        bool exclude_samples = (arguments["Use only selected samples"]=="true"?true:false);
        bool exclude_elements = (arguments["Use only selected elements"]=="true"?true:false);
        SourceSinkData TransformedData = Data()->CreateCorrectedAndFilteredDataset(exclude_samples, exclude_elements,false);
        if (arguments["OM and Size Correct based on target sample"] != "")
        {
            if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
            {
                QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
            TransformedData = TransformedData.CreateCorrectedDataset(arguments["OM and Size Correct based on target sample"], true, Data()->GetElementInformation());
        }
        CMBMatrix *cormatr = new CMBMatrix(TransformedData.at(arguments["Source/Target group"]).CalculateCorrelationMatrix());
        cormatr->SetLimit(_range::high,threshold);
        cormatr->SetLimit(_range::low,-threshold);
        corMatResItem.SetResult(cormatr);
        results.Append(corMatResItem);

    }
    if (command == "DFA")
    {
        if (arguments["Source/Target group I"] == arguments["Source/Target group II"])
        {
            QString message = "The selected sources must be different";
            QMessageBox::warning(mainwindow, "SedSAT3", message, QMessageBox::Ok);
            return false;
        }
        ProgressWindow* rtw = new ProgressWindow(mainwindow,0);
        rtw->show();
        results.SetName("DFA between " + arguments["Source/Target group I"] + "&" + arguments["Source/Target group II"] );

        bool exclude_samples = (arguments["Use only selected samples"]=="true"?true:false);
        bool exclude_elements = (arguments["Use only selected elements"]=="true"?true:false);
        
        SourceSinkData TransformedData = *Data();

        if (arguments["OM and Size Correct based on target sample"] != "")
        {
            if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
            {
                QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
            TransformedData = TransformedData.CreateCorrectedDataset(arguments["OM and Size Correct based on target sample"], true, Data()->GetElementInformation());
        }

        TransformedData = TransformedData.CreateCorrectedAndFilteredDataset(exclude_samples, exclude_elements, false);
        
        TransformedData.SetProgressWindow(rtw);
        if (arguments["Box-cox transformation"]=="true")
            TransformedData = TransformedData.BoxCoxTransformed(true);

        DFA_result dfa_res = TransformedData.DiscriminantFunctionAnalysis(arguments["Source/Target group I"], arguments["Source/Target group II"]);
        if (dfa_res.eigen_vectors.size()==0)
        {
            QMessageBox::warning(mainwindow, "SedSAT3", "Singular matrix in within group scatter matrix!\n", QMessageBox::Ok);
            return false;
        }
        CMBVector *p_value = new CMBVector(dfa_res.p_values);
        ResultItem DFA_P_Val;
        DFA_P_Val.SetName("Chi-squared P-Value");
        DFA_P_Val.SetType(result_type::vector);
        DFA_P_Val.SetShowTable(true);
        DFA_P_Val.SetShowGraph(false);

        DFA_P_Val.SetResult(p_value);
        results.Append(DFA_P_Val);

        CMBVector *f_test_p_value = new CMBVector(dfa_res.F_test_P_value);
        ResultItem DFA_F_Test_P_Val;
        DFA_F_Test_P_Val.SetName("F-test P-Value");
        DFA_F_Test_P_Val.SetType(result_type::vector);
        DFA_F_Test_P_Val.SetShowTable(true);
        DFA_F_Test_P_Val.SetShowGraph(false);

        DFA_F_Test_P_Val.SetResult(f_test_p_value);
        results.Append(DFA_F_Test_P_Val);


        ResultItem DFA_Projected;
        DFA_Projected.SetName("Projected Elemental Profiles");
        DFA_Projected.SetType(result_type::vectorset_groups);
        DFA_Projected.SetShowTable(true);
        DFA_Projected.SetShowGraph(true);
        DFA_Projected.SetYAxisMode(yaxis_mode::normal);
        CMBVectorSet *projected = new CMBVectorSet(dfa_res.projected);
        DFA_Projected.SetResult(projected);

        results.Append(DFA_Projected);

        ResultItem DFA_eigen_vector;
        DFA_Projected.SetName("Eigen vector");
        DFA_Projected.SetType(result_type::vector);
        DFA_Projected.SetShowTable(true);
        DFA_Projected.SetShowGraph(true);
        DFA_Projected.SetYAxisMode(yaxis_mode::normal);
        CMBVector *eigen_vector = new CMBVector(dfa_res.eigen_vectors.begin()->second);
        DFA_Projected.SetResult(eigen_vector);

        results.Append(DFA_Projected);


        rtw->SetProgress(1);

    }

    if (command == "DFAOnevsRest")
    {
        ProgressWindow* rtw = new ProgressWindow(mainwindow,0);
        rtw->show();
        results.SetName("DFA between " + arguments["Source group"] + "& the rest");

        bool exclude_samples = (arguments["Use only selected samples"]=="true"?true:false);
        bool exclude_elements = (arguments["Use only selected elements"]=="true"?true:false);

        SourceSinkData TransformedData = *Data();

        if (arguments["OM and Size Correct based on target sample"] != "")
        {
            if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
            {
                QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
            TransformedData = TransformedData.CreateCorrectedDataset(arguments["OM and Size Correct based on target sample"], true, Data()->GetElementInformation());
        }

        TransformedData = TransformedData.CreateCorrectedAndFilteredDataset(exclude_samples, exclude_elements, false);

        TransformedData.SetProgressWindow(rtw);
        if (arguments["Box-cox transformation"]=="true")
            TransformedData = TransformedData.BoxCoxTransformed(true);

        DFA_result dfa_res = TransformedData.DiscriminantFunctionAnalysis(arguments["Source group"]);
        if (dfa_res.eigen_vectors.size()==0)
        {
            QMessageBox::warning(mainwindow, "SedSAT3", "Singular matrix in within group scatter matrix!\n", QMessageBox::Ok);
            return false;
        }
        CMBVector *p_value = new CMBVector(dfa_res.p_values);
        ResultItem DFA_P_Val;
        DFA_P_Val.SetName("Chi-squared P-Value");
        DFA_P_Val.SetType(result_type::vector);
        DFA_P_Val.SetShowTable(true);
        DFA_P_Val.SetShowGraph(false);

        DFA_P_Val.SetResult(p_value);
        results.Append(DFA_P_Val);

        CMBVector *f_test_p_value = new CMBVector(dfa_res.F_test_P_value);
        ResultItem DFA_F_Test_P_Val;
        DFA_F_Test_P_Val.SetName("F-test P-Value");
        DFA_F_Test_P_Val.SetType(result_type::vector);
        DFA_F_Test_P_Val.SetShowTable(true);
        DFA_F_Test_P_Val.SetShowGraph(false);

        DFA_F_Test_P_Val.SetResult(f_test_p_value);
        results.Append(DFA_F_Test_P_Val);

        ResultItem DFA_Projected;
        DFA_Projected.SetName("Projected Elemental Profiles");
        DFA_Projected.SetType(result_type::vectorset_groups);
        DFA_Projected.SetShowTable(true);
        DFA_Projected.SetShowGraph(true);
        DFA_Projected.SetYAxisMode(yaxis_mode::normal);
        CMBVectorSet *projected = new CMBVectorSet(dfa_res.projected);
        DFA_Projected.SetResult(projected);

        results.Append(DFA_Projected);

        ResultItem DFA_eigen_vector;
        DFA_Projected.SetName("Eigen vector");
        DFA_Projected.SetType(result_type::vector);
        DFA_Projected.SetShowTable(true);
        DFA_Projected.SetShowGraph(true);
        DFA_Projected.SetYAxisMode(yaxis_mode::normal);
        CMBVector *eigen_vector = new CMBVector(dfa_res.eigen_vectors.begin()->second);
        DFA_Projected.SetResult(eigen_vector);

        results.Append(DFA_Projected);


        rtw->SetProgress(1);

    }
    if (command == "DFAM")
    {
        ProgressWindow* rtw = new ProgressWindow(mainwindow,0);
        rtw->show();
        results.SetName("Multi-way DFA analysis");

        bool exclude_samples = (arguments["Use only selected samples"]=="true"?true:false);
        bool exclude_elements = (arguments["Use only selected elements"]=="true"?true:false);

        SourceSinkData TransformedData = *Data();

        if (arguments["OM and Size Correct based on target sample"] != "")
        {
            if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
            {
                QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
            TransformedData = TransformedData.CreateCorrectedDataset(arguments["OM and Size Correct based on target sample"], true, Data()->GetElementInformation());
        }

        TransformedData = TransformedData.CreateCorrectedAndFilteredDataset(exclude_samples, exclude_elements, false);
        
        if (!CheckNegativeElements(&TransformedData))
            return false;

        TransformedData.SetProgressWindow(rtw);
        if (arguments["Box-cox transformation"]=="true")
            TransformedData = TransformedData.BoxCoxTransformed(true);

        DFA_result dfa_res = TransformedData.DiscriminantFunctionAnalysis();
        if (dfa_res.eigen_vectors.size()==0)
        {
            QMessageBox::warning(mainwindow, "SedSAT3", "Singular matrix in within group scatter matrix!\n", QMessageBox::Ok);
            return false;
        }
        CMBVector *p_value = new CMBVector(dfa_res.p_values);
        ResultItem DFA_P_Val;
        DFA_P_Val.SetName("Chi-squared P-Value");
        DFA_P_Val.SetType(result_type::vector);
        DFA_P_Val.SetShowTable(true);
        DFA_P_Val.SetShowGraph(false);

        DFA_P_Val.SetResult(p_value);
        results.Append(DFA_P_Val);

        CMBVector *f_test_p_value = new CMBVector(dfa_res.F_test_P_value);
        ResultItem DFA_F_Test_P_Val;
        DFA_F_Test_P_Val.SetName("F-test P-Value");
        DFA_F_Test_P_Val.SetType(result_type::vector);
        DFA_F_Test_P_Val.SetShowTable(true);
        DFA_F_Test_P_Val.SetShowGraph(false);

        DFA_F_Test_P_Val.SetResult(f_test_p_value);
        results.Append(DFA_F_Test_P_Val);

        ResultItem DFA_Projected;
        DFA_Projected.SetName("Multiway Projected Elemental Profiles");
        DFA_Projected.SetType(result_type::dfa_vectorsetset);
        DFA_Projected.SetShowTable(true);
        DFA_Projected.SetShowGraph(true);
        DFA_Projected.SetYAxisMode(yaxis_mode::normal);
        CMBVectorSetSet *projected = new CMBVectorSetSet(dfa_res.multi_projected);
        DFA_Projected.SetResult(projected);

        results.Append(DFA_Projected);

        ResultItem DFA_eigen_vector;
        DFA_Projected.SetName("Eigen vector");
        DFA_Projected.SetType(result_type::vectorset);
        DFA_Projected.SetShowTable(true);
        DFA_Projected.SetShowGraph(true);
        DFA_Projected.SetYAxisMode(yaxis_mode::normal);
        CMBVectorSet *eigen_vector = new CMBVectorSet(dfa_res.eigen_vectors);
        DFA_Projected.SetResult(eigen_vector);

        results.Append(DFA_Projected);


        rtw->SetProgress(1);

    }
    if (command == "SDFA")
    {
        if (arguments["Source/Target group I"] == arguments["Source/Target group II"])
        {
            QString message = "The selected sources must be different";
            QMessageBox::warning(mainwindow, "SedSAT3", message, QMessageBox::Ok);
            return false; 
        }
        ProgressWindow* rtw = new ProgressWindow(mainwindow,0);
        rtw->show();
        results.SetName("Stepwise DFA between " + arguments["Source/Target group I"] + "&" + arguments["Source/Target group II"] );

        bool exclude_samples = (arguments["Use only selected samples"]=="true"?true:false);
        bool exclude_elements = (arguments["Use only selected elements"]=="true"?true:false);
        bool OmandSizeCorrect = false;
        
        SourceSinkData TransformedData;
        if (arguments["OM and Size Correct based on target sample"] != "")
        {
            if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
            {
                QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
            OmandSizeCorrect = true;
            TransformedData = Data()->CreateCorrectedDataset(arguments["OM and Size Correct based on target sample"], true, Data()->GetElementInformation()).CreateCorrectedAndFilteredDataset(exclude_samples, exclude_elements,false);
        }
        else
            TransformedData = Data()->CreateCorrectedAndFilteredDataset(exclude_samples, exclude_elements,false);
        if (!CheckNegativeElements(&TransformedData))
            return false;


        if (arguments["Box-cox transformation"]=="true")
            TransformedData = TransformedData.BoxCoxTransformed(true);
        TransformedData.SetProgressWindow(rtw);
        vector<CMBVector> SDFA_res = TransformedData.StepwiseDiscriminantFunctionAnalysis(arguments["Source/Target group I"],arguments["Source/Target group II"]);
        if (SDFA_res[0].size()==0)
        {
            QMessageBox::warning(mainwindow, "SedSAT3", "Singular matrix in within group scatter matrix!\n", QMessageBox::Ok);
            return false;
        }
        CMBVector *PVector = new CMBVector(SDFA_res[0]);
        ResultItem DFASValues;
        DFASValues.SetName("Chi-squared P-Values");
        DFASValues.SetType(result_type::vector);
        DFASValues.SetShowTable(true);
        DFASValues.SetAbsoluteValue(true);
        DFASValues.SetYAxisMode(yaxis_mode::log);
        DFASValues.SetYLimit(_range::high,1);
        DFASValues.SetResult(PVector);
        
        

        CMBVector *WilksLambdaVector = new CMBVector(SDFA_res[1]);
        ResultItem DFASWilksLambdaValues;
        DFASWilksLambdaValues.SetName("Wilks' Lambda");
        DFASWilksLambdaValues.SetType(result_type::vector);
        DFASWilksLambdaValues.SetShowTable(true);
        DFASWilksLambdaValues.SetAbsoluteValue(true);
        DFASWilksLambdaValues.SetYAxisMode(yaxis_mode::log);
        DFASWilksLambdaValues.SetYLimit(_range::high,1);
        DFASWilksLambdaValues.SetResult(WilksLambdaVector);
        CMBVector *F_test_P_value = new CMBVector(SDFA_res[2]);
        ResultItem DFASF_Test_P_Value;
        DFASF_Test_P_Value.SetName("F-test P-Value");
        DFASF_Test_P_Value.SetType(result_type::vector);
        DFASF_Test_P_Value.SetShowTable(true);
        DFASF_Test_P_Value.SetAbsoluteValue(true);
        DFASF_Test_P_Value.SetYAxisMode(yaxis_mode::log);
        DFASF_Test_P_Value.SetYLimit(_range::high,1);
        DFASF_Test_P_Value.SetResult(F_test_P_value);

        results.Append(DFASValues);
        results.Append(DFASWilksLambdaValues);
        results.Append(DFASF_Test_P_Value);

    }

    if (command == "SDFAM")
    {
        ProgressWindow* rtw = new ProgressWindow(mainwindow,0);
        rtw->show();
        results.SetName("Multiway Stepwise DFA");

        bool exclude_samples = (arguments["Use only selected samples"]=="true"?true:false);
        bool exclude_elements = (arguments["Use only selected elements"]=="true"?true:false);
        bool OmandSizeCorrect = false;
        SourceSinkData TransformedData;
        if (arguments["OM and Size Correct based on target sample"] != "")
        {
            if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
            {
                QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
            OmandSizeCorrect = true;
            TransformedData = Data()->CreateCorrectedDataset(arguments["OM and Size Correct based on target sample"], true, Data()->GetElementInformation()).CreateCorrectedAndFilteredDataset(exclude_samples, exclude_elements,false);
        }
        else
            TransformedData = Data()->CreateCorrectedAndFilteredDataset(exclude_samples, exclude_elements,false);
        if (!CheckNegativeElements(&TransformedData))
            return false;


        if (arguments["Box-cox transformation"]=="true")
            TransformedData = TransformedData.BoxCoxTransformed(true);
        TransformedData.SetProgressWindow(rtw);
        vector<CMBVector> SDFA_res = TransformedData.StepwiseDiscriminantFunctionAnalysis();
        if (SDFA_res[0].size()==0)
        {
            QMessageBox::warning(mainwindow, "SedSAT3", "Singular matrix in within group scatter matrix!\n", QMessageBox::Ok);
            return false;
        }
        CMBVector *PVector = new CMBVector(SDFA_res[0]);
        ResultItem DFASValues;
        DFASValues.SetName("Chi-squared P-Values");
        DFASValues.SetType(result_type::vector);
        DFASValues.SetShowTable(true);
        DFASValues.SetAbsoluteValue(true);
        DFASValues.SetYAxisMode(yaxis_mode::log);
        DFASValues.SetYLimit(_range::high,1);
        DFASValues.SetResult(PVector);

        ResultItem DFASelected;
        DFASelected.SetName("Elements to be Selected");
        DFASelected.SetType(result_type::vector);
        DFASelected.SetShowTable(true);
        DFASelected.SetAbsoluteValue(true);
        DFASelected.SetYAxisMode(yaxis_mode::log);
        DFASelected.SetYLimit(_range::high, 1);
        
        CMBVector* PVectorSelected = new CMBVector();
        
        vector<string> selected = PVector->ExtractUpToMinimum().Labels();
        if (arguments["Modify the included elements based on the results"] == "true")
            Data()->IncludeExcludeElementsBasedOn(selected);
        for (int i = 0; i<selected.size(); i++)
        {
            PVectorSelected->append(selected[i], PVector->valueAt(i));
        }
        
        DFASelected.SetResult(PVectorSelected);
            
        CMBVector *WilksLambdaVector = new CMBVector(SDFA_res[1]);
        ResultItem DFASWilksLambdaValues;
        DFASWilksLambdaValues.SetName("Wilks' Lambda");
        DFASWilksLambdaValues.SetType(result_type::vector);
        DFASWilksLambdaValues.SetShowTable(true);
        DFASWilksLambdaValues.SetAbsoluteValue(true);
        DFASWilksLambdaValues.SetYAxisMode(yaxis_mode::log);
        DFASWilksLambdaValues.SetYLimit(_range::high,1);
        DFASWilksLambdaValues.SetResult(WilksLambdaVector);
        CMBVector *F_test_P_value = new CMBVector(SDFA_res[2]);
        ResultItem DFASF_Test_P_Value;
        DFASF_Test_P_Value.SetName("F-test P-Value");
        DFASF_Test_P_Value.SetType(result_type::vector);
        DFASF_Test_P_Value.SetShowTable(true);
        DFASF_Test_P_Value.SetAbsoluteValue(true);
        DFASF_Test_P_Value.SetYAxisMode(yaxis_mode::log);
        DFASF_Test_P_Value.SetYLimit(_range::high,1);
        DFASF_Test_P_Value.SetResult(F_test_P_value);

        results.Append(DFASValues);
		results.Append(DFASelected);
        results.Append(DFASWilksLambdaValues);
        results.Append(DFASF_Test_P_Value);

    }

    if (command == "SDFAOnevsRest")
    {
        ProgressWindow* rtw = new ProgressWindow(mainwindow,0);
        rtw->show();
        results.SetName("Stepwise DFA between " + arguments["Sourcegroup"] + "& the rest" );

        bool exclude_samples = (arguments["Use only selected samples"]=="true"?true:false);
        bool exclude_elements = (arguments["Use only selected elements"]=="true"?true:false);
        bool OmandSizeCorrect = false;
        SourceSinkData TransformedData;
        if (arguments["OM and Size Correct based on target sample"] != "")
        {
            if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
            {
                QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
            OmandSizeCorrect = true;
            TransformedData = Data()->CreateCorrectedDataset(arguments["OM and Size Correct based on target sample"], true, Data()->GetElementInformation()).CreateCorrectedAndFilteredDataset(exclude_samples, exclude_elements,false);
        }
        else
            TransformedData = Data()->CreateCorrectedAndFilteredDataset(exclude_samples, exclude_elements,false);
        if (!CheckNegativeElements(&TransformedData))
            return false;


        if (arguments["Box-cox transformation"]=="true")
            TransformedData = TransformedData.BoxCoxTransformed(true);
        TransformedData.SetProgressWindow(rtw);
        vector<CMBVector> SDFA_res = TransformedData.StepwiseDiscriminantFunctionAnalysis(arguments["Source group"]);
        if (SDFA_res[0].size()==0)
        {
            QMessageBox::warning(mainwindow, "SedSAT3", "Singular matrix in within group scatter matrix!\n", QMessageBox::Ok);
            return false;
        }
        CMBVector *PVector = new CMBVector(SDFA_res[0]);
        ResultItem DFASValues;
        DFASValues.SetName("Chi-squared P-Values");
        DFASValues.SetType(result_type::vector);
        DFASValues.SetShowTable(true);
        DFASValues.SetAbsoluteValue(true);
        DFASValues.SetYAxisMode(yaxis_mode::log);
        DFASValues.SetYLimit(_range::high,1);
        DFASValues.SetResult(PVector);
        CMBVector *WilksLambdaVector = new CMBVector(SDFA_res[1]);
        ResultItem DFASWilksLambdaValues;
        DFASWilksLambdaValues.SetName("Wilks' Lambda");
        DFASWilksLambdaValues.SetType(result_type::vector);
        DFASWilksLambdaValues.SetShowTable(true);
        DFASWilksLambdaValues.SetAbsoluteValue(true);
        DFASWilksLambdaValues.SetYAxisMode(yaxis_mode::log);
        DFASWilksLambdaValues.SetYLimit(_range::high,1);
        DFASWilksLambdaValues.SetResult(WilksLambdaVector);
        CMBVector *F_test_P_value = new CMBVector(SDFA_res[2]);
        ResultItem DFASF_Test_P_Value;
        DFASF_Test_P_Value.SetName("F-test P-Value");
        DFASF_Test_P_Value.SetType(result_type::vector);
        DFASF_Test_P_Value.SetShowTable(true);
        DFASF_Test_P_Value.SetAbsoluteValue(true);
        DFASF_Test_P_Value.SetYAxisMode(yaxis_mode::log);
        DFASF_Test_P_Value.SetYLimit(_range::high,1);
        DFASF_Test_P_Value.SetResult(F_test_P_value);

        results.Append(DFASValues);
        results.Append(DFASWilksLambdaValues);
        results.Append(DFASF_Test_P_Value);

    }
/*  if (command == "SDFAM")
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

        bool OmandSizeCorrect = false;
        SourceSinkData TransformedData;
        if (arguments["OM and Size Correct based on target sample"] != "")
        {
            if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
            {
                QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
            OmandSizeCorrect = true;
            TransformedData = Data()->Corrected(arguments["OM and Size Correct based on target sample"], true, Data()->GetElementInformation()).CopyandCorrect(exclude_samples, exclude_elements,false);
        }
        else
            TransformedData = Data()->CopyandCorrect(exclude_samples, exclude_elements,false);
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
        bool OmandSizeCorrect = false;
        SourceSinkData TransformedData;
        if (arguments["OM and Size Correct based on target sample"] != "")
        {
            if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
            {
                QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
            OmandSizeCorrect = true;
            TransformedData = Data()->Corrected(arguments["OM and Size Correct based on target sample"], true, Data()->GetElementInformation()).CopyandCorrect(exclude_samples, exclude_elements,false);
        }
        else
            TransformedData = Data()->CopyandCorrect(exclude_samples, exclude_elements,false);


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
                QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
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
        SourceSinkData TransformedData;
        bool OmandSizeCorrect = false;
        if (arguments["OM and Size Correct based on target sample"] != "")
        {
            if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
            {
                QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
            OmandSizeCorrect = true;
            TransformedData = Data()->Corrected(arguments["OM and Size Correct based on target sample"], true, Data()->GetElementInformation()).CopyandCorrect(exclude_samples, exclude_elements,false);
        }
        else
            TransformedData = Data()->CopyandCorrect(exclude_samples, exclude_elements,false);
        if (!CheckNegativeElements(&TransformedData))
            return false;

        if (arguments["Box-cox transformation"]=="true")
            TransformedData = TransformedData.BoxCoxTransformed(true);

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

    }*/
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
        CMBVector *ksoutput = new CMBVector(Data()->at(arguments["Source/Target group"]).CalculateKolmogorovSmirnovStatistics(dist));
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
        CMBTimeSeriesSet *ksoutput = new CMBTimeSeriesSet(Data()->at(arguments["Source/Target group"]).GetElementDistribution(arguments["Constituent"])->CreateCDFComparison(dist));
        KSItem.SetResult(ksoutput);
        KSItem.SetShowTable(false);
        results.Append(KSItem);
    }
    if (command == "CMB Bayesian")
    {
        if (arguments["Apply size and organic matter correction"]=="true")
        {
            if (Data()->OMandSizeConstituents()[0]=="" && Data()->OMandSizeConstituents()[1]=="")
            {
                QMessageBox::warning(mainwindow, "SedSAT3","Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
        }

        
        MCMC = std::make_unique<CMCMC<SourceSinkData>>();

        ProgressWindow *rtw = new ProgressWindow(mainwindow,3);
        rtw->SetTitle("Acceptance Rate",0);
        rtw->SetTitle("Purturbation Factor",1);
        rtw->SetTitle("Log posterior value",2);
        rtw->SetYAxisTitle("Acceptance Rate",0);
        rtw->SetYAxisTitle("Purturbation Factor",1);
        rtw->SetYAxisTitle("Log posterior value",2);
        rtw->show();
        results = Data()->MCMC(arguments["Sample"],arguments,MCMC.get(), rtw, workingfolder.toStdString());
        if (results.Error()!="")
        {
            QMessageBox::warning(mainwindow, "SedSat3",QString::fromStdString(results.Error()), QMessageBox::Ok);
            return false;
        }
        rtw->SetProgress(1);
    }
    if (command == "CMB Bayesian-Batch")
    {
        if (arguments["Apply size and organic matter correction"]=="true")
        {
            if (Data()->OMandSizeConstituents()[0]=="" && Data()->OMandSizeConstituents()[1]=="")
            {
                QMessageBox::warning(mainwindow, "SedSAT3","Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
        }

        
        MCMC = std::make_unique<CMCMC<SourceSinkData>>();

        ProgressWindow *rtw = new ProgressWindow(mainwindow,3, true);
        rtw->SetTitle("Acceptance Rate",0);
        rtw->SetTitle("Purturbation Factor",1);
        rtw->SetTitle("Log posterior value",2);
        rtw->SetYAxisTitle("Acceptance Rate",0);
        rtw->SetYAxisTitle("Purturbation Factor",1);
        rtw->SetYAxisTitle("Log posterior value",2);
        rtw->show();

        CMBMatrix* contributions = new CMBMatrix(Data()->MCMC_Batch(arguments,MCMC.get(), rtw, workingfolder.toStdString()));
// Need to add negative error check
        ResultItem contrib_matrix_item;
        contrib_matrix_item.SetName("Contribution Range Matrix");
        contrib_matrix_item.SetType(result_type::matrix);
        contrib_matrix_item.SetYAxisMode(yaxis_mode::normal);
        contrib_matrix_item.SetResult(contributions);
        contrib_matrix_item.SetShowTable(true);
        results.Append(contrib_matrix_item);
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
        *dists = samples->distribution(100,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
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
        /* only selected samples need to be added */

        bool exclude_samples = (arguments["Use only selected samples"]=="true"?true:false);

        SourceSinkData TransformedData;
        bool OmandSizeCorrect = false;
        if (arguments["OM and Size Correct based on target sample"] != "")
        {
            if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
            {
                QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
            OmandSizeCorrect = true;
            TransformedData = Data()->CreateCorrectedDataset(arguments["OM and Size Correct based on target sample"], true, Data()->GetElementInformation()).CreateCorrectedAndFilteredDataset(exclude_samples, false,false);
        }
        else
            TransformedData = Data()->CreateCorrectedAndFilteredDataset(exclude_samples, false,false);
        if (!CheckNegativeElements(&TransformedData))
            return false;

        if (arguments["Box-cox transformation"]=="true")
            TransformedData = TransformedData.BoxCoxTransformed(true);

        CMBTimeSeriesSet fitted_normal = TransformedData.at(arguments["Source/Target group"]).GetElementDistribution(arguments["Constituent"])->CreateFittedDistribution(distribution_type::normal);
        CMBTimeSeriesSet fitted_lognormal;
        if (arguments["Box-cox transformation"]!="true")
            fitted_lognormal = TransformedData.at(arguments["Source/Target group"]).GetElementDistribution(arguments["Constituent"])->CreateFittedDistribution(distribution_type::lognormal);
        CMBTimeSeriesSet observed_fitted_normal_CDF = TransformedData.at(arguments["Source/Target group"]).GetElementDistribution(arguments["Constituent"])->CreateCDFComparison(distribution_type::normal);
        CMBTimeSeriesSet observed_fitted_lognormal_CDF;
        if (arguments["Box-cox transformation"]!="true")
            observed_fitted_lognormal_CDF = TransformedData.at(arguments["Source/Target group"]).GetElementDistribution(arguments["Constituent"])->CreateCDFComparison(distribution_type::lognormal);
        CMBTimeSeriesSet *PDF = new CMBTimeSeriesSet();
        PDF->append(fitted_normal["Observed"]);
        PDF->append(fitted_normal["Fitted"]);
        if (arguments["Box-cox transformation"]!="true")
            PDF->append(fitted_lognormal["Fitted"]);
        PDF->setname(0,"Samples");
        PDF->setname(1, "Normal");
        if (arguments["Box-cox transformation"]!="true")
            PDF->setname(2,"Log-normal");
        CMBTimeSeriesSet *CDF = new CMBTimeSeriesSet();
        CDF->append(observed_fitted_normal_CDF["Observed"]);
        CDF->append(observed_fitted_normal_CDF["Fitted"]);
        if (arguments["Box-cox transformation"]!="true")
            CDF->append(observed_fitted_lognormal_CDF["Fitted"]);
        CDF->setname(0,"Observed");
        CDF->setname(1, "Normal");
        if (arguments["Box-cox transformation"]!="true")
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
        bool correct_based_on_size_and_organic_matter = (arguments["Correct based on size and organic matter"] == "true" ? true : false);
        SourceSinkData TransformedData = Data()->CreateCorrectedAndFilteredDataset(exclude_samples, exclude_elements,correct_based_on_size_and_organic_matter, arguments["Sample"]);
        if (!CheckNegativeElements(&TransformedData))
            return false;
        CMBVector *bracketingresult = new CMBVector(TransformedData.BracketTest(arguments["Sample"],false));
        bracketingresult->SetBooleanValue(true);
        BracketingResItem.SetResult(bracketingresult);
        results.Append(BracketingResItem);

    }
    if (command == "Bracketing Analysis Batch")
    {
        results.SetName("Bracketing analysis");
        ResultItem BracketingResItem;
        BracketingResItem.SetName("Bracketing results");
        BracketingResItem.SetType(result_type::matrix);
        BracketingResItem.SetShowTable(true);
        BracketingResItem.SetShowGraph(false);
        BracketingResItem.SetShowAsString(false);
        bool exclude_samples = (arguments["Use only selected samples"]=="true"?true:false);
        bool exclude_elements = (arguments["Use only selected elements"]=="true"?true:false);

        if (!CheckNegativeElements(data))
            return false;
        bool correct_based_on_size_and_organic_matter = (arguments["Correct based on size and organic matter"] == "true" ? true : false);;
        if (correct_based_on_size_and_organic_matter)
        {
            if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
            {
                QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
        }
        
        CMBMatrix *bracketingresult = new CMBMatrix(Data()->BracketTest(correct_based_on_size_and_organic_matter,exclude_elements,exclude_samples));
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
        CMBVector *boxcoxparams = new CMBVector(Data()->at(arguments["Source/Target group"]).CalculateBoxCoxParameters());
        BoxCoxResItem.SetResult(boxcoxparams);
        results.Append(BoxCoxResItem);

    }
    if (command == "Outlier")
    {
        bool exclude_samples = (arguments["Use only selected samples"] == "true" ? true : false);
        bool exclude_elements = (arguments["Use only selected elements"] == "true" ? true : false);
        SourceSinkData TransformedData = Data()->CreateCorrectedAndFilteredDataset(exclude_samples, exclude_elements, false);
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
        CMBMatrix *outliermatrix = new CMBMatrix(TransformedData.at(arguments["Source/Target group"]).DetectOutliers(-threshold,threshold));
        outliermatrix->SetLimit(_range::high,threshold);
        outliermatrix->SetLimit(_range::low,-threshold);
        OutlierResItem.SetResult(outliermatrix);
        results.Append(OutlierResItem);
    }
    if (command == "EDP")
    {
        results.SetName("Two-way element discriminant power between '" + arguments["Source/Target group I"] + "' and '" + arguments["Source/Target group II"] +"'");

        bool exclude_samples = (arguments["Use only selected samples"]=="true"?true:false);

        SourceSinkData TransformedData;
        bool OmandSizeCorrect = false;
        if (arguments["OM and Size Correct based on target sample"] != "")
        {
            if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
            {
                QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
            OmandSizeCorrect = true;
            TransformedData = Data()->CreateCorrectedDataset(arguments["OM and Size Correct based on target sample"], true, Data()->GetElementInformation()).CreateCorrectedAndFilteredDataset(exclude_samples, false,false);
        }
        else
            TransformedData = Data()->CreateCorrectedAndFilteredDataset(exclude_samples, false,false);
        if (!CheckNegativeElements(&TransformedData))
            return false;

        if (arguments["Box-cox transformation"]=="true")
            TransformedData = TransformedData.BoxCoxTransformed(true);

        ResultItem EDPresultStd;
        EDPresultStd.SetName("Discreminant difference to standard deviation ratio");
        EDPresultStd.SetType(result_type::predicted_concentration);
        EDPresultStd.SetShowAsString(false);
        EDPresultStd.SetShowTable(true);
        EDPresultStd.SetShowGraph(true);
        Elemental_Profile *EDPProfileSet = new Elemental_Profile(TransformedData.DifferentiationPower(arguments["Source/Target group I"], arguments["Source/Target group II"],false));
        EDPresultStd.SetYAxisMode(yaxis_mode::normal);
        EDPresultStd.setYAxisTitle("Discrimination power");
        EDPresultStd.SetResult(EDPProfileSet);
        EDPresultStd.setXAxisTitle("Element");
        EDPresultStd.setYAxisTitle("Standard deviation to mean ratio");
        results.Append(EDPresultStd);

        ResultItem EDPresultPercent;
        EDPresultPercent.SetName("Discriminat fraction");
        EDPresultPercent.SetType(result_type::predicted_concentration);
        EDPresultPercent.SetShowAsString(false);
        EDPresultPercent.setYAxisTitle("Percentage discriminated");
        EDPresultPercent.SetShowTable(true);
        EDPresultPercent.SetShowGraph(true);
        Elemental_Profile *EDPProfileSetPercent = new Elemental_Profile(TransformedData.DifferentiationPower_Percentage(arguments["Source/Target group I"], arguments["Source/Target group II"]));
        EDPresultPercent.SetYAxisMode(yaxis_mode::normal);
        EDPresultPercent.SetYLimit(_range::high,1);
        EDPresultPercent.SetResult(EDPProfileSetPercent);
        EDPresultPercent.setXAxisTitle("Element");
        EDPresultPercent.setYAxisTitle("Discriminant fraction");
        results.Append(EDPresultPercent);


        ResultItem EDPresult_pValue;
        EDPresult_pValue.SetName("Discriminat p-value");
        EDPresult_pValue.SetType(result_type::predicted_concentration);
        EDPresult_pValue.SetShowAsString(false);
        EDPresult_pValue.setYAxisTitle("p-Value");
        EDPresult_pValue.SetShowTable(true);
        EDPresult_pValue.SetShowGraph(true);
        Elemental_Profile *EDPProfileSet_pValue = new Elemental_Profile(TransformedData.t_TestPValue(arguments["Source/Target group I"], arguments["Source/Target group II"],false));
        EDPresult_pValue.SetYAxisMode(yaxis_mode::normal);
        EDPresult_pValue.SetYLimit(_range::high,1);
        EDPresult_pValue.SetResult(EDPProfileSet_pValue);
        EDPProfileSet_pValue->SetLimit(_range::high, aquiutils::atof(arguments["P-value threshold"]));
        EDPProfileSet_pValue->SetLimit(_range::low, 0);
        EDPresult_pValue.setXAxisTitle("Element");
        EDPresult_pValue.setYAxisTitle("Discriminant fraction");
        results.Append(EDPresult_pValue);

    }
    if (command == "EDPM")
    {

        bool include_target = (arguments["Include target samples"] == "true" ? true : false);

        bool exclude_samples = (arguments["Use only selected samples"]=="true"?true:false);

        SourceSinkData TransformedData;
        bool OmandSizeCorrect = false;
        if (arguments["OM and Size Correct based on target sample"] != "")
        {
            if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
            {
                QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
            OmandSizeCorrect = true;
            TransformedData = Data()->CreateCorrectedDataset(arguments["OM and Size Correct based on target sample"], true, Data()->GetElementInformation()).CreateCorrectedAndFilteredDataset(exclude_samples, false,false);
        }
        else
            TransformedData = Data()->CreateCorrectedAndFilteredDataset(exclude_samples, false,false);
        if (!CheckNegativeElements(&TransformedData))
            return false;

        if (arguments["Box-cox transformation"]=="true")
            TransformedData = TransformedData.BoxCoxTransformed(true);

        results.SetName("Multi-way element discriminant power between '" + arguments["Source/Target group I"] + "' and '" + arguments["Source/Target group II"] +"'");
        ResultItem EDPresultStd;
        EDPresultStd.SetName("Multi-way discreminant difference to standard deviation ratio");
        EDPresultStd.SetType(result_type::elemental_profile_set);
        EDPresultStd.SetShowAsString(true);
        EDPresultStd.SetShowTable(true);
        EDPresultStd.SetShowGraph(true);
        Elemental_Profile_Set *EDPProfileSet = new Elemental_Profile_Set(TransformedData.DifferentiationPower(false,include_target));
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
        Elemental_Profile_Set *EDPProfileSetPercent = new Elemental_Profile_Set(TransformedData.DifferentiationPower_Percentage(include_target));
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
        Elemental_Profile_Set *EDPProfileSetPValue = new Elemental_Profile_Set(TransformedData.DifferentiationPower_P_value(include_target));
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

        bool exclude_samples = (arguments["Use only selected samples"]=="true"?true:false);
        bool exclude_elements = (arguments["Use only selected elements"]=="true"?true:false);

        SourceSinkData TransformedData;
        if (arguments["OM and Size Correct based on target sample"] != "")
        {
            if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
            {
                QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }

            TransformedData = Data()->CreateCorrectedDataset(arguments["OM and Size Correct based on target sample"], true, Data()->GetElementInformation()).CreateCorrectedAndFilteredDataset(true, false,false);
        }
        else
            TransformedData = Data()->CreateCorrectedAndFilteredDataset(true, false,false);

        if (!CheckNegativeElements(&TransformedData))
                return false;

        if (arguments["Box-cox transformation"]=="true")
        {   TransformedData = TransformedData.BoxCoxTransformed(true);
            log=false;
        }

        results.SetName("ANOVA analysis");
        ResultItem Anovaresults;
        Anovaresults.SetName("ANOVA");
        Anovaresults.SetType(result_type::vector);
        Anovaresults.SetShowAsString(true);
        Anovaresults.SetShowTable(true);
        Anovaresults.SetShowGraph(true);
        CMBVector *PValues = new CMBVector(TransformedData.ANOVA(log));

        PValues->SetLimit(_range::high, aquiutils::atof(arguments["P-value threshold"]));
        PValues->SetLimit(_range::low, 0);
        Anovaresults.SetYAxisMode(yaxis_mode::normal);
        Anovaresults.SetResult(PValues);
        Anovaresults.setYAxisTitle("P-value");
        Anovaresults.setXAxisTitle("Element");

        if (arguments["Modify the included elements based on the results"] == "true")
        {
            vector<string> selected = PValues->ExtractWithinRange(0,aquiutils::atof(arguments["P-value threshold"])).Labels();
            Data()->IncludeExcludeElementsBasedOn(selected);
        }

        results.Append(Anovaresults);

    }
    if (command == "Error_Analysis")
    {

        results.SetName("Error Analysis");
        ProgressWindow* rtw = new ProgressWindow(mainwindow,0);

        bool organicnsizecorrection;
        if (arguments["Apply size and organic matter correction"]=="true")
        {   organicnsizecorrection = true;
            if (Data()->OMandSizeConstituents()[0]=="" && Data()->OMandSizeConstituents()[1]=="")
            {
                QMessageBox::warning(mainwindow, "SedSAT3","Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
        }
        else
            organicnsizecorrection = false;


        rtw->show();

        SourceSinkData correctedData = Data()->CreateCorrectedDataset(arguments["Sample"],organicnsizecorrection,Data()->GetElementInformation());
        correctedData.SetProgressWindow(rtw);
        if (!CheckNegativeElements(&correctedData))
            return false;

        bool softmax = false;
        if (arguments["Softmax transformation"] == "true")
            softmax = true;

        correctedData.InitializeParametersAndObservations(arguments["Sample"]);
        bool out = correctedData.BootStrap(&results, aquiutils::atof(arguments["Pecentage eliminated"]),aquiutils::atoi(arguments["Number of realizations"]),arguments["Sample"],softmax);

    }
    if (command == "Source_Verify")
    {

        results.SetName("Source Verification");
        ProgressWindow* rtw = new ProgressWindow(mainwindow,0);

        bool organicnsizecorrection;
        if (arguments["Apply size and organic matter correction"]=="true")
        {   organicnsizecorrection = true;
            if (Data()->OMandSizeConstituents()[0]=="" && Data()->OMandSizeConstituents()[1]=="")
            {
                QMessageBox::warning(mainwindow, "SedSAT3","Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
        }
        else
            organicnsizecorrection = false;

        rtw->show();

        SourceSinkData correctedData = *Data();
        correctedData.SetProgressWindow(rtw);
        if (!CheckNegativeElements(&correctedData))
            return false;

        bool softmax = false;
        if (arguments["Softmax transformation"] == "true")
            softmax = true;

        CMBTimeSeriesSet *contributions = new CMBTimeSeriesSet(correctedData.VerifySource(arguments["Source Group"],softmax,organicnsizecorrection));
        ResultItem contributions_result_item;
        results.SetName("Source verification for source'" + arguments["Source Group"] +"'");
        contributions->GetOptions().X_suffix = "";
        contributions->GetOptions().Y_suffix = "";
        contributions->SetOption(options_key::single_column_x, true);
        contributions_result_item.SetName("Source Verification");
        contributions_result_item.SetResult(contributions);
        contributions_result_item.SetType(result_type::stacked_bar_chart);
        contributions_result_item.SetShowAsString(true);
        contributions_result_item.SetShowTable(true);
        contributions_result_item.SetShowGraph(true);
        contributions_result_item.SetYLimit(_range::high, 1);
        contributions_result_item.SetXAxisMode(xaxis_mode::counter);
        contributions_result_item.setYAxisTitle("Contribution");
        contributions_result_item.setXAxisTitle("Source Sample");
        contributions_result_item.SetYLimit(_range::low, 0);
        results.Append(contributions_result_item);
        rtw->SetProgress(1);

    }
    if (command == "AutoSelect")
    {
        results.SetName("Auto-select elements");
        SourceSinkData TransformedData;
        bool OmandSizeCorrect = false;
        if (arguments["OM and Size Correct based on target sample"] != "")
        {
            if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
            {
                QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
                return false;
            }
            OmandSizeCorrect = true;
            TransformedData = Data()->CreateCorrectedDataset(arguments["OM and Size Correct based on target sample"], true, Data()->GetElementInformation()).CreateCorrectedAndFilteredDataset(true, false,false);
        }
        else
            TransformedData = Data()->CreateCorrectedAndFilteredDataset(true, false,false);
        if (!CheckNegativeElements(&TransformedData))
            return false;

        bool isotopes = false;
        if (arguments["Include Isotopes"] == "true")
            isotopes = true;

        TransformedData = TransformedData.ExtractChemicalElements(isotopes);
        ResultItem EDP_pValue;
        EDP_pValue.SetName("Multi-way discriminat p-value");
        EDP_pValue.SetType(result_type::elemental_profile_set);
        EDP_pValue.SetShowAsString(true);
        EDP_pValue.setYAxisTitle("p-Value");
        EDP_pValue.SetShowTable(true);
        EDP_pValue.SetShowGraph(true);
        Elemental_Profile_Set *EDPProfileSetPValue = new Elemental_Profile_Set(TransformedData.DifferentiationPower_P_value(false));
        EDP_pValue.SetYAxisMode(yaxis_mode::normal);
        EDP_pValue.SetYLimit(_range::high,1);
        EDP_pValue.SetResult(EDPProfileSetPValue);
        results.Append(EDP_pValue);

        Elemental_Profile *selected = new Elemental_Profile(EDPProfileSetPValue->SelectTopElementsAggregate(aquiutils::atoi(arguments["Number of elements from each pair"])));
        ResultItem SelectedElements;
        SelectedElements.SetResult(selected);
        SelectedElements.SetName("Selected Elements");
        SelectedElements.SetType(result_type::predicted_concentration);
        SelectedElements.SetShowAsString(true);
        SelectedElements.setYAxisTitle("p-Value");
        SelectedElements.SetShowTable(true);
        SelectedElements.SetShowGraph(true);
        results.Append(SelectedElements);

        if (arguments["Modify the included elements based on the results"] == "true")
        {
            Data()->IncludeExcludeElementsBasedOn(selected->GetElementNames());
        }

    }
    Data()->AddtoToolsUsed(command);
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
        QMessageBox::warning(mainwindow, "SedSAT3",message, QMessageBox::Ok);
        return false;
    }
    return true;

}

bool Conductor::CheckNegativeElements(map<string,vector<string>> negative_elements)
{
    QString message;
    for (map<string,vector<string>>::iterator it = negative_elements.begin(); it!=negative_elements.end(); it++)
    {
        if (it->second.size()>0)
        {   message += "For target sample '" + QString::fromStdString(it->first) + ":\n";
            for (unsigned int i=0; i<it->second.size(); i++)
            {
                message += QString::fromStdString("\t" + it->second[i]) + "\n";
            }
        }

    }

    if (message!="")
    {   QMessageBox::warning(mainwindow, "SedSAT3",message, QMessageBox::Ok);
        return false;
    }

}
