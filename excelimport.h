// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#ifndef EXCELIMPORT_H
#define EXCELIMPORT_H

#include <QString>
#include <QStringList>

class SourceSinkData;
namespace QXlsx { class Document; }

/**
 * @brief Why an Excel import was refused
 */
struct ExcelImportError
{
    QString title;    ///< Short heading for a message box, e.g. "Invalid Data"
    QString message;  ///< What was found and where (sheet, row, column, value)
};

/**
 * @brief Reads the selected sheets of a workbook into a dataset
 *
 * Each sheet is one sample group. Row 1 holds the constituent names from
 * column 2 on; each following row is a sample, with its name in column 1.
 * The import is refused, with the location, if:
 * - the constituent names or their order differ between sheets;
 * - a header cell is empty while later header cells are not (the columns
 *   after it would otherwise be ignored);
 * - a sample name is empty while later rows hold samples (the rows below it
 *   would otherwise be ignored), or a sample name repeats within a sheet;
 * - a data cell is empty or not a finite number (see numericparsing.h).
 *
 * Kept free of dialogs so the same code runs in the GUI and in the tests.
 *
 * @param document Loaded workbook
 * @param sheets Sheets to read, in order
 * @param data Filled with the groups; left cleared if the import is refused
 * @param error Set if the import is refused
 * @return true on success
 */
bool ImportExcelSheets(QXlsx::Document& document, const QStringList& sheets,
                       SourceSinkData& data, ExcelImportError& error);

#endif // EXCELIMPORT_H
