// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

// Unit tests for the defects found in the USGS code review (October 2026) and
// for the reproducibility of the MCMC. Run with the unit-test build:
//
//   qmake tests/unit/unittests.pro && make && ./sedsat3-unittests
//
// Each test names the review item it covers; docs/review/REVIEW_RESPONSE.md
// lists them.

#include <QtTest>
#include <QApplication>
#include <QCheckBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>

#include <cmath>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "analysishost.h"
#include "progressreporter.h"
#include "conductor.h"
#include "sourcesinkdata.h"
#include "concentrationset.h"
#include "numericparsing.h"
#include "randomstream.h"
#include "MCMC.h"
#include "testmcmc.h"
#include "Binary.h"
#include "elementstablemodel.h"
#include "formelementinformation.h"

namespace {

/// AnalysisHost that records warnings instead of showing them
class RecordingHost : public AnalysisHost
{
public:
    void ShowWarning(const std::string& message) override { warnings.push_back(message); }
    ProgressReporter* CreateProgressReporter(int, bool) override { return new NullProgressReporter(); }
    void DisposeProgressReporter(ProgressReporter* reporter) override { delete reporter; }

    bool Warned(const std::string& text) const
    {
        for (const std::string& warning : warnings)
            if (warning.find(text) != std::string::npos)
                return true;
        return false;
    }

    std::vector<std::string> warnings;
};

/// group -> sample -> constituent -> value
using GroupData = std::map<std::string, std::map<std::string, std::map<std::string, double>>>;

/// Builds a dataset the way the importers do
void BuildData(SourceSinkData& data, const GroupData& groups, const std::string& target)
{
    for (const auto& [group, samples] : groups)
    {
        Elemental_Profile_Set* set = data.AppendSampleSet(group);
        for (const auto& [sample, values] : samples)
        {
            Elemental_Profile profile;
            for (const auto& [constituent, value] : values)
                profile.AppendElement(constituent, value);
            set->AppendProfile(sample, profile);
        }
    }
    data.SetTargetGroup(target);
    data.PopulateElementInformation();
    data.PopulateElementDistributions();
    data.AssignAllDistributions();
}

/// Two sources and a target, all values positive
GroupData PositiveGroups()
{
    return {
        {"Agriculture", {{"A1", {{"Al", 8.1}, {"Fe", 4.0}}}, {"A2", {{"Al", 7.6}, {"Fe", 4.4}}},
                         {"A3", {{"Al", 8.9}, {"Fe", 3.7}}}, {"A4", {{"Al", 8.4}, {"Fe", 4.1}}}}},
        {"Forest", {{"F1", {{"Al", 5.2}, {"Fe", 2.9}}}, {"F2", {{"Al", 5.9}, {"Fe", 3.1}}},
                    {"F3", {{"Al", 4.8}, {"Fe", 2.6}}}, {"F4", {{"Al", 5.5}, {"Fe", 3.3}}}}},
        {"Mixture", {{"M1", {{"Al", 6.8}, {"Fe", 3.5}}}}},
    };
}

QJsonObject ResultsJson(Conductor& conductor)
{
    Results* results = conductor.GetResults();
    QJsonObject json = results->toJsonObject();
    delete results;
    return json;
}

} // namespace

class SedSAT3Tests : public QObject
{
    Q_OBJECT

private slots:
    // ---------- Numeric parser (review: NaN handling; Phase 4) ----------
    void parser_acceptsNumbers_data();
    void parser_acceptsNumbers();
    void parser_rejects_data();
    void parser_rejects();
    void parser_variantAndJson();

    // ---------- Box-Cox (review item 2.6) ----------
    void boxcox_positiveData();
    void boxcox_zeroAndNegativeData();
    void boxcox_lambdaZeroAndNearZero();
    void boxcox_logLikelihoodIsFinite();

    // ---------- CBinary::operator[] (review item 2.4) ----------
    void binary_indexing();

    // ---------- CheckNegativeElements (review item 2.3) ----------
    void negativeCheck_clean();
    void negativeCheck_negative();
    void negativeCheck_sourceAfterTarget();

    // ---------- "Include all elements" (review item 2.2 / G1) ----------
    void includeAll_dataLevel();
    void includeAll_checkBox();

    // ---------- Distribution fitting (review item 2.1 / B1) ----------
    void distributionFitting_degenerateSets();
    void distributionFitting_conductor();

