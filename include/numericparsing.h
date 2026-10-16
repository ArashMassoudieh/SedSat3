// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#pragma once

#include <QString>
#include <cmath>
#include <vector>

class QVariant;
class QJsonValue;

/**
 * @file numericparsing.h
 * @brief The one place where data values entering SedSAT3 are checked
 *
 * Every concentration, ratio and option read from an Excel file, a project
 * file, the command line or the element table goes through these functions,
 * so the rest of the program can rely on every stored value being a finite
 * number. A value that fails is not stored: the import or load stops and
 * reports where the value is (sheet, row and column, or group, sample and
 * constituent) and why it was refused. This is the same treatment empty and
 * non-numeric Excel cells have always had; SedSAT3 has no missing-data
 * value.
 *
 * Accepted: an optional sign, digits with '.' as the decimal separator, and
 * an optional exponent, with surrounding spaces ignored ("12", "-0.5",
 * "1.5e-3"). Refused: empty text; NaN, Inf and Infinity in any case; numbers
 * too large for a double; anything else, including "1,5" (a decimal comma)
 * and "1,234" (a thousands separator), which are reported as such.
 */

/**
 * @brief Parses text as a finite number
 * @param text Text to parse
 * @param out Set to the value if the text is accepted; unchanged otherwise
 * @param error Set to the reason if the text is refused
 * @return true if accepted
 */
bool parseNumericCell(const QString& text, double& out, QString& error);

/**
 * @brief Parses a spreadsheet cell value as a finite number
 *
 * Numeric cells are accepted if finite; text cells go through
 * parseNumericCell(); anything else (dates, booleans, errors) is refused.
 */
bool parseNumericValue(const QVariant& value, double& out, QString& error);

/**
 * @brief Parses a project-file value as a finite number
 *
 * JSON numbers are accepted if finite; strings go through
 * parseNumericCell(); null, which is how Qt writes NaN and infinity to a
 * project file, and any other type are refused.
 */
bool parseNumericJson(const QJsonValue& value, double& out, QString& error);

/// Whether every value is finite. Data imported through the functions above always is.
inline bool AllFinite(const std::vector<double>& values)
{
    for (double value : values)
        if (!std::isfinite(value))
            return false;
    return true;
}
