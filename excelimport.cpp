// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#include "excelimport.h"
#include "numericparsing.h"
#include "sourcesinkdata.h"
#include "xlsxdocument.h"
#include "xlsxcellrange.h"

#include <QSet>

using namespace QXlsx;

namespace {

QString CellText(Document& document, int row, int col)
{
    auto cell = document.cellAt(row, col);
    return cell ? cell->readValue().toString().trimmed() : QString();
}

bool Refuse(SourceSinkData& data, ExcelImportError& error, const QString& title, const QString& message)
{
    data.Clear();
    error.title = title;
    error.message = message;
    return false;
}

} // namespace

bool ImportExcelSheets(Document& document, const QStringList& sheets,
                       SourceSinkData& data, ExcelImportError& error)
{
    data.Clear();

    // Validate element names are consistent across all sheets
    QList<QStringList> elementNamesBySheet;

    for (int sheetIdx = 0; sheetIdx < sheets.count(); ++sheetIdx)
    {
        document.selectSheet(sheets[sheetIdx]);
        const int lastColumn = document.dimension().lastColumn();
        QStringList elementNames;

        // Read element names from row 1, starting at column 2
        int col = 2;
        while (true)
        {
            const QString elemName = CellText(document, 1, col);
            if (elemName.isEmpty())
                break;
            elementNames.append(elemName);
            col++;
        }

        // An empty header cell used to end the header silently, dropping
        // every column after it
        for (int later = col + 1; later <= lastColumn; ++later)
        {
            if (!CellText(document, 1, later).isEmpty())
            {
                return Refuse(data, error, QObject::tr("Missing Column Name"),
                    QObject::tr("The header of sheet '%1' has an empty cell in column %2, "
                                "but column %3 has a name. Every constituent column needs a name "
                                "in row 1, with no gaps.")
                    .arg(sheets[sheetIdx]).arg(col).arg(later));
            }
        }

        // Verify element names match across sheets
        if (sheetIdx > 0 && elementNames != elementNamesBySheet.last())
        {
            return Refuse(data, error, QObject::tr("Element Mismatch"),
                QObject::tr("Element names and their order must be identical in all sheets.\n\n"
                            "Mismatch found between:\n  '%1'\n  '%2'")
                .arg(sheets[sheetIdx - 1]).arg(sheets[sheetIdx]));
        }

        elementNamesBySheet.append(elementNames);
    }

    // Read sample data from each sheet
    for (int sheetIdx = 0; sheetIdx < sheets.count(); ++sheetIdx)
    {
        document.selectSheet(sheets[sheetIdx]);
        const int lastRow = document.dimension().lastRow();

        Elemental_Profile_Set* profileSet = data.AppendSampleSet(sheets[sheetIdx].toStdString());
        QSet<QString> sampleNames;

        // Read samples starting from row 2
        int row = 2;
        while (true)
        {
            // The name is kept as written (untrimmed), as it always has been
            auto sampleCell = document.cellAt(row, 1);
            const QString sampleName = sampleCell ? sampleCell->readValue().toString() : QString();
            if (sampleName.isEmpty())
                break;

            if (sampleNames.contains(sampleName))
            {
                return Refuse(data, error, QObject::tr("Duplicate Sample"),
                    QObject::tr("Sample name '%1' appears more than once in sheet '%2' "
                                "(again in row %3). Sample names must be unique within a sheet.")
                    .arg(sampleName).arg(sheets[sheetIdx]).arg(row));
            }
            sampleNames.insert(sampleName);

            Elemental_Profile elementalProfile;

            // Read element values for this sample
            for (int col = 0; col < elementNamesBySheet[0].count(); ++col)
            {
                auto dataCell = document.cellAt(row, col + 2);

                if (!dataCell)
                {
                    return Refuse(data, error, QObject::tr("Empty Cell"),
                        QObject::tr("Empty cell found in:\n"
                                    "Sheet: %1\n"
                                    "Row: %2\n"
                                    "Column: %3")
                        .arg(sheets[sheetIdx]).arg(row).arg(col + 2));
                }

                // Every value must be a finite number; NaN and Inf are refused
                // like any other non-number (QVariant::toDouble accepted them)
                double value = 0;
                QString reason;
                if (!parseNumericValue(dataCell->readValue(), value, reason))
                {
                    return Refuse(data, error, QObject::tr("Invalid Data"),
                        QObject::tr("Non-numerical value found:\n"
                                    "Sheet: %1\n"
                                    "Row: %2\n"
                                    "Column: %3\n"
                                    "Value: '%4'\n"
                                    "(%5)")
                        .arg(sheets[sheetIdx]).arg(row).arg(col + 2)
                        .arg(dataCell->readValue().toString()).arg(reason));
                }

                elementalProfile.AppendElement(elementNamesBySheet[0][col].toStdString(), value);
            }

            profileSet->AppendProfile(sampleName.toStdString(), elementalProfile);
            row++;
        }

        // A row with no sample name used to end the sheet silently, dropping
        // every sample below it
        for (int later = row + 1; later <= lastRow; ++later)
        {
            if (!CellText(document, later, 1).isEmpty())
            {
                return Refuse(data, error, QObject::tr("Missing Sample Name"),
                    QObject::tr("Sheet '%1' has no sample name in row %2, but row %3 has one. "
                                "Remove the empty row or give the sample a name.")
                    .arg(sheets[sheetIdx]).arg(row).arg(later));
            }
        }
    }

    // Finalize data collection
    data.PopulateElementInformation();
    data.PopulateElementDistributions();
    data.AssignAllDistributions();
    return true;
}