    // ---------- Random numbers and MCMC (review: RNG seed, thread safety) ----------
    void random_streamsAreReproducible();
    void random_seedParsing();
    void mcmc_sameSeedSameResult();
    void mcmc_differentSeedDifferentResult();
    void mcmc_independentOfThreadCount();
};

// =====================================================================
// Numeric parser
// =====================================================================

void SedSAT3Tests::parser_acceptsNumbers_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<double>("expected");
    QTest::newRow("integer") << "12" << 12.0;
    QTest::newRow("negative decimal") << "-0.5" << -0.5;
    QTest::newRow("plus sign") << "+3" << 3.0;
    QTest::newRow("exponent") << "1.5e-3" << 1.5e-3;
    QTest::newRow("upper-case exponent") << "2E+02" << 200.0;
    QTest::newRow("leading point") << ".25" << 0.25;
    QTest::newRow("trailing point") << "5." << 5.0;
    QTest::newRow("surrounding spaces") << "  42.0  " << 42.0;
}

void SedSAT3Tests::parser_acceptsNumbers()
{
    QFETCH(QString, text);
    QFETCH(double, expected);
    double value = -1;
    QString error;
    QVERIFY2(parseNumericCell(text, value, error), qPrintable(error));
    QCOMPARE(value, expected);
}

void SedSAT3Tests::parser_rejects_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<QString>("reason");
    QTest::newRow("empty") << "" << "empty";
    QTest::newRow("blank") << "   " << "empty";
    QTest::newRow("NaN") << "NaN" << "not a finite number";
    QTest::newRow("nan") << "nan" << "not a finite number";
    QTest::newRow("inf") << "inf" << "not a finite number";
    QTest::newRow("-inf") << "-inf" << "not a finite number";
    QTest::newRow("Infinity") << "Infinity" << "not a finite number";
    QTest::newRow("text") << "abc" << "not a number";
    QTest::newRow("embedded NaN") << "31NaN50" << "not a number";
    QTest::newRow("decimal comma") << "1,5" << "decimal separator";
    QTest::newRow("thousands separator") << "1,234" << "decimal separator";
    QTest::newRow("two numbers") << "1 2" << "not a number";
    QTest::newRow("overflow") << "1e400" << "too large";
    QTest::newRow("hexadecimal") << "0x10" << "not a number";
}

void SedSAT3Tests::parser_rejects()
{
    QFETCH(QString, text);
    QFETCH(QString, reason);
    double value = 7.0;
    QString error;
    QVERIFY(!parseNumericCell(text, value, error));
    QCOMPARE(value, 7.0);  // unchanged on failure
    QVERIFY2(error.contains(reason), qPrintable(error));
}

void SedSAT3Tests::parser_variantAndJson()
{
    double value = 0;
    QString error;

    // Spreadsheet cells
    QVERIFY(parseNumericValue(QVariant(3.25), value, error));
    QCOMPARE(value, 3.25);
    QVERIFY(parseNumericValue(QVariant(7), value, error));
    QCOMPARE(value, 7.0);
    QVERIFY(parseNumericValue(QVariant(QStringLiteral("8.5")), value, error));
    QCOMPARE(value, 8.5);
    QVERIFY(!parseNumericValue(QVariant(std::numeric_limits<double>::quiet_NaN()), value, error));
    QVERIFY(!parseNumericValue(QVariant(std::numeric_limits<double>::infinity()), value, error));
    QVERIFY(!parseNumericValue(QVariant(QStringLiteral("NaN")), value, error));
    QVERIFY(!parseNumericValue(QVariant(), value, error));
    QVERIFY(!parseNumericValue(QVariant(true), value, error));

    // Project-file values
    QVERIFY(parseNumericJson(QJsonValue(2.5), value, error));
    QCOMPARE(value, 2.5);
    QVERIFY(parseNumericJson(QJsonValue(QStringLiteral("4")), value, error));
    QVERIFY(!parseNumericJson(QJsonValue(QJsonValue::Null), value, error));
    QVERIFY(!parseNumericJson(QJsonValue(QJsonValue::Undefined), value, error));
    QVERIFY(!parseNumericJson(QJsonValue(true), value, error));

    // A NaN saved to a project file comes back as null and is refused
    QJsonObject saved;
    saved["Al"] = std::numeric_limits<double>::quiet_NaN();
    const QJsonObject reloaded = QJsonDocument::fromJson(QJsonDocument(saved).toJson()).object();
    QVERIFY(!parseNumericJson(reloaded["Al"], value, error));

    QVERIFY(AllFinite({1.0, 2.0}));
    QVERIFY(!AllFinite({1.0, std::numeric_limits<double>::quiet_NaN()}));
}

