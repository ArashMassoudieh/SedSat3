#include "conductor.h"
#include "ProgressWindow.h"
#include "results.h"
#include "contribution.h"
#include "resultitem.h"
#include "testmcmc.h"
#include "rangeset.h"


Conductor::Conductor()
{

}

bool Conductor::Execute(const string &command, map<string,string> arguments)
{
    results.clear();
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

        results.SetName("GA " + arguments["Sample"]);

        ResultItem result_cont = GA->Model_out.GetContribution();
        results.Append(result_cont);

        ResultItem result_calculated_means = GA->Model_out.GetCalculatedElementMeans();
        results.Append(result_calculated_means);

        ResultItem result_estimated_means = GA->Model_out.GetEstimatedElementMean();
        results.Append(result_estimated_means);

        ResultItem result_modeled_vs_measured = GA->Model_out.GetObservedvsModeledElementalProfile();
        results.Append(result_modeled_vs_measured);

        ResultItem result_modeled_vs_measured_isotope = GA->Model_out.GetObservedvsModeledElementalProfile_Isotope();
        results.Append(result_modeled_vs_measured_isotope);

    }
    if (command == "GA (fixed elemental contribution)")
    {
        if (GA!=nullptr) delete GA;
        ProgressWindow *rtw = new ProgressWindow();
        rtw->SetTitle("Fitness",0);
        rtw->SetYAxisTitle("Fitness",0);
        bool organicnsizecorrection;
        if (arguments["Apply size and organic matter correction"]=="true")
            organicnsizecorrection = true;
        else
            organicnsizecorrection = false;

        SourceSinkData correctedData = Data()->Corrected(arguments["Sample"],organicnsizecorrection);
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
        ProgressWindow *rtw = new ProgressWindow();
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

        ResultItem result_calculated_stds = GA->Model_out.GetCalculatedElementStandardDev();
        results.Append(result_calculated_stds);

        ResultItem result_estimated_stds = GA->Model_out.GetEstimatedElementSigma();
        results.Append(result_estimated_stds);



    }
    if (command == "Levenberg-Marquardt")
    {
        ProgressWindow* rtw = new ProgressWindow();
        rtw->show();
        bool organicnsizecorrection;
        if (arguments["Apply size and organic matter correction"]=="true")
            organicnsizecorrection = true;
        else
            organicnsizecorrection = false;

        SourceSinkData correctedData = Data()->Corrected(arguments["Sample"],organicnsizecorrection);
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
    if (command == "OM-Size Correct")
    {
        SourceSinkData correctedData = Data()->Corrected(arguments["Sample"]);
        vector<ResultItem> resultitems = correctedData.GetSourceProfiles();
        results.SetName("Corrected Elemental Profiles for Target" + arguments["Sample"]);
        for (unsigned int i=0; i<resultitems.size(); i++)
            results.Append(resultitems[i]);
    }
    if (command == "MLR")
    {
        results.SetName("MLR_vs_OM&Size ");

        if (arguments["Equation"]=="Linear")
            Data()->Perform_Regression_vs_om_size(arguments["Organic Matter constituent"],arguments["Particla Size constituent"],regression_form::linear);
        else
            Data()->Perform_Regression_vs_om_size(arguments["Organic Matter constituent"],arguments["Particla Size constituent"],regression_form::power);
        vector<ResultItem> regression_result = Data()->GetMLRResults();
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
        corMatResItem.SetName("Correlation Matrix for " + arguments["Source/Target group"] );
        corMatResItem.SetShowTable(true);
        corMatResItem.SetType(result_type::matrix);
        corMatResItem.SetShowGraph(false);
        double threshold = QString::fromStdString(arguments["Threshold"]).toDouble();
        CMBMatrix *cormatr = new CMBMatrix(Data()->at(arguments["Source/Target group"]).CorrelationMatrix());
        cormatr->SetLimit(_range::high,threshold);
        cormatr->SetLimit(_range::low,-threshold);
        corMatResItem.SetResult(cormatr);
        results.Append(corMatResItem);

    }
    if (command == "DFA")
    {
        results.SetName("DFA coefficients between " + arguments["Source/Target group I"] + "&" + arguments["Source/Target group II"] );
        ResultItem DFAResItem;
        DFAResItem.SetName("DFA coefficients between " + arguments["Source/Target group I"] + "&" + arguments["Source/Target group II"]  );
        DFAResItem.SetType(result_type::vector);
        DFAResItem.SetShowTable(true);
        DFAResItem.SetYAxisMode(yaxis_mode::normal);
        CMBVector *dfaeigenvector = new CMBVector(Data()->DiscriminantFunctionAnalysis(arguments["Source/Target group I"],arguments["Source/Target group II"]));
        DFAResItem.SetResult(dfaeigenvector);
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
            organicnsizecorrection = true;
        else
            organicnsizecorrection = false;

        SourceSinkData correctedData = Data()->Corrected(arguments["Sample"],organicnsizecorrection);

        if (MCMC!=nullptr) delete MCMC;
        MCMC = new CMCMC<SourceSinkData>();
        MCMC->Model = &correctedData;
        ProgressWindow *rtw = new ProgressWindow(nullptr,3);
        rtw->SetTitle("Acceptance Rate",0);
        rtw->SetTitle("Purturbation Factor",1);
        rtw->SetTitle("Log posterior value",2);
        rtw->SetYAxisTitle("Acceptance Rate",0);
        rtw->SetYAxisTitle("Purturbation Factor",1);
        rtw->SetYAxisTitle("Log posterior value",2);
        rtw->show();
// Samples
        correctedData.InitializeParametersObservations(arguments["Sample"]);
        MCMC->SetProperty("number_of_samples",arguments["Number of samples"]);
        MCMC->SetProperty("number_of_chains",arguments["Number of chains"]);
        MCMC->SetProperty("number_of_burnout_samples",arguments["Samples to be discarded (burnout)"]);
        MCMC->SetProperty("dissolve_chains",arguments["Dissolve Chains"]);
        MCMC->initialize(samples,true);
        string folderpath;
        if (!QString::fromStdString(arguments["samples_file_name"]).contains("/"))
            folderpath = workingfolder.toStdString()+"/";
        MCMC->step(QString::fromStdString(arguments["Number of chains"]).toInt(), QString::fromStdString(arguments["Number of samples"]).toInt(), folderpath + arguments["samples_file_name"], samples, rtw);
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
        ResultItem predicted_distribution_res_item;
        CMBTimeSeriesSet *predicted_dists = new CMBTimeSeriesSet();

        vector<string> ConstituentNames = correctedData.ElementOrder();
        vector<string> IsotopeNames = correctedData.IsotopeOrder();

        ConstituentNames.insert( ConstituentNames.end(), IsotopeNames.begin(), IsotopeNames.end() );
        for (int i=0; i<predicted_samples.nvars; i++)
            predicted_samples.setname(i,ConstituentNames[i]);


        *predicted_dists = predicted_samples.distribution(100,0,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
        for (int i=0; i<predicted_samples.nvars; i++)
            predicted_dists->SetObservedValue(i,correctedData.observation(i)->Value());

        predicted_distribution_res_item.SetName("Posterior Predicted Constituents");
        predicted_distribution_res_item.SetShowAsString(false);
        predicted_distribution_res_item.SetShowTable(true);
        predicted_distribution_res_item.SetType(result_type::distribution_with_observed);
        predicted_distribution_res_item.SetResult(predicted_dists);
        results.Append(predicted_distribution_res_item);

//predicted 95% credible intervals

        RangeSet *predicted_credible_intervals = new RangeSet();
        vector<double> percentile_low = predicted_samples.percentile(0.025,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
        vector<double> percentile_high = predicted_samples.percentile(0.975,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
        vector<double> mean = predicted_samples.mean(QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
        vector<double> median = predicted_samples.percentile(0.5,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
        for (int i=0; i<predicted_dists->nvars; i++)
        {
            Range range;
            range.Set(_range::low,percentile_low[i]);
            range.Set(_range::high,percentile_high[i]);
            range.SetMean(mean[i]);
            range.SetMedian(median[i]);
            predicted_credible_intervals->operator[](predicted_dists->names[i]) = range;
            predicted_credible_intervals->operator[](predicted_dists->names[i]).SetValue(correctedData.observation(i)->Value());
        }
        ResultItem predicted_credible_intervals_result_item;
        predicted_credible_intervals_result_item.SetName("Predicted Samples Credible Intervals");
        predicted_credible_intervals_result_item.SetShowAsString(true);
        predicted_credible_intervals_result_item.SetShowTable(true);
        predicted_credible_intervals_result_item.SetType(result_type::rangeset_with_observed);
        predicted_credible_intervals_result_item.SetResult(predicted_credible_intervals);
        predicted_credible_intervals_result_item.SetYAxisMode(yaxis_mode::log);
        results.Append(predicted_credible_intervals_result_item);
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
        ProgressWindow *rtw = new ProgressWindow(nullptr,3);
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
        CMBVector *bracketingresult = new CMBVector(Data()->BracketTest(arguments["Sample"]));
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
        results.SetName("Outlier analysis for '" + arguments["Source/Target group"] + "'");
        ResultItem OutlierResItem;
        OutlierResItem.SetName("Outlier Analysis");
        OutlierResItem.SetType(result_type::matrix);
        OutlierResItem.SetShowAsString(false);
        OutlierResItem.SetShowTable(true);
        OutlierResItem.SetShowGraph(false);
        double threshold = QString::fromStdString(arguments["Threshold"]).toDouble();
        CMBMatrix *outliermatrix = new CMBMatrix(Data()->at(arguments["Source/Target group"]).Outlier());
        outliermatrix->SetLimit(_range::high,threshold);
        outliermatrix->SetLimit(_range::low,-threshold);
        OutlierResItem.SetResult(outliermatrix);
        results.Append(OutlierResItem);

    }
    return true;
}
