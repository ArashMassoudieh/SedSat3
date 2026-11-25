// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#ifndef TARGETSHEET_H
#define TARGETSHEET_H

#include <QStringList>

/**
 * @file targetsheet.h
 * @brief Which sheet of a workbook holds the target samples
 *
 * Only one group in a project is the target; every other group is a source
 * group. Which one it is cannot be read from the data, so the user says so
 * when importing a workbook, and the answer is stored in the project file.
 *
 * Getting it wrong is costly and quiet. A source group put in the target's
 * place is left out of every source list in the program, because those
 * lists hold the source groups only, and the unmixing then runs against its
 * samples instead of the real target samples. Both look like the program
 * misbehaving rather than like a wrong setting.
 */

/**
 * @brief Index of the sheet whose name says it holds the target samples
 *
 * Used to preselect a row in the import dialog. A workbook's sheet order
 * says nothing about which sheet is the target, so the sheet is looked for
 * by name instead: "target", "sink" or "mixture", matched as a substring,
 * ignoring case and surrounding spaces.
 *
 * Returns -1 unless exactly one sheet matches, so an ambiguous workbook
 * (say, "Target" and "Target old") preselects nothing and the choice stays
 * with the user: no guess is better than a wrong one.
 *
 * @param sheets Sheet names, in the order they are shown
 * @return Index into @p sheets, or -1 if no single sheet matches
 */
int GuessTargetSheet(const QStringList& sheets);

#endif // TARGETSHEET_H