// =====================================================================
// Box-Cox
// =====================================================================

void SedSAT3Tests::boxcox_positiveData()
{
    ConcentrationSet data;
    for (double x : {1.0, 2.0, 4.0, 8.0, 16.0})
        data.push_back(x);
    QVERIFY(data.CanBoxCoxTransform());

    const ConcentrationSet half = data.ApplyBoxCoxTransform(0.5, false);
    for (size_t i = 0; i < data.size(); i++)
        QCOMPARE(half[i], (std::sqrt(data[i]) - 1.0) / 0.5);

    const ConcentrationSet one = data.ApplyBoxCoxTransform(1.0, false);
    for (size_t i = 0; i < data.size(); i++)
        QCOMPARE(one[i], data[i] - 1.0);

    const double lambda = data.FindOptimalBoxCoxParameter(-5.0, 5.0, 10);
    QVERIFY(std::isfinite(lambda));
    QVERIFY(lambda >= -5.0 && lambda <= 5.0);
}

void SedSAT3Tests::boxcox_zeroAndNegativeData()
{
    for (double bad : {0.0, -1.5})
    {
        ConcentrationSet data;
        for (double x : {2.0, 3.0, bad, 5.0})
            data.push_back(x);
        QVERIFY(!data.CanBoxCoxTransform());

        // Left untransformed, never log(0) or a NaN
        const ConcentrationSet transformed = data.ApplyBoxCoxTransform(0.0, false);
        for (size_t i = 0; i < data.size(); i++)
            QCOMPARE(transformed[i], data[i]);
        QCOMPARE(data.FindOptimalBoxCoxParameter(-5.0, 5.0, 10), 1.0);
    }
}

void SedSAT3Tests::boxcox_lambdaZeroAndNearZero()
{
    for (double x : {0.01, 0.5, 1.0, 3.0, 250.0})
    {
        QCOMPARE(ConcentrationSet::BoxCoxTransformValue(x, 0.0), std::log(x));

        // Near zero the transform tends to log x; the difference is about
        // lambda (log x)^2 / 2, so it must be that small and computed without
        // the cancellation of (x^lambda - 1)
        for (double lambda : {1e-12, -1e-9, 1e-6, -1e-4})
        {
            const double value = ConcentrationSet::BoxCoxTransformValue(x, lambda);
            const double log_x = std::log(x);
            const double expected = log_x + lambda * log_x * log_x / 2.0;
            QVERIFY2(std::fabs(value - expected) <= 1e-12 * std::max(1.0, std::fabs(log_x)) +
                                                      std::fabs(lambda * lambda * log_x * log_x * log_x),
                     qPrintable(QString("x=%1 lambda=%2 value=%3 expected=%4")
                                    .arg(x).arg(lambda).arg(value, 0, 'g', 17).arg(expected, 0, 'g', 17)));
        }

        // Away from zero the textbook formula is used unchanged
        QCOMPARE(ConcentrationSet::BoxCoxTransformValue(x, -0.8), (std::pow(x, -0.8) - 1.0) / -0.8);
    }
}

void SedSAT3Tests::boxcox_logLikelihoodIsFinite()
{
    ConcentrationSet data;
    for (double x : {1.2, 2.5, 3.1, 4.8, 7.7})
        data.push_back(x);
    QVERIFY(std::isfinite(data.CalculateBoxCoxLogLikelihood(0.5)));

    data.push_back(0.0);
    QVERIFY(std::isnan(data.CalculateBoxCoxLogLikelihood(0.5)));
}

// =====================================================================
// CBinary
// =====================================================================

void SedSAT3Tests::binary_indexing()
{
    CBinary binary(4);
    binary[0] = 1;
    binary[3] = 1;
    QCOMPARE(binary[0], 1);
    QCOMPARE(binary[1], 0);
    QCOMPARE(binary[3], 1);

    bool threw = false;
    try { binary[4] = 1; }
    catch (const std::out_of_range&) { threw = true; }
    QVERIFY(threw);

    // No digits at all: the old code dereferenced a null pointer here
    CBinary empty;
    threw = false;
    try { (void)empty[0]; }
    catch (const std::out_of_range&) { threw = true; }
    QVERIFY(threw);
}

