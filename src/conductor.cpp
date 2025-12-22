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
        ExecuteGA(arguments);
    }
    if (command == "GA (fixed elemental contribution)")
    {
		ExecuteGA_FixedProfile(arguments);
    }
    if (command == "GA (disregarding targets)")
    {
        ExecuteGA_NoTargets(arguments);
    }
    if (command == "Levenberg-Marquardt")
    {
        ExecuteLevenbergMarquardt(arguments); 
    }
    if (command == "Levenberg-Marquardt-Batch")
    {
		ExecuteLevenbergMarquardtBatch(arguments);

    }
    if (command == "OM-Size Correct")
    {
		ExecuteOMSizeCorrect(arguments);
    }
    if (command == "MLR")
    {
		ExecuteMLR(arguments);
    }
    if (command == "CovMat")
    {
		ExecuteCovarianceMatrix(arguments);

    }
    if (command == "CorMat")
    {
		ExecuteCorrelationMatrix(arguments);
    }
    if (command == "DFA")
    {
		ExecuteDFA(arguments);
    }

    if (command == "DFAOnevsRest")
    {
		ExecuteDFAOnevsRest(arguments);

    }
    if (command == "DFAM")
    {
		ExecuteDFAM(arguments);

    }
    if (command == "SDFA")
    {
		ExecuteSDFA(arguments);

    }

    if (command == "SDFAM")
    {
		ExecuteSDFAM(arguments);
    }

    if (command == "SDFAOnevsRest")
    {
		ExecuteSDFAOnevsRest(arguments);

    }

    if (command == "KS")
    {
		ExecuteKolmogorovSmirnov(arguments);
    }
    if (command == "KS-individual")
    {
		ExecuteKolmogorovSmirnovIndividual(arguments);
    }
    if (command == "CMB Bayesian")
    {
		ExecuteCMBBayesian(arguments);
    }
    if (command == "CMB Bayesian-Batch")
    {
		ExecuteCMBBayesianBatch(arguments);
    }
    if (command == "Test CMB Bayesian")
    {
		ExecuteTestCMBBayesian(arguments);


    }

    if (command == "DF")
    {
		ExecuteDistributionFitting(arguments);

    }

    if (command == "Bracketing Analysis")
    {
		ExecuteBracketingAnalysis(arguments);

    }
    if (command == "Bracketing Analysis Batch")
    {
		ExecuteBracketingAnalysisBatch(arguments);

    }
    if (command == "BoxCox")
    {
        return ExecuteBoxCox(arguments);
    }

    if (command == "Outlier")
    {
        return ExecuteOutlierAnalysis(arguments);
    }

    if (command == "EDP")
    {
        return ExecuteEDP(arguments);
    }

    if (command == "EDPM")
    {
        return ExecuteEDPM(arguments);
    }
    if (command == "ANOVA")
    {
        return ExecuteANOVA(arguments);
    }

    if (command == "Error_Analysis")
    {
        return ExecuteErrorAnalysis(arguments);
    }

    if (command == "Source_Verify")
    {
        return ExecuteSourceVerify(arguments);
    }

    if (command == "AutoSelect")
    {
        return ExecuteAutoSelect(arguments);
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

bool Conductor::ExecuteGA(const std::map<std::string, std::string>& arguments)
{
    ProgressWindow* rtw = new ProgressWindow(mainwindow);

    bool organic_size_correction;
    if (arguments.at("Apply size and organic matter correction") == "true")
    {
        organic_size_correction = true;
        if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
        {
            QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
            return false;
        }
    }
    else
    {
        organic_size_correction = false;
    }

    SourceSinkData corrected_data = Data()->CreateCorrectedDataset(
        arguments.at("Sample"),
        organic_size_correction,
        Data()->GetElementInformation()
    );

    rtw->show();
    corrected_data.InitializeParametersAndObservations(arguments.at("Sample"));

    if (!CheckNegativeElements(&corrected_data))
        return false;

    GA = std::make_unique<CGA<SourceSinkData>>(&corrected_data);
    GA->filenames.pathname = workingfolder.toStdString() + "/";
    GA->SetRunTimeWindow(rtw);
    GA->SetProperties(arguments);
    GA->InitiatePopulation();
    GA->optimize();

    results.SetName("GA " + arguments.at("Sample"));

    ResultItem result_contribution = GA->Model_out.GetContribution();
    result_contribution.SetShowTable(true);
    result_contribution.SetType(result_type::contribution);
    result_contribution.SetShowGraph(true);
    results.Append(result_contribution);

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
    result_estimated_means.SetShowGraph(false);
    results.Append(result_estimated_means);

    ResultItem result_calculated_mu = GA->Model_out.GetCalculatedElementMu();
    result_calculated_mu.SetShowTable(true);
    result_calculated_mu.SetShowGraph(false);
    results.Append(result_calculated_mu);

    ResultItem result_estimated_mu = GA->Model_out.GetEstimatedElementMu();
    result_estimated_mu.SetShowTable(true);
    result_estimated_mu.SetShowGraph(false);
    results.Append(result_estimated_mu);

    ResultItem result_calculated_sigma = GA->Model_out.GetCalculatedElementSigma();
    result_calculated_sigma.SetShowTable(true);
    result_calculated_sigma.SetShowGraph(false);
    results.Append(result_calculated_sigma);

    ResultItem result_estimated_sigma = GA->Model_out.GetEstimatedElementSigma();
    result_estimated_sigma.SetShowTable(true);
    result_estimated_sigma.SetShowGraph(false);
    results.Append(result_estimated_sigma);

    return true;
}


bool Conductor::ExecuteGA_FixedProfile(const std::map<std::string, std::string>& arguments)
{
    ProgressWindow* rtw = new ProgressWindow(mainwindow);
    rtw->SetTitle("Fitness", 0);
    rtw->SetYAxisTitle("Fitness", 0);

    bool organic_size_correction;
    if (arguments.at("Apply size and organic matter correction") == "true")
    {
        organic_size_correction = true;
        if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
        {
            QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
            return false;
        }
    }
    else
    {
        organic_size_correction = false;
    }

    SourceSinkData corrected_data = Data()->CreateCorrectedDataset(
        arguments.at("Sample"),
        organic_size_correction,
        Data()->GetElementInformation()
    );

    if (!CheckNegativeElements(&corrected_data))
        return false;

    rtw->show();
    corrected_data.InitializeParametersAndObservations(
        arguments.at("Sample"),
        estimation_mode::only_contributions
    );
    corrected_data.SetParameterEstimationMode(estimation_mode::only_contributions);

    GA = std::make_unique<CGA<SourceSinkData>>(&corrected_data);
    GA->filenames.pathname = workingfolder.toStdString() + "/";
    GA->SetRunTimeWindow(rtw);
    GA->SetProperties(arguments);
    GA->InitiatePopulation();
    GA->optimize();

    results.SetName("GA (fixed profile) " + arguments.at("Sample"));

    ResultItem result_contribution = GA->Model_out.GetContribution();
    results.Append(result_contribution);

    ResultItem result_modeled_vs_measured = GA->Model_out.GetObservedvsModeledElementalProfile(parameter_mode::direct);
    results.Append(result_modeled_vs_measured);

    ResultItem result_modeled_vs_measured_isotope = GA->Model_out.GetObservedvsModeledElementalProfile_Isotope(parameter_mode::direct);
    results.Append(result_modeled_vs_measured_isotope);

    return true;
}

bool Conductor::ExecuteGA_NoTargets(const std::map<std::string, std::string>& arguments)
{
    ProgressWindow* rtw = new ProgressWindow(mainwindow);
    rtw->show();

    Data()->InitializeParametersAndObservations(
        arguments.at("Sample"),
        estimation_mode::source_elemental_profiles_based_on_source_data
    );
    Data()->SetParameterEstimationMode(estimation_mode::source_elemental_profiles_based_on_source_data);

    GA = std::make_unique<CGA<SourceSinkData>>(Data());
    GA->filenames.pathname = workingfolder.toStdString() + "/";
    GA->SetRunTimeWindow(rtw);
    GA->SetProperties(arguments);
    GA->InitiatePopulation();
    GA->optimize();

    results.SetName("GA (no targets) " + arguments.at("Sample"));

    ResultItem result_calculated_means = GA->Model_out.GetCalculatedElementMeans();
    results.Append(result_calculated_means);

    ResultItem result_estimated_means = GA->Model_out.GetEstimatedElementMean();
    results.Append(result_estimated_means);

    ResultItem result_calculated_stds = GA->Model_out.GetCalculatedElementSigma();
    results.Append(result_calculated_stds);

    ResultItem result_estimated_stds = GA->Model_out.GetEstimatedElementSigma();
    results.Append(result_estimated_stds);

    return true;
}

bool Conductor::ExecuteLevenbergMarquardt(const std::map<std::string, std::string>& arguments)
{
    ProgressWindow* rtw = new ProgressWindow(mainwindow);

    bool organic_size_correction;
    if (arguments.at("Apply size and organic matter correction") == "true")
    {
        organic_size_correction = true;
        if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
        {
            QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
            return false;
        }
    }
    else
    {
        organic_size_correction = false;
    }

    rtw->show();

    SourceSinkData corrected_data = Data()->CreateCorrectedDataset(
        arguments.at("Sample"),
        organic_size_correction,
        Data()->GetElementInformation()
    );

    if (!CheckNegativeElements(&corrected_data))
        return false;

    corrected_data.InitializeParametersAndObservations(arguments.at("Sample"));
    corrected_data.SetProgressWindow(rtw);

    if (arguments.at("Softmax transformation") == "true")
    {
        corrected_data.SolveLevenberg_Marquardt(transformation::softmax);
    }
    else
    {
        corrected_data.SolveLevenberg_Marquardt(transformation::linear);
    }

    results.SetName("LM " + arguments.at("Sample"));

    ResultItem result_contribution = corrected_data.GetContribution();
    result_contribution.SetShowTable(true);
    results.Append(result_contribution);

    ResultItem result_modeled = corrected_data.GetPredictedElementalProfile(parameter_mode::direct);
    result_modeled.SetShowTable(true);
    results.Append(result_modeled);

    ResultItem result_modeled_vs_measured = corrected_data.GetObservedvsModeledElementalProfile();
    result_modeled_vs_measured.SetShowTable(true);
    results.Append(result_modeled_vs_measured);

    ResultItem result_modeled_vs_measured_isotope = corrected_data.GetObservedvsModeledElementalProfile_Isotope(parameter_mode::direct);
    result_modeled_vs_measured_isotope.SetShowTable(true);
    results.Append(result_modeled_vs_measured_isotope);

    return true;
}

bool Conductor::ExecuteLevenbergMarquardtBatch(const std::map<std::string, std::string>& arguments)
{
    ProgressWindow* rtw = new ProgressWindow(mainwindow, 0);

    bool organic_size_correction;
    if (arguments.at("Apply size and organic matter correction") == "true")
    {
        organic_size_correction = true;
        if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
        {
            QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
            return false;
        }
    }
    else
    {
        organic_size_correction = false;
    }

    rtw->show();

    Data()->SetProgressWindow(rtw);

    CMBTimeSeriesSet* contributions;
    std::map<std::string, std::vector<std::string>> negative_elements;

    if (arguments.at("Softmax transformation") == "true")
    {
        contributions = new CMBTimeSeriesSet(Data()->LM_Batch(
            transformation::softmax,
            organic_size_correction,
            negative_elements
        ));
    }
    else
    {
        contributions = new CMBTimeSeriesSet(Data()->LM_Batch(
            transformation::linear,
            organic_size_correction,
            negative_elements
        ));
    }

    if (negative_elements.size() > 0)
    {
        CheckNegativeElements(negative_elements);
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

    return true;
}

bool Conductor::ExecuteOMSizeCorrect(const std::map<std::string, std::string>& arguments)
{
    bool exclude_samples = (arguments.at("Use only selected samples") == "true");
    bool exclude_elements = (arguments.at("Use only selected elements") == "true");

    Data()->SetSelectedTargetSample(arguments.at("Sample"));

    SourceSinkData transformed_data = Data()->CreateCorrectedAndFilteredDataset(
        exclude_samples,
        exclude_elements,
        true,
        arguments.at("Sample")
    );

    std::vector<ResultItem> result_items = transformed_data.GetSourceProfiles();

    results.SetName("Corrected Elemental Profiles for Target" + arguments.at("Sample"));

    for (size_t i = 0; i < result_items.size(); i++)
    {
        results.Append(result_items[i]);
    }

    return true;
}

bool Conductor::ExecuteMLR(const std::map<std::string, std::string>& arguments)
{
    if (arguments.at("Organic Matter constituent") == "" &&
        arguments.at("Particle Size constituent") == "")
    {
        QMessageBox::information(
            mainwindow,
            "Exclude Elements",
            "At least one of Organic Matter constituent and Particle Size constituent must be selected",
            QMessageBox::Ok
        );
        return false;
    }

    results.SetName("MLR_vs_OM&Size ");

    bool exclude_samples = (arguments.at("Use only selected samples") == "true");

    SourceSinkData transformed_data = Data()->CreateCorrectedAndFilteredDataset(
        exclude_samples,
        false,
        false
    );

    double p_value_threshold = QString::fromStdString(arguments.at("P-value threshold")).toDouble();

    if (arguments.at("Equation") == "Linear")
    {
        transformed_data.PerformRegressionVsOMAndSize(
            arguments.at("Organic Matter constituent"),
            arguments.at("Particle Size constituent"),
            regression_form::linear,
            p_value_threshold
        );
    }
    else
    {
        transformed_data.PerformRegressionVsOMAndSize(
            arguments.at("Organic Matter constituent"),
            arguments.at("Particle Size constituent"),
            regression_form::power,
            p_value_threshold
        );
    }

    Data()->SetOMandSizeConstituents(
        arguments.at("Organic Matter constituent"),
        arguments.at("Particle Size constituent")
    );

    for (std::map<std::string, Elemental_Profile_Set>::iterator it = Data()->begin();
        it != Data()->end();
        ++it)
    {
        if (it->first != Data()->GetTargetGroup())
        {
            it->second.SetRegressionModels(transformed_data[it->first].GetRegressionModels());
        }
    }

    std::vector<ResultItem> regression_results = transformed_data.GetMLRResults();

    for (size_t i = 0; i < regression_results.size(); i++)
    {
        results.Append(regression_results[i]);
    }

    return true;
}

bool Conductor::ExecuteCovarianceMatrix(const std::map<std::string, std::string>& arguments)
{
    results.SetName("Covariance Matrix for " + arguments.at("Source/Target group"));

    ResultItem covariance_matrix_item;
    covariance_matrix_item.SetName("Covariance Matrix for " + arguments.at("Source/Target group"));
    covariance_matrix_item.SetShowTable(true);
    covariance_matrix_item.SetType(result_type::matrix);
    covariance_matrix_item.SetShowGraph(false);

    CMBMatrix* covariance_matrix = new CMBMatrix(
        Data()->at(arguments.at("Source/Target group")).CalculateCovarianceMatrix()
    );

    covariance_matrix_item.SetResult(covariance_matrix);
    results.Append(covariance_matrix_item);

    return true;
}


bool Conductor::ExecuteCorrelationMatrix(const std::map<std::string, std::string>& arguments)
{
    results.SetName("Correlation Matrix for " + arguments.at("Source/Target group"));

    ResultItem correlation_matrix_item;
    correlation_matrix_item.SetName("Correlation Matrix");
    correlation_matrix_item.SetShowTable(true);
    correlation_matrix_item.setTableTitle("Correlation Matrix for source group '" +
        arguments.at("Source/Target group") + "'");
    correlation_matrix_item.SetType(result_type::matrix);
    correlation_matrix_item.SetShowGraph(false);

    double threshold = QString::fromStdString(arguments.at("Threshold")).toDouble();
    bool exclude_samples = (arguments.at("Use only selected samples") == "true");
    bool exclude_elements = (arguments.at("Use only selected elements") == "true");

    SourceSinkData transformed_data = Data()->CreateCorrectedAndFilteredDataset(
        exclude_samples,
        exclude_elements,
        false
    );

    if (arguments.at("OM and Size Correct based on target sample") != "")
    {
        if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
        {
            QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
            return false;
        }
        transformed_data = transformed_data.CreateCorrectedDataset(
            arguments.at("OM and Size Correct based on target sample"),
            true,
            Data()->GetElementInformation()
        );
    }

    CMBMatrix* correlation_matrix = new CMBMatrix(
        transformed_data.at(arguments.at("Source/Target group")).CalculateCorrelationMatrix()
    );

    correlation_matrix->SetLimit(_range::high, threshold);
    correlation_matrix->SetLimit(_range::low, -threshold);

    correlation_matrix_item.SetResult(correlation_matrix);
    results.Append(correlation_matrix_item);

    return true;
}

bool Conductor::ExecuteDFA(const std::map<std::string, std::string>& arguments)
{
    if (arguments.at("Source/Target group I") == arguments.at("Source/Target group II"))
    {
        QMessageBox::warning(mainwindow, "SedSAT3", "The selected sources must be different", QMessageBox::Ok);
        return false;
    }

    ProgressWindow* rtw = new ProgressWindow(mainwindow, 0);
    rtw->show();

    results.SetName("DFA between " + arguments.at("Source/Target group I") +
        "&" + arguments.at("Source/Target group II"));

    bool exclude_samples = (arguments.at("Use only selected samples") == "true");
    bool exclude_elements = (arguments.at("Use only selected elements") == "true");

    SourceSinkData transformed_data = *Data();

    if (arguments.at("OM and Size Correct based on target sample") != "")
    {
        if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
        {
            QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
            return false;
        }
        transformed_data = transformed_data.CreateCorrectedDataset(
            arguments.at("OM and Size Correct based on target sample"),
            true,
            Data()->GetElementInformation()
        );
    }

    transformed_data = transformed_data.CreateCorrectedAndFilteredDataset(
        exclude_samples,
        exclude_elements,
        false
    );

    transformed_data.SetProgressWindow(rtw);

    if (arguments.at("Box-cox transformation") == "true")
    {
        transformed_data = transformed_data.BoxCoxTransformed(true);
    }

    DFA_result dfa_result = transformed_data.DiscriminantFunctionAnalysis(
        arguments.at("Source/Target group I"),
        arguments.at("Source/Target group II")
    );

    if (dfa_result.eigen_vectors.size() == 0)
    {
        QMessageBox::warning(mainwindow, "SedSAT3", "Singular matrix in within group scatter matrix!\n", QMessageBox::Ok);
        return false;
    }

    // Chi-squared P-Value result
    CMBVector* p_value = new CMBVector(dfa_result.p_values);
    ResultItem dfa_p_value;
    dfa_p_value.SetName("Chi-squared P-Value");
    dfa_p_value.SetType(result_type::vector);
    dfa_p_value.SetShowTable(true);
    dfa_p_value.SetShowGraph(false);
    dfa_p_value.SetResult(p_value);
    results.Append(dfa_p_value);

    // F-test P-Value result
    CMBVector* f_test_p_value = new CMBVector(dfa_result.F_test_P_value);
    ResultItem dfa_f_test_p_value;
    dfa_f_test_p_value.SetName("F-test P-Value");
    dfa_f_test_p_value.SetType(result_type::vector);
    dfa_f_test_p_value.SetShowTable(true);
    dfa_f_test_p_value.SetShowGraph(false);
    dfa_f_test_p_value.SetResult(f_test_p_value);
    results.Append(dfa_f_test_p_value);

    // Projected Elemental Profiles result
    ResultItem dfa_projected;
    dfa_projected.SetName("Projected Elemental Profiles");
    dfa_projected.SetType(result_type::vectorset_groups);
    dfa_projected.SetShowTable(true);
    dfa_projected.SetShowGraph(true);
    dfa_projected.SetYAxisMode(yaxis_mode::normal);
    CMBVectorSet* projected = new CMBVectorSet(dfa_result.projected);
    dfa_projected.SetResult(projected);
    results.Append(dfa_projected);

    // Eigen vector result
    ResultItem dfa_eigen_vector;
    dfa_eigen_vector.SetName("Eigen vector");
    dfa_eigen_vector.SetType(result_type::vector);
    dfa_eigen_vector.SetShowTable(true);
    dfa_eigen_vector.SetShowGraph(true);
    dfa_eigen_vector.SetYAxisMode(yaxis_mode::normal);
    CMBVector* eigen_vector = new CMBVector(dfa_result.eigen_vectors.begin()->second);
    dfa_eigen_vector.SetResult(eigen_vector);
    results.Append(dfa_eigen_vector);

    rtw->SetProgress(1);

    return true;
}

bool Conductor::ExecuteDFAOnevsRest(const std::map<std::string, std::string>& arguments)
{
    ProgressWindow* rtw = new ProgressWindow(mainwindow, 0);
    rtw->show();

    results.SetName("DFA between " + arguments.at("Source group") + "& the rest");

    bool exclude_samples = (arguments.at("Use only selected samples") == "true");
    bool exclude_elements = (arguments.at("Use only selected elements") == "true");

    SourceSinkData transformed_data = *Data();

    if (arguments.at("OM and Size Correct based on target sample") != "")
    {
        if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
        {
            QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
            return false;
        }
        transformed_data = transformed_data.CreateCorrectedDataset(
            arguments.at("OM and Size Correct based on target sample"),
            true,
            Data()->GetElementInformation()
        );
    }

    transformed_data = transformed_data.CreateCorrectedAndFilteredDataset(
        exclude_samples,
        exclude_elements,
        false
    );

    transformed_data.SetProgressWindow(rtw);

    if (arguments.at("Box-cox transformation") == "true")
    {
        transformed_data = transformed_data.BoxCoxTransformed(true);
    }

    DFA_result dfa_result = transformed_data.DiscriminantFunctionAnalysis(
        arguments.at("Source group")
    );

    if (dfa_result.eigen_vectors.size() == 0)
    {
        QMessageBox::warning(mainwindow, "SedSAT3", "Singular matrix in within group scatter matrix!\n", QMessageBox::Ok);
        return false;
    }

    // Chi-squared P-Value result
    CMBVector* p_value = new CMBVector(dfa_result.p_values);
    ResultItem dfa_p_value;
    dfa_p_value.SetName("Chi-squared P-Value");
    dfa_p_value.SetType(result_type::vector);
    dfa_p_value.SetShowTable(true);
    dfa_p_value.SetShowGraph(false);
    dfa_p_value.SetResult(p_value);
    results.Append(dfa_p_value);

    // F-test P-Value result
    CMBVector* f_test_p_value = new CMBVector(dfa_result.F_test_P_value);
    ResultItem dfa_f_test_p_value;
    dfa_f_test_p_value.SetName("F-test P-Value");
    dfa_f_test_p_value.SetType(result_type::vector);
    dfa_f_test_p_value.SetShowTable(true);
    dfa_f_test_p_value.SetShowGraph(false);
    dfa_f_test_p_value.SetResult(f_test_p_value);
    results.Append(dfa_f_test_p_value);

    // Projected Elemental Profiles result
    ResultItem dfa_projected;
    dfa_projected.SetName("Projected Elemental Profiles");
    dfa_projected.SetType(result_type::vectorset_groups);
    dfa_projected.SetShowTable(true);
    dfa_projected.SetShowGraph(true);
    dfa_projected.SetYAxisMode(yaxis_mode::normal);
    CMBVectorSet* projected = new CMBVectorSet(dfa_result.projected);
    dfa_projected.SetResult(projected);
    results.Append(dfa_projected);

    // Eigen vector result
    ResultItem dfa_eigen_vector;
    dfa_eigen_vector.SetName("Eigen vector");
    dfa_eigen_vector.SetType(result_type::vector);
    dfa_eigen_vector.SetShowTable(true);
    dfa_eigen_vector.SetShowGraph(true);
    dfa_eigen_vector.SetYAxisMode(yaxis_mode::normal);
    CMBVector* eigen_vector = new CMBVector(dfa_result.eigen_vectors.begin()->second);
    dfa_eigen_vector.SetResult(eigen_vector);
    results.Append(dfa_eigen_vector);

    rtw->SetProgress(1);

    return true;
}

bool Conductor::ExecuteDFAM(const std::map<std::string, std::string>& arguments)
{
    ProgressWindow* rtw = new ProgressWindow(mainwindow, 0);
    rtw->show();

    results.SetName("Multi-way DFA analysis");

    bool exclude_samples = (arguments.at("Use only selected samples") == "true");
    bool exclude_elements = (arguments.at("Use only selected elements") == "true");

    SourceSinkData transformed_data = *Data();

    if (arguments.at("OM and Size Correct based on target sample") != "")
    {
        if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
        {
            QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
            return false;
        }
        transformed_data = transformed_data.CreateCorrectedDataset(
            arguments.at("OM and Size Correct based on target sample"),
            true,
            Data()->GetElementInformation()
        );
    }

    transformed_data = transformed_data.CreateCorrectedAndFilteredDataset(
        exclude_samples,
        exclude_elements,
        false
    );

    if (!CheckNegativeElements(&transformed_data))
        return false;

    transformed_data.SetProgressWindow(rtw);

    if (arguments.at("Box-cox transformation") == "true")
    {
        transformed_data = transformed_data.BoxCoxTransformed(true);
    }

    DFA_result dfa_result = transformed_data.DiscriminantFunctionAnalysis();

    if (dfa_result.eigen_vectors.size() == 0)
    {
        QMessageBox::warning(mainwindow, "SedSAT3", "Singular matrix in within group scatter matrix!\n", QMessageBox::Ok);
        return false;
    }

    // Chi-squared P-Value result
    CMBVector* p_value = new CMBVector(dfa_result.p_values);
    ResultItem dfa_p_value;
    dfa_p_value.SetName("Chi-squared P-Value");
    dfa_p_value.SetType(result_type::vector);
    dfa_p_value.SetShowTable(true);
    dfa_p_value.SetShowGraph(false);
    dfa_p_value.SetResult(p_value);
    results.Append(dfa_p_value);

    // F-test P-Value result
    CMBVector* f_test_p_value = new CMBVector(dfa_result.F_test_P_value);
    ResultItem dfa_f_test_p_value;
    dfa_f_test_p_value.SetName("F-test P-Value");
    dfa_f_test_p_value.SetType(result_type::vector);
    dfa_f_test_p_value.SetShowTable(true);
    dfa_f_test_p_value.SetShowGraph(false);
    dfa_f_test_p_value.SetResult(f_test_p_value);
    results.Append(dfa_f_test_p_value);

    // Multiway Projected Elemental Profiles result
    ResultItem dfa_projected;
    dfa_projected.SetName("Multiway Projected Elemental Profiles");
    dfa_projected.SetType(result_type::dfa_vectorsetset);
    dfa_projected.SetShowTable(true);
    dfa_projected.SetShowGraph(true);
    dfa_projected.SetYAxisMode(yaxis_mode::normal);
    CMBVectorSetSet* projected = new CMBVectorSetSet(dfa_result.multi_projected);
    dfa_projected.SetResult(projected);
    results.Append(dfa_projected);

    // Eigen vectors result (multiple vectors for multiway)
    ResultItem dfa_eigen_vectors;
    dfa_eigen_vectors.SetName("Eigen vector");
    dfa_eigen_vectors.SetType(result_type::vectorset);
    dfa_eigen_vectors.SetShowTable(true);
    dfa_eigen_vectors.SetShowGraph(true);
    dfa_eigen_vectors.SetYAxisMode(yaxis_mode::normal);
    CMBVectorSet* eigen_vectors = new CMBVectorSet(dfa_result.eigen_vectors);
    dfa_eigen_vectors.SetResult(eigen_vectors);
    results.Append(dfa_eigen_vectors);

    rtw->SetProgress(1);

    return true;
}

bool Conductor::ExecuteSDFA(const std::map<std::string, std::string>& arguments)
{
    if (arguments.at("Source/Target group I") == arguments.at("Source/Target group II"))
    {
        QMessageBox::warning(mainwindow, "SedSAT3", "The selected sources must be different", QMessageBox::Ok);
        return false;
    }

    ProgressWindow* rtw = new ProgressWindow(mainwindow, 0);
    rtw->show();

    results.SetName("Stepwise DFA between " + arguments.at("Source/Target group I") +
        "&" + arguments.at("Source/Target group II"));

    bool exclude_samples = (arguments.at("Use only selected samples") == "true");
    bool exclude_elements = (arguments.at("Use only selected elements") == "true");

    SourceSinkData transformed_data;

    if (arguments.at("OM and Size Correct based on target sample") != "")
    {
        if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
        {
            QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
            return false;
        }
        transformed_data = Data()->CreateCorrectedDataset(
            arguments.at("OM and Size Correct based on target sample"),
            true,
            Data()->GetElementInformation()
        ).CreateCorrectedAndFilteredDataset(exclude_samples, exclude_elements, false);
    }
    else
    {
        transformed_data = Data()->CreateCorrectedAndFilteredDataset(
            exclude_samples,
            exclude_elements,
            false
        );
    }

    if (!CheckNegativeElements(&transformed_data))
        return false;

    if (arguments.at("Box-cox transformation") == "true")
    {
        transformed_data = transformed_data.BoxCoxTransformed(true);
    }

    transformed_data.SetProgressWindow(rtw);

    std::vector<CMBVector> sdfa_results = transformed_data.StepwiseDiscriminantFunctionAnalysis(
        arguments.at("Source/Target group I"),
        arguments.at("Source/Target group II")
    );

    if (sdfa_results[0].size() == 0)
    {
        QMessageBox::warning(mainwindow, "SedSAT3", "Singular matrix in within group scatter matrix!\n", QMessageBox::Ok);
        return false;
    }

    // Chi-squared P-Values result
    CMBVector* p_vector = new CMBVector(sdfa_results[0]);
    ResultItem sdfa_p_values;
    sdfa_p_values.SetName("Chi-squared P-Values");
    sdfa_p_values.SetType(result_type::vector);
    sdfa_p_values.SetShowTable(true);
    sdfa_p_values.SetAbsoluteValue(true);
    sdfa_p_values.SetYAxisMode(yaxis_mode::log);
    sdfa_p_values.SetYLimit(_range::high, 1);
    sdfa_p_values.SetResult(p_vector);
    results.Append(sdfa_p_values);

    // Wilks' Lambda result
    CMBVector* wilks_lambda_vector = new CMBVector(sdfa_results[1]);
    ResultItem sdfa_wilks_lambda;
    sdfa_wilks_lambda.SetName("Wilks' Lambda");
    sdfa_wilks_lambda.SetType(result_type::vector);
    sdfa_wilks_lambda.SetShowTable(true);
    sdfa_wilks_lambda.SetAbsoluteValue(true);
    sdfa_wilks_lambda.SetYAxisMode(yaxis_mode::log);
    sdfa_wilks_lambda.SetYLimit(_range::high, 1);
    sdfa_wilks_lambda.SetResult(wilks_lambda_vector);
    results.Append(sdfa_wilks_lambda);

    // F-test P-Value result
    CMBVector* f_test_p_value = new CMBVector(sdfa_results[2]);
    ResultItem sdfa_f_test_p_value;
    sdfa_f_test_p_value.SetName("F-test P-Value");
    sdfa_f_test_p_value.SetType(result_type::vector);
    sdfa_f_test_p_value.SetShowTable(true);
    sdfa_f_test_p_value.SetAbsoluteValue(true);
    sdfa_f_test_p_value.SetYAxisMode(yaxis_mode::log);
    sdfa_f_test_p_value.SetYLimit(_range::high, 1);
    sdfa_f_test_p_value.SetResult(f_test_p_value);
    results.Append(sdfa_f_test_p_value);

    return true;
}

bool Conductor::ExecuteSDFAM(const std::map<std::string, std::string>& arguments)
{
    ProgressWindow* rtw = new ProgressWindow(mainwindow, 0);
    rtw->show();

    results.SetName("Multiway Stepwise DFA");

    bool exclude_samples = (arguments.at("Use only selected samples") == "true");
    bool exclude_elements = (arguments.at("Use only selected elements") == "true");

    SourceSinkData transformed_data;

    if (arguments.at("OM and Size Correct based on target sample") != "")
    {
        if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
        {
            QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
            return false;
        }
        transformed_data = Data()->CreateCorrectedDataset(
            arguments.at("OM and Size Correct based on target sample"),
            true,
            Data()->GetElementInformation()
        ).CreateCorrectedAndFilteredDataset(exclude_samples, exclude_elements, false);
    }
    else
    {
        transformed_data = Data()->CreateCorrectedAndFilteredDataset(
            exclude_samples,
            exclude_elements,
            false
        );
    }

    if (!CheckNegativeElements(&transformed_data))
        return false;

    if (arguments.at("Box-cox transformation") == "true")
    {
        transformed_data = transformed_data.BoxCoxTransformed(true);
    }

    transformed_data.SetProgressWindow(rtw);

    std::vector<CMBVector> sdfa_results = transformed_data.StepwiseDiscriminantFunctionAnalysis();

    if (sdfa_results[0].size() == 0)
    {
        QMessageBox::warning(mainwindow, "SedSAT3", "Singular matrix in within group scatter matrix!\n", QMessageBox::Ok);
        return false;
    }

    // Chi-squared P-Values result
    CMBVector* p_vector = new CMBVector(sdfa_results[0]);
    ResultItem sdfa_p_values;
    sdfa_p_values.SetName("Chi-squared P-Values");
    sdfa_p_values.SetType(result_type::vector);
    sdfa_p_values.SetShowTable(true);
    sdfa_p_values.SetAbsoluteValue(true);
    sdfa_p_values.SetYAxisMode(yaxis_mode::log);
    sdfa_p_values.SetYLimit(_range::high, 1);
    sdfa_p_values.SetResult(p_vector);
    results.Append(sdfa_p_values);

    // Selected elements result
    ResultItem sdfa_selected;
    sdfa_selected.SetName("Elements to be Selected");
    sdfa_selected.SetType(result_type::vector);
    sdfa_selected.SetShowTable(true);
    sdfa_selected.SetAbsoluteValue(true);
    sdfa_selected.SetYAxisMode(yaxis_mode::log);
    sdfa_selected.SetYLimit(_range::high, 1);

    CMBVector* p_vector_selected = new CMBVector();
    std::vector<std::string> selected = p_vector->ExtractUpToMinimum().Labels();

    if (arguments.at("Modify the included elements based on the results") == "true")
    {
        Data()->IncludeExcludeElementsBasedOn(selected);
    }

    for (size_t i = 0; i < selected.size(); i++)
    {
        p_vector_selected->append(selected[i], p_vector->valueAt(i));
    }

    sdfa_selected.SetResult(p_vector_selected);
    results.Append(sdfa_selected);

    // Wilks' Lambda result
    CMBVector* wilks_lambda_vector = new CMBVector(sdfa_results[1]);
    ResultItem sdfa_wilks_lambda;
    sdfa_wilks_lambda.SetName("Wilks' Lambda");
    sdfa_wilks_lambda.SetType(result_type::vector);
    sdfa_wilks_lambda.SetShowTable(true);
    sdfa_wilks_lambda.SetAbsoluteValue(true);
    sdfa_wilks_lambda.SetYAxisMode(yaxis_mode::log);
    sdfa_wilks_lambda.SetYLimit(_range::high, 1);
    sdfa_wilks_lambda.SetResult(wilks_lambda_vector);
    results.Append(sdfa_wilks_lambda);

    // F-test P-Value result
    CMBVector* f_test_p_value = new CMBVector(sdfa_results[2]);
    ResultItem sdfa_f_test_p_value;
    sdfa_f_test_p_value.SetName("F-test P-Value");
    sdfa_f_test_p_value.SetType(result_type::vector);
    sdfa_f_test_p_value.SetShowTable(true);
    sdfa_f_test_p_value.SetAbsoluteValue(true);
    sdfa_f_test_p_value.SetYAxisMode(yaxis_mode::log);
    sdfa_f_test_p_value.SetYLimit(_range::high, 1);
    sdfa_f_test_p_value.SetResult(f_test_p_value);
    results.Append(sdfa_f_test_p_value);

    return true;
}

bool Conductor::ExecuteSDFAOnevsRest(const std::map<std::string, std::string>& arguments)
{
    ProgressWindow* rtw = new ProgressWindow(mainwindow, 0);
    rtw->show();

    results.SetName("Stepwise DFA between " + arguments.at("Source group") + "& the rest");

    bool exclude_samples = (arguments.at("Use only selected samples") == "true");
    bool exclude_elements = (arguments.at("Use only selected elements") == "true");

    SourceSinkData transformed_data;

    if (arguments.at("OM and Size Correct based on target sample") != "")
    {
        if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
        {
            QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
            return false;
        }
        transformed_data = Data()->CreateCorrectedDataset(
            arguments.at("OM and Size Correct based on target sample"),
            true,
            Data()->GetElementInformation()
        ).CreateCorrectedAndFilteredDataset(exclude_samples, exclude_elements, false);
    }
    else
    {
        transformed_data = Data()->CreateCorrectedAndFilteredDataset(
            exclude_samples,
            exclude_elements,
            false
        );
    }

    if (!CheckNegativeElements(&transformed_data))
        return false;

    if (arguments.at("Box-cox transformation") == "true")
    {
        transformed_data = transformed_data.BoxCoxTransformed(true);
    }

    transformed_data.SetProgressWindow(rtw);

    std::vector<CMBVector> sdfa_results = transformed_data.StepwiseDiscriminantFunctionAnalysis(
        arguments.at("Source group")
    );

    if (sdfa_results[0].size() == 0)
    {
        QMessageBox::warning(mainwindow, "SedSAT3", "Singular matrix in within group scatter matrix!\n", QMessageBox::Ok);
        return false;
    }

    // Chi-squared P-Values result
    CMBVector* p_vector = new CMBVector(sdfa_results[0]);
    ResultItem sdfa_p_values;
    sdfa_p_values.SetName("Chi-squared P-Values");
    sdfa_p_values.SetType(result_type::vector);
    sdfa_p_values.SetShowTable(true);
    sdfa_p_values.SetAbsoluteValue(true);
    sdfa_p_values.SetYAxisMode(yaxis_mode::log);
    sdfa_p_values.SetYLimit(_range::high, 1);
    sdfa_p_values.SetResult(p_vector);
    results.Append(sdfa_p_values);

    // Wilks' Lambda result
    CMBVector* wilks_lambda_vector = new CMBVector(sdfa_results[1]);
    ResultItem sdfa_wilks_lambda;
    sdfa_wilks_lambda.SetName("Wilks' Lambda");
    sdfa_wilks_lambda.SetType(result_type::vector);
    sdfa_wilks_lambda.SetShowTable(true);
    sdfa_wilks_lambda.SetAbsoluteValue(true);
    sdfa_wilks_lambda.SetYAxisMode(yaxis_mode::log);
    sdfa_wilks_lambda.SetYLimit(_range::high, 1);
    sdfa_wilks_lambda.SetResult(wilks_lambda_vector);
    results.Append(sdfa_wilks_lambda);

    // F-test P-Value result
    CMBVector* f_test_p_value = new CMBVector(sdfa_results[2]);
    ResultItem sdfa_f_test_p_value;
    sdfa_f_test_p_value.SetName("F-test P-Value");
    sdfa_f_test_p_value.SetType(result_type::vector);
    sdfa_f_test_p_value.SetShowTable(true);
    sdfa_f_test_p_value.SetAbsoluteValue(true);
    sdfa_f_test_p_value.SetYAxisMode(yaxis_mode::log);
    sdfa_f_test_p_value.SetYLimit(_range::high, 1);
    sdfa_f_test_p_value.SetResult(f_test_p_value);
    results.Append(sdfa_f_test_p_value);

    return true;
}

bool Conductor::ExecuteKolmogorovSmirnov(const std::map<std::string, std::string>& arguments)
{
    results.SetName("Kolmogorov–Smirnov statististics for " + arguments.at("Source/Target group"));

    ResultItem ks_item;
    ks_item.SetName("Kolmogorov–Smirnov statististics for " + arguments.at("Source/Target group"));
    ks_item.SetType(result_type::vector);
    ks_item.SetYAxisMode(yaxis_mode::normal);

    distribution_type dist;
    if (arguments.at("Distribution") == "Normal")
    {
        dist = distribution_type::normal;
    }
    else if (arguments.at("Distribution") == "Lognormal")
    {
        dist = distribution_type::lognormal;
    }

    CMBVector* ks_output = new CMBVector(
        Data()->at(arguments.at("Source/Target group")).CalculateKolmogorovSmirnovStatistics(dist)
    );

    ks_item.SetShowTable(true);
    ks_item.SetResult(ks_output);
    results.Append(ks_item);

    return true;
}

bool Conductor::ExecuteKolmogorovSmirnovIndividual(const std::map<std::string, std::string>& arguments)
{
    results.SetName("Kolmogorov–Smirnov statististics for constituent " +
        arguments.at("Constituent") + " in group " +
        arguments.at("Source/Target group"));

    ResultItem ks_item;
    ks_item.SetName("Kolmogorov–Smirnov statististics for constituent " +
        arguments.at("Constituent") + " in group " +
        arguments.at("Source/Target group"));
    ks_item.SetType(result_type::timeseries_set);

    distribution_type dist;
    if (arguments.at("Distribution") == "Normal")
    {
        dist = distribution_type::normal;
    }
    else if (arguments.at("Distribution") == "Lognormal")
    {
        dist = distribution_type::lognormal;
    }

    CMBTimeSeriesSet* ks_output = new CMBTimeSeriesSet(
        Data()->at(arguments.at("Source/Target group"))
        .GetElementDistribution(arguments.at("Constituent"))
        ->CreateCDFComparison(dist)
    );

    ks_item.SetResult(ks_output);
    ks_item.SetShowTable(false);
    results.Append(ks_item);

    return true;
}

bool Conductor::ExecuteCMBBayesianBatch(const std::map<std::string, std::string>& arguments)
{
    if (arguments.at("Apply size and organic matter correction") == "true")
    {
        if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
        {
            QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
            return false;
        }
    }

    // Initialize MCMC object
    MCMC = std::make_unique<CMCMC<SourceSinkData>>();

    // Create progress window with 3 charts for monitoring (with batch mode enabled)
    ProgressWindow* rtw = new ProgressWindow(mainwindow, 3, true);
    rtw->SetTitle("Acceptance Rate", 0);
    rtw->SetTitle("Purturbation Factor", 1);
    rtw->SetTitle("Log posterior value", 2);
    rtw->SetYAxisTitle("Acceptance Rate", 0);
    rtw->SetYAxisTitle("Purturbation Factor", 1);
    rtw->SetYAxisTitle("Log posterior value", 2);
    rtw->show();

    // Execute batch MCMC sampling
    CMBMatrix* contributions = new CMBMatrix(
        Data()->MCMC_Batch(
            arguments,
            MCMC.get(),
            rtw,
            workingfolder.toStdString()
        )
    );

    results.SetName("CMB Bayesian-Batch");

    // Create result item for contribution range matrix
    ResultItem contribution_matrix_item;
    contribution_matrix_item.SetName("Contribution Range Matrix");
    contribution_matrix_item.SetType(result_type::matrix);
    contribution_matrix_item.SetYAxisMode(yaxis_mode::normal);
    contribution_matrix_item.SetResult(contributions);
    contribution_matrix_item.SetShowTable(true);
    results.Append(contribution_matrix_item);

    rtw->SetProgress(1);

    return true;
}

bool Conductor::ExecuteTestCMBBayesian(const std::map<std::string, std::string>& arguments)
{
    results.SetName("MCMC results for testing MCMC'");

    // Create MCMC samples result item
    ResultItem mcmc_samples;
    mcmc_samples.SetShowAsString(false);
    mcmc_samples.SetType(result_type::mcmc_samples);
    mcmc_samples.SetName("MCMC samples for testing MCMC'");

    CMBTimeSeriesSet* samples = new CMBTimeSeriesSet();

    // Initialize MCMC for testing with TestMCMC model
    CMCMC<TestMCMC>* mcmc_for_testing = new CMCMC<TestMCMC>();
    TestMCMC testing_model;
    mcmc_for_testing->Model = &testing_model;

    // Create progress window with 3 charts
    ProgressWindow* rtw = new ProgressWindow(mainwindow, 3);
    rtw->SetTitle("Acceptance Rate", 0);
    rtw->SetTitle("Purturbation Factor", 1);
    rtw->SetTitle("Log posterior value", 2);
    rtw->SetYAxisTitle("Acceptance Rate", 0);
    rtw->SetYAxisTitle("Purturbation Factor", 1);
    rtw->SetYAxisTitle("Log posterior value", 2);
    rtw->show();

    // Set up parameter bounds for test model
    std::vector<double> mins;
    std::vector<double> maxs;
    mins.push_back(0.1);
    mins.push_back(1);
    maxs.push_back(1.0);
    maxs.push_back(10);

    testing_model.InitializeParametersObservations(mins, maxs);

    // Configure MCMC parameters
    mcmc_for_testing->SetProperty("number_of_samples", arguments.at("Number of samples"));
    mcmc_for_testing->SetProperty("number_of_chains", arguments.at("Number of chains"));
    mcmc_for_testing->SetProperty("number_of_burnout_samples", arguments.at("Samples to be discarded (burnout)"));

    mcmc_for_testing->initialize(samples, true);

    // Determine output folder path
    std::string folder_path;
    if (!QString::fromStdString(arguments.at("samples_file_name")).contains("/"))
    {
        folder_path = workingfolder.toStdString() + "/";
    }

    // Execute MCMC sampling
    mcmc_for_testing->step(
        QString::fromStdString(arguments.at("Number of chains")).toInt(),
        QString::fromStdString(arguments.at("Number of samples")).toInt(),
        folder_path + arguments.at("samples_file_name"),
        samples,
        rtw
    );

    mcmc_samples.SetResult(samples);
    results.Append(mcmc_samples);

    // Generate posterior distributions from samples
    ResultItem distribution_result_item;
    CMBTimeSeriesSet* dists = new CMBTimeSeriesSet();
    *dists = samples->distribution(
        100,
        QString::fromStdString(arguments.at("Samples to be discarded (burnout)")).toInt()
    );

    distribution_result_item.SetName("Posterior Distributions");
    distribution_result_item.SetShowAsString(false);
    distribution_result_item.SetType(result_type::distribution);
    distribution_result_item.SetResult(dists);
    results.Append(distribution_result_item);

    // Clean up (TestMCMC uses raw pointer)
    delete mcmc_for_testing;

    return true;
}

bool Conductor::ExecuteDistributionFitting(const std::map<std::string, std::string>& arguments)
{
    results.SetName("Distribution fitting results for '" + arguments.at("Constituent") +
        "' in '" + arguments.at("Source/Target group"));

    // Prepare result items for PDF and CDF
    ResultItem distribution_item;
    distribution_item.SetName("Fitted PDF for '" + arguments.at("Constituent") +
        "' in '" + arguments.at("Source/Target group"));
    distribution_item.SetShowAsString(false);
    distribution_item.SetType(result_type::timeseries_set_first_symbol);
    distribution_item.SetYAxisMode(yaxis_mode::normal);
    distribution_item.setXAxisTitle("Value");
    distribution_item.setYAxisTitle("PDF");

    ResultItem cumulative_distribution_item;
    cumulative_distribution_item.SetName("Fitted CDF for '" + arguments.at("Constituent") +
        "' in '" + arguments.at("Source/Target group"));
    cumulative_distribution_item.SetShowAsString(false);
    cumulative_distribution_item.SetType(result_type::timeseries_set_first_symbol);
    cumulative_distribution_item.SetYAxisMode(yaxis_mode::normal);
    cumulative_distribution_item.setXAxisTitle("Value");
    cumulative_distribution_item.setYAxisTitle("CDF");

    bool exclude_samples = (arguments.at("Use only selected samples") == "true");

    SourceSinkData transformed_data;

    if (arguments.at("OM and Size Correct based on target sample") != "")
    {
        if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
        {
            QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
            return false;
        }
        transformed_data = Data()->CreateCorrectedDataset(
            arguments.at("OM and Size Correct based on target sample"),
            true,
            Data()->GetElementInformation()
        ).CreateCorrectedAndFilteredDataset(exclude_samples, false, false);
    }
    else
    {
        transformed_data = Data()->CreateCorrectedAndFilteredDataset(exclude_samples, false, false);
    }

    if (!CheckNegativeElements(&transformed_data))
        return false;

    if (arguments.at("Box-cox transformation") == "true")
    {
        transformed_data = transformed_data.BoxCoxTransformed(true);
    }

    // Fit normal distribution
    CMBTimeSeriesSet fitted_normal = transformed_data.at(arguments.at("Source/Target group"))
        .GetElementDistribution(arguments.at("Constituent"))
        ->CreateFittedDistribution(distribution_type::normal);

    // Fit lognormal distribution (only if not Box-Cox transformed)
    CMBTimeSeriesSet fitted_lognormal;
    if (arguments.at("Box-cox transformation") != "true")
    {
        fitted_lognormal = transformed_data.at(arguments.at("Source/Target group"))
            .GetElementDistribution(arguments.at("Constituent"))
            ->CreateFittedDistribution(distribution_type::lognormal);
    }

    // Create CDF comparisons for normal
    CMBTimeSeriesSet observed_fitted_normal_cdf = transformed_data.at(arguments.at("Source/Target group"))
        .GetElementDistribution(arguments.at("Constituent"))
        ->CreateCDFComparison(distribution_type::normal);

    // Create CDF comparisons for lognormal (only if not Box-Cox transformed)
    CMBTimeSeriesSet observed_fitted_lognormal_cdf;
    if (arguments.at("Box-cox transformation") != "true")
    {
        observed_fitted_lognormal_cdf = transformed_data.at(arguments.at("Source/Target group"))
            .GetElementDistribution(arguments.at("Constituent"))
            ->CreateCDFComparison(distribution_type::lognormal);
    }

    // Build PDF result
    CMBTimeSeriesSet* pdf = new CMBTimeSeriesSet();
    pdf->append(fitted_normal["Observed"]);
    pdf->append(fitted_normal["Fitted"]);
    if (arguments.at("Box-cox transformation") != "true")
    {
        pdf->append(fitted_lognormal["Fitted"]);
    }
    pdf->setname(0, "Samples");
    pdf->setname(1, "Normal");
    if (arguments.at("Box-cox transformation") != "true")
    {
        pdf->setname(2, "Log-normal");
    }

    // Build CDF result
    CMBTimeSeriesSet* cdf = new CMBTimeSeriesSet();
    cdf->append(observed_fitted_normal_cdf["Observed"]);
    cdf->append(observed_fitted_normal_cdf["Fitted"]);
    if (arguments.at("Box-cox transformation") != "true")
    {
        cdf->append(observed_fitted_lognormal_cdf["Fitted"]);
    }
    cdf->setname(0, "Observed");
    cdf->setname(1, "Normal");
    if (arguments.at("Box-cox transformation") != "true")
    {
        cdf->setname(2, "Log-normal");
    }

    distribution_item.SetResult(pdf);
    cumulative_distribution_item.SetResult(cdf);

    results.Append(distribution_item);
    results.Append(cumulative_distribution_item);

    return true;
}

bool Conductor::ExecuteBracketingAnalysis(const std::map<std::string, std::string>& arguments)
{
    results.SetName("Bracketing analysis for sample '" + arguments.at("Sample") + "'");

    ResultItem bracketing_result_item;
    bracketing_result_item.SetName("Bracketing results");
    bracketing_result_item.SetType(result_type::vector);
    bracketing_result_item.SetShowTable(true);
    bracketing_result_item.SetShowGraph(false);

    bool exclude_samples = (arguments.at("Use only selected samples") == "true");
    bool exclude_elements = (arguments.at("Use only selected elements") == "true");
    bool correct_based_on_size_and_organic_matter = (arguments.at("Correct based on size and organic matter") == "true");

    SourceSinkData transformed_data = Data()->CreateCorrectedAndFilteredDataset(
        exclude_samples,
        exclude_elements,
        correct_based_on_size_and_organic_matter,
        arguments.at("Sample")
    );

    if (!CheckNegativeElements(&transformed_data))
        return false;

    CMBVector* bracketing_result = new CMBVector(
        transformed_data.BracketTest(arguments.at("Sample"), false)
    );

    bracketing_result->SetBooleanValue(true);
    bracketing_result_item.SetResult(bracketing_result);
    results.Append(bracketing_result_item);

    return true;
}

bool Conductor::ExecuteBracketingAnalysisBatch(const std::map<std::string, std::string>& arguments)
{
    results.SetName("Bracketing analysis");

    ResultItem bracketing_result_item;
    bracketing_result_item.SetName("Bracketing results");
    bracketing_result_item.SetType(result_type::matrix);
    bracketing_result_item.SetShowTable(true);
    bracketing_result_item.SetShowGraph(false);
    bracketing_result_item.SetShowAsString(false);

    bool exclude_samples = (arguments.at("Use only selected samples") == "true");
    bool exclude_elements = (arguments.at("Use only selected elements") == "true");
    bool correct_based_on_size_and_organic_matter = (arguments.at("Correct based on size and organic matter") == "true");

    if (!CheckNegativeElements(data))
        return false;

    if (correct_based_on_size_and_organic_matter)
    {
        if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
        {
            QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
            return false;
        }
    }

    CMBMatrix* bracketing_result = new CMBMatrix(
        Data()->BracketTest(
            correct_based_on_size_and_organic_matter,
            exclude_elements,
            exclude_samples
        )
    );

    bracketing_result->SetBooleanValue(true);
    bracketing_result_item.SetResult(bracketing_result);
    results.Append(bracketing_result_item);

    return true;
}

bool Conductor::ExecuteBoxCox(const std::map<std::string, std::string>& arguments)
{
    results.SetName("Box-Cox parameter for '" + arguments.at("Source/Target group") + "'");

    ResultItem boxcox_result_item;
    boxcox_result_item.SetName("Box-Cox parameters");
    boxcox_result_item.SetType(result_type::vector);
    boxcox_result_item.SetShowTable(true);
    boxcox_result_item.SetShowGraph(true);
    boxcox_result_item.SetYAxisMode(yaxis_mode::normal);

    CMBVector* boxcox_params = new CMBVector(
        Data()->at(arguments.at("Source/Target group")).CalculateBoxCoxParameters()
    );

    boxcox_result_item.SetResult(boxcox_params);
    results.Append(boxcox_result_item);

    return true;
}

bool Conductor::ExecuteOutlierAnalysis(const std::map<std::string, std::string>& arguments)
{
    bool exclude_samples = (arguments.at("Use only selected samples") == "true");
    bool exclude_elements = (arguments.at("Use only selected elements") == "true");

    SourceSinkData transformed_data = Data()->CreateCorrectedAndFilteredDataset(
        exclude_samples,
        exclude_elements,
        false
    );

    if (!CheckNegativeElements(&transformed_data))
        return false;

    results.SetName("Outlier analysis for '" + arguments.at("Source/Target group") + "'");

    ResultItem outlier_result_item;
    outlier_result_item.SetName("Outlier Analysis");
    outlier_result_item.SetType(result_type::matrix);
    outlier_result_item.SetShowAsString(false);
    outlier_result_item.SetShowTable(true);
    outlier_result_item.SetShowGraph(false);

    double threshold = QString::fromStdString(arguments.at("Threshold")).toDouble();

    CMBMatrix* outlier_matrix = new CMBMatrix(
        transformed_data.at(arguments.at("Source/Target group")).DetectOutliers(-threshold, threshold)
    );

    outlier_matrix->SetLimit(_range::high, threshold);
    outlier_matrix->SetLimit(_range::low, -threshold);

    outlier_result_item.SetResult(outlier_matrix);
    results.Append(outlier_result_item);

    return true;
}

bool Conductor::ExecuteEDP(const std::map<std::string, std::string>& arguments)
{
    results.SetName("Two-way element discriminant power between '" +
        arguments.at("Source/Target group I") + "' and '" +
        arguments.at("Source/Target group II") + "'");

    bool exclude_samples = (arguments.at("Use only selected samples") == "true");

    SourceSinkData transformed_data;

    if (arguments.at("OM and Size Correct based on target sample") != "")
    {
        if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
        {
            QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
            return false;
        }
        transformed_data = Data()->CreateCorrectedDataset(
            arguments.at("OM and Size Correct based on target sample"),
            true,
            Data()->GetElementInformation()
        ).CreateCorrectedAndFilteredDataset(exclude_samples, false, false);
    }
    else
    {
        transformed_data = Data()->CreateCorrectedAndFilteredDataset(exclude_samples, false, false);
    }

    if (!CheckNegativeElements(&transformed_data))
        return false;

    if (arguments.at("Box-cox transformation") == "true")
    {
        transformed_data = transformed_data.BoxCoxTransformed(true);
    }

    // Discriminant difference to standard deviation ratio
    ResultItem edp_result_std;
    edp_result_std.SetName("Discreminant difference to standard deviation ratio");
    edp_result_std.SetType(result_type::predicted_concentration);
    edp_result_std.SetShowAsString(false);
    edp_result_std.SetShowTable(true);
    edp_result_std.SetShowGraph(true);

    Elemental_Profile* edp_profile_set = new Elemental_Profile(
        transformed_data.DifferentiationPower(
            arguments.at("Source/Target group I"),
            arguments.at("Source/Target group II"),
            false
        )
    );

    edp_result_std.SetYAxisMode(yaxis_mode::normal);
    edp_result_std.setYAxisTitle("Discrimination power");
    edp_result_std.SetResult(edp_profile_set);
    edp_result_std.setXAxisTitle("Element");
    edp_result_std.setYAxisTitle("Standard deviation to mean ratio");
    results.Append(edp_result_std);

    // Discriminant fraction
    ResultItem edp_result_percent;
    edp_result_percent.SetName("Discriminat fraction");
    edp_result_percent.SetType(result_type::predicted_concentration);
    edp_result_percent.SetShowAsString(false);
    edp_result_percent.setYAxisTitle("Percentage discriminated");
    edp_result_percent.SetShowTable(true);
    edp_result_percent.SetShowGraph(true);

    Elemental_Profile* edp_profile_set_percent = new Elemental_Profile(
        transformed_data.DifferentiationPower_Percentage(
            arguments.at("Source/Target group I"),
            arguments.at("Source/Target group II")
        )
    );

    edp_result_percent.SetYAxisMode(yaxis_mode::normal);
    edp_result_percent.SetYLimit(_range::high, 1);
    edp_result_percent.SetResult(edp_profile_set_percent);
    edp_result_percent.setXAxisTitle("Element");
    edp_result_percent.setYAxisTitle("Discriminant fraction");
    results.Append(edp_result_percent);

    // Discriminant p-value
    ResultItem edp_result_p_value;
    edp_result_p_value.SetName("Discriminat p-value");
    edp_result_p_value.SetType(result_type::predicted_concentration);
    edp_result_p_value.SetShowAsString(false);
    edp_result_p_value.setYAxisTitle("p-Value");
    edp_result_p_value.SetShowTable(true);
    edp_result_p_value.SetShowGraph(true);

    Elemental_Profile* edp_profile_set_p_value = new Elemental_Profile(
        transformed_data.t_TestPValue(
            arguments.at("Source/Target group I"),
            arguments.at("Source/Target group II"),
            false
        )
    );

    edp_result_p_value.SetYAxisMode(yaxis_mode::normal);
    edp_result_p_value.SetYLimit(_range::high, 1);
    edp_result_p_value.SetResult(edp_profile_set_p_value);
    edp_profile_set_p_value->SetLimit(_range::high, aquiutils::atof(arguments.at("P-value threshold")));
    edp_profile_set_p_value->SetLimit(_range::low, 0);
    edp_result_p_value.setXAxisTitle("Element");
    edp_result_p_value.setYAxisTitle("Discriminant fraction");
    results.Append(edp_result_p_value);

    return true;
}

bool Conductor::ExecuteEDPM(const std::map<std::string, std::string>& arguments)
{
    bool include_target = (arguments.at("Include target samples") == "true");
    bool exclude_samples = (arguments.at("Use only selected samples") == "true");

    SourceSinkData transformed_data;

    if (arguments.at("OM and Size Correct based on target sample") != "")
    {
        if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
        {
            QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
            return false;
        }
        transformed_data = Data()->CreateCorrectedDataset(
            arguments.at("OM and Size Correct based on target sample"),
            true,
            Data()->GetElementInformation()
        ).CreateCorrectedAndFilteredDataset(exclude_samples, false, false);
    }
    else
    {
        transformed_data = Data()->CreateCorrectedAndFilteredDataset(exclude_samples, false, false);
    }

    if (!CheckNegativeElements(&transformed_data))
        return false;

    if (arguments.at("Box-cox transformation") == "true")
    {
        transformed_data = transformed_data.BoxCoxTransformed(true);
    }

    results.SetName("Multi-way element discriminant power between '" +
        arguments.at("Source/Target group I") + "' and '" +
        arguments.at("Source/Target group II") + "'");

    // Multi-way discriminant difference to standard deviation ratio
    ResultItem edp_result_std;
    edp_result_std.SetName("Multi-way discreminant difference to standard deviation ratio");
    edp_result_std.SetType(result_type::elemental_profile_set);
    edp_result_std.SetShowAsString(true);
    edp_result_std.SetShowTable(true);
    edp_result_std.SetShowGraph(true);

    Elemental_Profile_Set* edp_profile_set = new Elemental_Profile_Set(
        transformed_data.DifferentiationPower(false, include_target)
    );

    edp_result_std.SetYAxisMode(yaxis_mode::normal);
    edp_result_std.setYAxisTitle("Discrimination power");
    edp_result_std.SetResult(edp_profile_set);
    results.Append(edp_result_std);

    // Multi-way discriminant fraction
    ResultItem edp_result_percent;
    edp_result_percent.SetName("Multi-way discriminat fraction");
    edp_result_percent.SetType(result_type::elemental_profile_set);
    edp_result_percent.SetShowAsString(true);
    edp_result_percent.setYAxisTitle("Percentage discriminated");
    edp_result_percent.SetShowTable(true);
    edp_result_percent.SetShowGraph(true);

    Elemental_Profile_Set* edp_profile_set_percent = new Elemental_Profile_Set(
        transformed_data.DifferentiationPower_Percentage(include_target)
    );

    edp_result_percent.SetYAxisMode(yaxis_mode::normal);
    edp_result_percent.SetYLimit(_range::high, 1);
    edp_result_percent.SetResult(edp_profile_set_percent);
    results.Append(edp_result_percent);

    // Multi-way discriminant p-value
    ResultItem edp_p_value;
    edp_p_value.SetName("Multi-way discriminat p-value");
    edp_p_value.SetType(result_type::elemental_profile_set);
    edp_p_value.SetShowAsString(true);
    edp_p_value.setYAxisTitle("p-Value");
    edp_p_value.SetShowTable(true);
    edp_p_value.SetShowGraph(true);

    Elemental_Profile_Set* edp_profile_set_p_value = new Elemental_Profile_Set(
        transformed_data.DifferentiationPower_P_value(include_target)
    );

    edp_p_value.SetYAxisMode(yaxis_mode::normal);
    edp_p_value.SetYLimit(_range::high, 1);
    edp_p_value.SetResult(edp_profile_set_p_value);
    edp_profile_set_p_value->SetLimit(_range::high, aquiutils::atof(arguments.at("P-value threshold")));
    edp_profile_set_p_value->SetLimit(_range::low, 0);
    results.Append(edp_p_value);

    return true;
}

bool Conductor::ExecuteANOVA(const std::map<std::string, std::string>& arguments)
{
    bool log_transformation = (arguments.at("Log Transformation") == "true");
    bool exclude_samples = (arguments.at("Use only selected samples") == "true");
    bool exclude_elements = (arguments.at("Use only selected elements") == "true");

    SourceSinkData transformed_data;

    if (arguments.at("OM and Size Correct based on target sample") != "")
    {
        if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
        {
            QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
            return false;
        }
        transformed_data = Data()->CreateCorrectedDataset(
            arguments.at("OM and Size Correct based on target sample"),
            true,
            Data()->GetElementInformation()
        ).CreateCorrectedAndFilteredDataset(true, false, false);
    }
    else
    {
        transformed_data = Data()->CreateCorrectedAndFilteredDataset(true, false, false);
    }

    if (!CheckNegativeElements(&transformed_data))
        return false;

    if (arguments.at("Box-cox transformation") == "true")
    {
        transformed_data = transformed_data.BoxCoxTransformed(true);
        log_transformation = false;
    }

    results.SetName("ANOVA analysis");

    ResultItem anova_results;
    anova_results.SetName("ANOVA");
    anova_results.SetType(result_type::vector);
    anova_results.SetShowAsString(true);
    anova_results.SetShowTable(true);
    anova_results.SetShowGraph(true);

    CMBVector* p_values = new CMBVector(transformed_data.ANOVA(log_transformation));

    p_values->SetLimit(_range::high, aquiutils::atof(arguments.at("P-value threshold")));
    p_values->SetLimit(_range::low, 0);

    anova_results.SetYAxisMode(yaxis_mode::normal);
    anova_results.SetResult(p_values);
    anova_results.setYAxisTitle("P-value");
    anova_results.setXAxisTitle("Element");

    if (arguments.at("Modify the included elements based on the results") == "true")
    {
        std::vector<std::string> selected = p_values->ExtractWithinRange(
            0,
            aquiutils::atof(arguments.at("P-value threshold"))
        ).Labels();
        Data()->IncludeExcludeElementsBasedOn(selected);
    }

    results.Append(anova_results);

    return true;
}

bool Conductor::ExecuteErrorAnalysis(const std::map<std::string, std::string>& arguments)
{
    results.SetName("Error Analysis");

    ProgressWindow* rtw = new ProgressWindow(mainwindow, 0);

    bool organic_size_correction;
    if (arguments.at("Apply size and organic matter correction") == "true")
    {
        organic_size_correction = true;
        if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
        {
            QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
            return false;
        }
    }
    else
    {
        organic_size_correction = false;
    }

    rtw->show();

    SourceSinkData corrected_data = Data()->CreateCorrectedDataset(
        arguments.at("Sample"),
        organic_size_correction,
        Data()->GetElementInformation()
    );

    corrected_data.SetProgressWindow(rtw);

    if (!CheckNegativeElements(&corrected_data))
        return false;

    bool softmax = (arguments.at("Softmax transformation") == "true");

    corrected_data.InitializeParametersAndObservations(arguments.at("Sample"));

    bool outcome = corrected_data.BootStrap(
        &results,
        aquiutils::atof(arguments.at("Pecentage eliminated")),
        aquiutils::atoi(arguments.at("Number of realizations")),
        arguments.at("Sample"),
        softmax
    );

    return outcome;
}

bool Conductor::ExecuteSourceVerify(const std::map<std::string, std::string>& arguments)
{
    results.SetName("Source Verification");

    ProgressWindow* rtw = new ProgressWindow(mainwindow, 0);

    bool organic_size_correction;
    if (arguments.at("Apply size and organic matter correction") == "true")
    {
        organic_size_correction = true;
        if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
        {
            QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
            return false;
        }
    }
    else
    {
        organic_size_correction = false;
    }

    rtw->show();

    SourceSinkData corrected_data = *Data();
    corrected_data.SetProgressWindow(rtw);

    if (!CheckNegativeElements(&corrected_data))
        return false;

    bool softmax = (arguments.at("Softmax transformation") == "true");

    CMBTimeSeriesSet* contributions = new CMBTimeSeriesSet(
        corrected_data.VerifySource(
            arguments.at("Source Group"),
            softmax,
            organic_size_correction
        )
    );

    ResultItem contributions_result_item;
    results.SetName("Source verification for source'" + arguments.at("Source Group") + "'");

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

    return true;
}

bool Conductor::ExecuteAutoSelect(const std::map<std::string, std::string>& arguments)
{
    results.SetName("Auto-select elements");

    SourceSinkData transformed_data;

    if (arguments.at("OM and Size Correct based on target sample") != "")
    {
        if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
        {
            QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
            return false;
        }
        transformed_data = Data()->CreateCorrectedDataset(
            arguments.at("OM and Size Correct based on target sample"),
            true,
            Data()->GetElementInformation()
        ).CreateCorrectedAndFilteredDataset(true, false, false);
    }
    else
    {
        transformed_data = Data()->CreateCorrectedAndFilteredDataset(true, false, false);
    }

    if (!CheckNegativeElements(&transformed_data))
        return false;

    bool isotopes = (arguments.at("Include Isotopes") == "true");

    transformed_data = transformed_data.ExtractChemicalElements(isotopes);

    // Multi-way discriminant p-value result
    ResultItem edp_p_value;
    edp_p_value.SetName("Multi-way discriminat p-value");
    edp_p_value.SetType(result_type::elemental_profile_set);
    edp_p_value.SetShowAsString(true);
    edp_p_value.setYAxisTitle("p-Value");
    edp_p_value.SetShowTable(true);
    edp_p_value.SetShowGraph(true);

    Elemental_Profile_Set* edp_profile_set_p_value = new Elemental_Profile_Set(
        transformed_data.DifferentiationPower_P_value(false)
    );

    edp_p_value.SetYAxisMode(yaxis_mode::normal);
    edp_p_value.SetYLimit(_range::high, 1);
    edp_p_value.SetResult(edp_profile_set_p_value);
    results.Append(edp_p_value);

    // Selected elements result
    Elemental_Profile* selected = new Elemental_Profile(
        edp_profile_set_p_value->SelectTopElementsAggregate(
            aquiutils::atoi(arguments.at("Number of elements from each pair"))
        )
    );

    ResultItem selected_elements;
    selected_elements.SetResult(selected);
    selected_elements.SetName("Selected Elements");
    selected_elements.SetType(result_type::predicted_concentration);
    selected_elements.SetShowAsString(true);
    selected_elements.setYAxisTitle("p-Value");
    selected_elements.SetShowTable(true);
    selected_elements.SetShowGraph(true);
    results.Append(selected_elements);

    if (arguments.at("Modify the included elements based on the results") == "true")
    {
        Data()->IncludeExcludeElementsBasedOn(selected->GetElementNames());
    }

    return true;
}

bool Conductor::ExecuteCMBBayesian(const std::map<std::string, std::string>& arguments)
{
    if (arguments.at("Apply size and organic matter correction") == "true")
    {
        if (Data()->OMandSizeConstituents()[0] == "" && Data()->OMandSizeConstituents()[1] == "")
        {
            QMessageBox::warning(mainwindow, "SedSAT3", "Perform Organic Matter and Size Correction first!\n", QMessageBox::Ok);
            return false;
        }
    }

    // Initialize MCMC object
    MCMC = std::make_unique<CMCMC<SourceSinkData>>();

    // Create progress window with 3 charts for monitoring
    ProgressWindow* rtw = new ProgressWindow(mainwindow, 3);
    rtw->SetTitle("Acceptance Rate", 0);
    rtw->SetTitle("Purturbation Factor", 1);
    rtw->SetTitle("Log posterior value", 2);
    rtw->SetYAxisTitle("Acceptance Rate", 0);
    rtw->SetYAxisTitle("Purturbation Factor", 1);
    rtw->SetYAxisTitle("Log posterior value", 2);
    rtw->show();

    // Execute MCMC sampling
    results = Data()->MCMC(
        arguments.at("Sample"),
        arguments,
        MCMC.get(),
        rtw,
        workingfolder.toStdString()
    );

    // Check for errors during execution
    if (results.Error() != "")
    {
        QMessageBox::warning(mainwindow, "SedSat3", QString::fromStdString(results.Error()), QMessageBox::Ok);
        return false;
    }

    rtw->SetProgress(1);

    return true;
}