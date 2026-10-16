// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#include "numericparsing.h"

#include <QJsonValue>
#include <QRegularExpression>
#include <QVariant>

bool parseNumericCell(const QString& text, double& out, QString& error)
{
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty())
    {
        error = QStringLiteral("the cell is empty");
        return false;
    }

    // QString::toDouble accepts "nan", "inf" and "infinity", which are not data
    static const QRegularExpression number(
        QStringLiteral("^[+-]?(\\d+\\.?\\d*|\\.\\d+)([eE][+-]?\\d+)?$"));
    if (!number.match(trimmed).hasMatch())
    {
        static const QRegularExpression non_finite(
            QStringLiteral("^[+-]?(nan|inf|infinity)$"), QRegularExpression::CaseInsensitiveOption);
        static const QRegularExpression comma_number(QStringLiteral("^[+-]?\\d*,\\d+$"));
        if (non_finite.match(trimmed).hasMatch())
            error = QStringLiteral("'%1' is not a finite number").arg(trimmed);
        else if (comma_number.match(trimmed).hasMatch())
            error = QStringLiteral("'%1' is not a number; use a point as the decimal separator "
                                   "and no thousands separator").arg(trimmed);
        else
            error = QStringLiteral("'%1' is not a number").arg(trimmed);
        return false;
    }

    bool ok = false;
    const double value = trimmed.toDouble(&ok);
    if (!ok || !std::isfinite(value))
    {
        error = QStringLiteral("'%1' is too large to represent").arg(trimmed);
        return false;
    }
    out = value;
    return true;
}

bool parseNumericValue(const QVariant& value, double& out, QString& error)
{
    switch (value.typeId())
    {
    case QMetaType::Double:
    case QMetaType::Float:
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
    {
        const double number = value.toDouble();
        if (!std::isfinite(number))
        {
            error = QStringLiteral("'%1' is not a finite number").arg(value.toString());
            return false;
        }
        out = number;
        return true;
    }
    case QMetaType::QString:
        return parseNumericCell(value.toString(), out, error);
    default:
        if (!value.isValid() || value.isNull())
            error = QStringLiteral("the cell is empty");
        else
            error = QStringLiteral("'%1' is not a number").arg(value.toString());
        return false;
    }
}

bool parseNumericJson(const QJsonValue& value, double& out, QString& error)
{
    if (value.isDouble())
    {
        const double number = value.toDouble();
        if (!std::isfinite(number))
        {
            error = QStringLiteral("the value is not a finite number");
            return false;
        }
        out = number;
        return true;
    }
    if (value.isString())
        return parseNumericCell(value.toString(), out, error);
    if (value.isNull())
        error = QStringLiteral("the value is missing (NaN and infinity are saved as null)");
    else if (value.isUndefined())
        error = QStringLiteral("the value is missing");
    else
        error = QStringLiteral("the value is not a number");
    return false;
}