// =====================================================================
// CheckNegativeElements
// =====================================================================

void SedSAT3Tests::negativeCheck_clean()
{
    SourceSinkData data;
    BuildData(data, PositiveGroups(), "Mixture");
    RecordingHost host;
    Conductor conductor(&host);
    conductor.SetData(&data);
    QVERIFY(conductor.CheckNegativeElements(&data));
    QVERIFY(host.warnings.empty());

    std::map<std::string, std::vector<std::string>> none = {{"M1", {}}};
    QVERIFY(conductor.CheckNegativeElements(none));
}

void SedSAT3Tests::negativeCheck_negative()
{
    GroupData groups = PositiveGroups();
    groups["Forest"]["F2"]["Fe"] = -0.1;
    SourceSinkData data;
    BuildData(data, groups, "Mixture");
    RecordingHost host;
    Conductor conductor(&host);
    conductor.SetData(&data);
    QVERIFY(!conductor.CheckNegativeElements(&data));
    QVERIFY(host.Warned("'Fe'"));
    QVERIFY(host.Warned("'Forest'"));

    std::map<std::string, std::vector<std::string>> some = {{"M1", {"Fe"}}};
    QVERIFY(!conductor.CheckNegativeElements(some));
}

void SedSAT3Tests::negativeCheck_sourceAfterTarget()
{
    // "Pasture" sorts after the target "Mixture"; the check used to skip
    // the last group alphabetically and so missed it
    GroupData groups = PositiveGroups();
    groups["Pasture"] = {{"P1", {{"Al", 6.0}, {"Fe", 0.0}}}, {"P2", {{"Al", 6.4}, {"Fe", 3.0}}}};
    SourceSinkData data;
    BuildData(data, groups, "Mixture");
    RecordingHost host;
    Conductor conductor(&host);
    conductor.SetData(&data);
    QVERIFY(!conductor.CheckNegativeElements(&data));
    QVERIFY(host.Warned("'Pasture'"));
}

// =====================================================================
// Include all elements
// =====================================================================

void SedSAT3Tests::includeAll_dataLevel()
{
    SourceSinkData data;
    BuildData(data, PositiveGroups(), "Mixture");
    data.IncludeExcludeAllElements(false);
    for (const auto& element : *data.GetElementInformation())
        QVERIFY(!element.second.include_in_analysis);
    data.IncludeExcludeAllElements(true);
    for (const auto& element : *data.GetElementInformation())
        QVERIFY(element.second.include_in_analysis);
}

void SedSAT3Tests::includeAll_checkBox()
{
    SourceSinkData data;
    BuildData(data, PositiveGroups(), "Mixture");

    FormElementInformation form;
    ElementTableModel model(&data);
    form.setModel(&model);
    QCheckBox* check_box = form.findChild<QCheckBox*>("Include_Exclude_All_checkBox");
    QVERIFY(check_box != nullptr);

    // The label never changes, and the box shows the data's state
    QCOMPARE(check_box->text(), QStringLiteral("Include all elements"));
    QVERIFY(check_box->isChecked());

    // Unchecking excludes every element; checking includes every element
    check_box->setChecked(false);
    for (const auto& element : *data.GetElementInformation())
        QVERIFY(!element.second.include_in_analysis);
    QCOMPARE(check_box->text(), QStringLiteral("Include all elements"));

    check_box->setChecked(true);
    for (const auto& element : *data.GetElementInformation())
        QVERIFY(element.second.include_in_analysis);

    // Excluding one element in the table clears the box without excluding the rest
    model.setData(model.index(0, 4), false, Qt::EditRole);  // what the table's check box sends
    QVERIFY(!check_box->isChecked());
    model.setData(model.index(0, 4), true, Qt::EditRole);
    QVERIFY(check_box->isChecked());
    model.setData(model.index(0, 4), QStringLiteral("No"), Qt::EditRole);  // text also works
    QVERIFY(!check_box->isChecked());
    int included = 0;
    for (const auto& element : *data.GetElementInformation())
        included += element.second.include_in_analysis ? 1 : 0;
    QCOMPARE(included, int(data.GetElementInformation()->size()) - 1);
}

// =====================================================================
// Distribution fitting
// =====================================================================

