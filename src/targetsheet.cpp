// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#include "targetsheet.h"

int GuessTargetSheet(const QStringList& sheets)
{
    // The names that say "these are the target samples". Matched as
    // substrings so "Target samples" and "Sink_2013" are recognised too.
    static const QStringList kTargetWords = { "target", "sink", "mixture" };

    int found = -1;
    for (int i = 0; i < sheets.count(); ++i)
    {
        const QString name = sheets[i].trimmed().toLower();
        bool matches = false;
        for (const QString& word : kTargetWords)
            matches = matches || name.contains(word);

        if (!matches)
            continue;

        if (found >= 0)
            return -1; // More than one candidate: let the user choose

        found = i;
    }

    return found;
}
