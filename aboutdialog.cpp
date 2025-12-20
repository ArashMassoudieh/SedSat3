#include "aboutdialog.h"
#include <QApplication>
#include <QScreen>
#include <QFont>
#include <QFrame>

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent),
    versionLabel(nullptr),
    buildDateLabel(nullptr),
    descriptionBrowser(nullptr)
{
    setupUI();
}

void AboutDialog::setupUI()
{
    setWindowTitle(tr("About SedSat3"));
    setModal(true);
    setMinimumSize(500, 400);
    resize(600, 500);

    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Add sections
    mainLayout->addWidget(createHeader());
    mainLayout->addWidget(createInfoSection());
    mainLayout->addWidget(createDescriptionSection(), 1); // Stretch factor 1
    mainLayout->addWidget(createFooter());

    setLayout(mainLayout);
}

QWidget* AboutDialog::createHeader()
{
    QWidget* headerWidget = new QWidget(this);
    headerWidget->setStyleSheet(
        "QWidget { "
        "background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #2c3e50, stop:1 #34495e); "
        "}"
    );
    headerWidget->setMinimumHeight(100);

    QVBoxLayout* headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setContentsMargins(20, 20, 20, 20);

    // Application title
    QLabel* titleLabel = new QLabel(tr("SedSat3"), headerWidget);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("QLabel { color: white; }");
    titleLabel->setAlignment(Qt::AlignCenter);

    // Subtitle
    QLabel* subtitleLabel = new QLabel(
        tr("Sediment Source Apportionment Tool"),
        headerWidget
    );
    QFont subtitleFont = subtitleLabel->font();
    subtitleFont.setPointSize(11);
    subtitleLabel->setFont(subtitleFont);
    subtitleLabel->setStyleSheet("QLabel { color: #ecf0f1; }");
    subtitleLabel->setAlignment(Qt::AlignCenter);

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(subtitleLabel);

    return headerWidget;
}

QWidget* AboutDialog::createInfoSection()
{
    QWidget* infoWidget = new QWidget(this);
    infoWidget->setStyleSheet("QWidget { background-color: white; }");

    QVBoxLayout* infoLayout = new QVBoxLayout(infoWidget);
    infoLayout->setContentsMargins(20, 20, 20, 10);
    infoLayout->setSpacing(10);

    // Version
    versionLabel = new QLabel(tr("Version: "), infoWidget);
    QFont boldFont = versionLabel->font();
    boldFont.setPointSize(10);
    versionLabel->setFont(boldFont);

    // Build date
    buildDateLabel = new QLabel(tr("Build Date: "), infoWidget);
    buildDateLabel->setFont(boldFont);

    // Authors section
    QLabel* authorsLabel = new QLabel(tr("<b>Authors:</b>"), infoWidget);
    authorsLabel->setFont(boldFont);

    QLabel* authorsText = new QLabel(
        tr("Arash Massoudieh<br>"
            "Allen Gellis<br>"
            "Cara Peterman"),
        infoWidget
    );
    authorsText->setIndent(20);
    authorsText->setTextFormat(Qt::RichText);

    // Description
    QLabel* descriptionLabel = new QLabel(
        tr("<b>Description:</b>"),
        infoWidget
    );
    descriptionLabel->setFont(boldFont);

    infoLayout->addWidget(versionLabel);
    infoLayout->addWidget(buildDateLabel);
    infoLayout->addSpacing(10);
    infoLayout->addWidget(authorsLabel);
    infoLayout->addWidget(authorsText);
    infoLayout->addSpacing(10);
    infoLayout->addWidget(descriptionLabel);

    return infoWidget;
}

QTextBrowser* AboutDialog::createDescriptionSection()
{
    descriptionBrowser = new QTextBrowser(this);
    descriptionBrowser->setFrameStyle(QFrame::NoFrame);
    descriptionBrowser->setStyleSheet(
        "QTextBrowser { "
        "background-color: white; "
        "border: none; "
        "padding-left: 20px; "
        "padding-right: 20px; "
        "}"
    );
    descriptionBrowser->setOpenExternalLinks(true);

    // Default description
    descriptionBrowser->setHtml(
        tr("<p>SedSat3 is a comprehensive tool for sediment source apportionment "
            "analysis using advanced statistical methods including Chemical Mass "
            "Balance (CMB) modeling, Bayesian inference, MCMC sampling, and "
            "multivariate statistical techniques.</p>"
            "<p>This software enables researchers to identify and quantify the "
            "contributions of different sediment sources to receptor sites, "
            "supporting environmental pollution research and watershed management.</p>")
    );

    return descriptionBrowser;
}

QWidget* AboutDialog::createFooter()
{
    QWidget* footerWidget = new QWidget(this);
    footerWidget->setStyleSheet("QWidget { background-color: #f8f9fa; }");

    QVBoxLayout* footerLayout = new QVBoxLayout(footerWidget);
    footerLayout->setContentsMargins(20, 15, 20, 15);
    footerLayout->setSpacing(10);

    // Copyright
    QLabel* copyrightLabel = new QLabel(
        tr("© 2025 SedSat3 Development Team"),
        footerWidget
    );
    copyrightLabel->setAlignment(Qt::AlignCenter);
    QFont copyrightFont = copyrightLabel->font();
    copyrightFont.setPointSize(9);
    copyrightLabel->setFont(copyrightFont);
    copyrightLabel->setStyleSheet("QLabel { color: #6c757d; }");

    // Close button
    QPushButton* closeButton = new QPushButton(tr("Close"), footerWidget);
    closeButton->setMinimumWidth(100);
    closeButton->setStyleSheet(
        "QPushButton { "
        "background-color: #3498db; "
        "color: white; "
        "border: none; "
        "padding: 8px 16px; "
        "border-radius: 4px; "
        "font-weight: bold; "
        "} "
        "QPushButton:hover { "
        "background-color: #2980b9; "
        "} "
        "QPushButton:pressed { "
        "background-color: #21618c; "
        "}"
    );

    connect(closeButton, &QPushButton::clicked, this, &AboutDialog::accept);

    // Button layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();

    footerLayout->addWidget(copyrightLabel);
    footerLayout->addLayout(buttonLayout);

    return footerWidget;
}

void AboutDialog::setVersion(const QString& version)
{
    if (versionLabel)
    {
        versionLabel->setText(tr("Version: %1").arg(version));
    }
}

void AboutDialog::setBuildDate(const QString& date)
{
    if (buildDateLabel)
    {
        buildDateLabel->setText(tr("Build Date: %1").arg(date));
    }
}

void AboutDialog::appendText(const QString& text)
{
    if (descriptionBrowser)
    {
        descriptionBrowser->append(text);
    }
}

void AboutDialog::clearText()
{
    if (descriptionBrowser)
    {
        descriptionBrowser->clear();
    }
}