void SedSAT3Tests::distributionFitting_degenerateSets()
{
    // Empty, single and constant sets used to loop forever (zero step) or
    // index series that did not exist; they must return promptly
    std::vector<std::vector<double>> sets = {{}, {3.0}, {2.0, 2.0, 2.0}};
    for (const auto& values : sets)
    {
        ConcentrationSet data;
        for (double x : values)
            data.push_back(x);
        QVERIFY(!data.CanFitDistribution());
        const CMBTimeSeriesSet pdf = data.CreateFittedDistribution(distribution_type::normal);
        QVERIFY(pdf.Contains("Observed"));
        QVERIFY(!pdf.Contains("Fitted"));
        const CMBTimeSeriesSet cdf = data.CreateCDFComparison(distribution_type::lognormal);
        QVERIFY(!cdf.Contains("Fitted"));
        QVERIFY(std::isnan(data.CalculateKolmogorovSmirnovStatistic(distribution_type::normal)));
    }

    // A proper set gives named, non-empty series
    ConcentrationSet data;
    for (double x : {1.0, 2.0, 2.5, 3.5, 5.0})
        data.push_back(x);
    const CMBTimeSeriesSet pdf = data.CreateFittedDistribution(distribution_type::lognormal);
    QVERIFY(pdf.Contains("Observed"));
    QVERIFY(pdf.Contains("Fitted"));
    QCOMPARE(int(pdf["Observed"].size()), 5);
    QVERIFY(pdf["Fitted"].size() > 10);
}

void SedSAT3Tests::distributionFitting_conductor()
{
    std::map<std::string, std::string> arguments = {
        {"Constituent", "Al"},
        {"Source/Target group", "Agriculture"},
        {"Box-cox transformation", "false"},
        {"OM and Size Correct based on target sample", ""},
        {"Use only selected samples", "false"},
    };

    // Normal case: both fitted distributions, no exception
    {
        SourceSinkData data;
        BuildData(data, PositiveGroups(), "Mixture");
        RecordingHost host;
        Conductor conductor(&host);
        conductor.SetData(&data);
        QVERIFY(conductor.Execute("DF", arguments));
        const QJsonObject results = ResultsJson(conductor);
        QCOMPARE(results.size() >= 2, true);
    }

    // A group with one sample: refused with a message, not a crash
    {
        GroupData groups = PositiveGroups();
        groups["Agriculture"] = {{"A1", {{"Al", 8.1}, {"Fe", 4.0}}}};
        SourceSinkData data;
        BuildData(data, groups, "Mixture");
        RecordingHost host;
        Conductor conductor(&host);
        conductor.SetData(&data);
        QVERIFY(!conductor.Execute("DF", arguments));
        QVERIFY(host.Warned("at least two samples"));
    }

    // An unknown constituent: refused with a message
    {
        SourceSinkData data;
        BuildData(data, PositiveGroups(), "Mixture");
        RecordingHost host;
        Conductor conductor(&host);
        conductor.SetData(&data);
        std::map<std::string, std::string> unknown = arguments;
        unknown["Constituent"] = "Zr";
        QVERIFY(!conductor.Execute("DF", unknown));
        QVERIFY(host.Warned("'Zr'"));
    }
}

// =====================================================================
// Random numbers and MCMC
// =====================================================================

void SedSAT3Tests::random_streamsAreReproducible()
{
    RandomStream a(42, 0), b(42, 0), other_stream(42, 1), other_seed(43, 0);
    bool differs_by_stream = false, differs_by_seed = false;
    for (int i = 0; i < 1000; i++)
    {
        const double x = a.Uniform();
        QCOMPARE(x, b.Uniform());
        QVERIFY(x >= 0.0 && x < 1.0);
        differs_by_stream |= (x != other_stream.Uniform());
        differs_by_seed |= (x != other_seed.Uniform());
        const double n = a.StandardNormal();
        QCOMPARE(n, b.StandardNormal());
        QVERIFY(std::isfinite(n));
    }
    QVERIFY(differs_by_stream);
    QVERIFY(differs_by_seed);

    // The values are fixed by the C++ standard's mt19937_64 and seed_seq and
    // this file's transforms, so they are the same on every platform
    RandomStream fixed(20261016, 0);
    const double first = fixed.Uniform();
    RandomStream again(20261016, 0);
    QCOMPARE(again.Uniform(), first);
}

