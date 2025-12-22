#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextBrowser>
#include <QPushButton>
#include <QPixmap>

/**
 * @class AboutDialog
 * @brief Professional about dialog for SedSat3 application
 *
 * Displays application information including version, authors, description,
 * and copyright information. Built entirely in code without .ui file.
 */
class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the about dialog
     * @param parent Parent widget
     */
    explicit AboutDialog(QWidget* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~AboutDialog() override = default;

    /**
     * @brief Sets the application version
     * @param version Version string (e.g., "1.1.3")
     */
    void setVersion(const QString& version);

    /**
     * @brief Sets the build date
     * @param date Date string (e.g., "December 20, 2025")
     */
    void setBuildDate(const QString& date);

    /**
     * @brief Appends additional text to the description area
     * @param text Text to append
     */
    void appendText(const QString& text);

    /**
     * @brief Clears all text from the description area
     */
    void clearText();

private:
    /**
     * @brief Sets up the dialog UI
     */
    void setupUI();

    /**
     * @brief Creates the header section with logo and title
     * @return Header widget
     */
    QWidget* createHeader();

    /**
     * @brief Creates the information section with version and authors
     * @return Information widget
     */
    QWidget* createInfoSection();

    /**
     * @brief Creates the description text browser
     * @return Text browser widget
     */
    QTextBrowser* createDescriptionSection();

    /**
     * @brief Creates the footer with copyright and buttons
     * @return Footer widget
     */
    QWidget* createFooter();

private:
    QLabel* versionLabel;
    QLabel* buildDateLabel;
    QTextBrowser* descriptionBrowser;
};

#endif // ABOUTDIALOG_H