void SedSAT3Tests::random_seedParsing()
{
    std::uint64_t seed = 0;
    QVERIFY(ParseRandomSeed("42", seed));
    QCOMPARE(seed, std::uint64_t(42));
    QVERIFY(ParseRandomSeed(" 18446744073709551615 ", seed));
    QCOMPARE(seed, std::uint64_t(18446744073709551615ULL));
    QVERIFY(ParseRandomSeed("", seed));      // drawn
    QVERIFY(!ParseRandomSeed("-1", seed));
    QVERIFY(!ParseRandomSeed("1.5", seed));
    QVERIFY(!ParseRandomSeed("abc", seed));
    QVERIFY(!ParseRandomSeed("18446744073709551616", seed));  // 2^64
}

namespace {

/// Runs the test MCMC (the "Test CMB Bayesian" model) and returns its samples
std::vector<std::vector<double>> RunTestMCMC(std::uint64_t seed, int threads, const QString& folder)
{
    CMCMC<TestMCMC> mcmc;
    TestMCMC model;
    mcmc.Model = &model;
    model.InitializeParametersObservations({0.1, 1.0}, {1.0, 10.0});
    mcmc.SetProperty("number_of_samples", "800");
    mcmc.SetProperty("number_of_chains", "8");
    mcmc.SetProperty("number_of_burnout_samples", "100");
    mcmc.SetProperty("number_of_threads", std::to_string(threads));
    mcmc.SetRandomSeed(seed);

    CMBTimeSeriesSet samples;
    mcmc.initialize(&samples, true);
    NullProgressReporter progress;
    mcmc.step(8, 800, (folder + "/samples.txt").toStdString(), &samples, &progress);

    std::vector<std::vector<double>> values;
    for (size_t series = 0; series < samples.size(); series++)
    {
        std::vector<double> column;
        for (size_t i = 0; i < samples[series].size(); i++)
            column.push_back(samples[series].getValue(i));
        values.push_back(column);
    }
    return values;
}

} // namespace

void SedSAT3Tests::mcmc_sameSeedSameResult()
{
    QTemporaryDir folder;
    QVERIFY(folder.isValid());
    const auto first = RunTestMCMC(42, 4, folder.path());
    const auto second = RunTestMCMC(42, 4, folder.path());
    QVERIFY(!first.empty() && !first[0].empty());
    QVERIFY(first == second);

    // Through the analysis interface, with the seed recorded in the results name
    SourceSinkData data;
    RecordingHost host;
    Conductor conductor(&host);
    conductor.SetData(&data);
    conductor.SetWorkingFolder(folder.path());
    std::map<std::string, std::string> arguments = {
        {"Number of chains", "8"}, {"Number of samples", "1000"},
        {"Samples to be discarded (burnout)", "100"},
        {"samples_file_name", "test_samples.txt"}, {"Random seed", "1234"}};
    QVERIFY(conductor.Execute("Test CMB Bayesian", arguments));
    Results* named = conductor.GetResults();
    QVERIFY(QString::fromStdString(named->GetName()).contains("random seed 1234"));
    delete named;
    const QJsonObject run1 = ResultsJson(conductor);
    QVERIFY(conductor.Execute("Test CMB Bayesian", arguments));
    const QJsonObject run2 = ResultsJson(conductor);
    QCOMPARE(run1, run2);

    QFile samples(folder.path() + "/test_samples.txt");
    QVERIFY(samples.open(QIODevice::ReadOnly));
    QVERIFY(samples.readLine().startsWith("# SedSAT3 MCMC random seed: 1234"));
}

void SedSAT3Tests::mcmc_differentSeedDifferentResult()
{
    QTemporaryDir folder;
    QVERIFY(folder.isValid());
    QVERIFY(RunTestMCMC(42, 4, folder.path()) != RunTestMCMC(43, 4, folder.path()));
}

void SedSAT3Tests::mcmc_independentOfThreadCount()
{
    QTemporaryDir folder;
    QVERIFY(folder.isValid());
    const auto one_thread = RunTestMCMC(7, 1, folder.path());
    QVERIFY(one_thread == RunTestMCMC(7, 3, folder.path()));
    QVERIFY(one_thread == RunTestMCMC(7, 8, folder.path()));
}

int main(int argc, char* argv[])
{
    // The check-box test creates a widget; no display is needed
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication application(argc, argv);
    SedSAT3Tests tests;
    return QTest::qExec(&tests, argc, argv);
}

#include "tst_sedsat3.moc"
