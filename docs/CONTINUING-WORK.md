# Continuing work — handover notes

Written 2026-10-01. State of the branch as of commit `1d751a4`.

These notes exist so the work can be picked up on another machine, in
particular a Windows machine, without re-deriving what was already found.
Everything below was verified by building and running unless it says otherwise.

---

## 1. Where things stand

### Pushed to GitHub `main` (`1d751a4`)

```
1d751a4  Merge branch 'fix-log-axis-crash'
83fedcb  Finish loading a project before running analyses on it
238dd69  Check scripts against the form definitions and let them configure a project
c79f71d  Add a command line build that runs analyses from a script
004af10  Run analyses against a host interface instead of the main window
586b4ae  Report completion when Levenberg-Marquardt finishes
acfc109  Attach plotted series to the axis that was actually created
```

GitHub remote: `https://github.com/ArashMassoudieh/SedSat3.git`. Note that the
remote named `origin` is the repository's old name, `CMBSource`; GitHub
redirects, so both names reach the same repository.

### Not merged

| branch | commit | what it is |
|---|---|---|
| `fix-isotope-base-element-validation` | `b58dd50` | refuses to run, with a readable message, when an included isotope's base element is not itself included |
| `add-gitlab-pages-docs` | `61d48b4` | `.gitlab-ci.yml` with a `pages` job; already pushed to GitLab, waiting on Cara to merge |

### GitLab

`https://code.usgs.gov/cphipps/SedimentSourceAssessmentTool3` — the repository
under review. Its `main` is a protected branch and the account has Developer
access, so **nothing can be pushed to it directly**; changes go through a merge
request. Git over SSH does not work (the host is behind Cloudflare and port 22
is closed) — HTTPS with a personal access token is the only route, and the
account has 2FA so the account password will not work.

GitLab `main` is at `ce7bc1d` and does **not** yet have any of the fixes above.

---

## 2. Getting this onto the Windows machine

If the repository is already cloned there, `main` needs to be brought up to
`1d751a4`, which is where the CLI lives:

```
git checkout main
git pull origin main
git submodule update --init --recursive
```

The submodule step is not optional. `Utilities` and `thirdparty/QXlsx` are
submodules, and the CLI build needs `Utilities` — without it the build fails on
missing headers rather than saying anything useful.

If `git pull` complains that the remote has moved, that is expected: the remote
named `origin` is the repository's old name, `CMBSource`, and GitHub redirects
to `SedSat3`. It still works. To point it at the current name:

```
git remote set-url origin https://github.com/ArashMassoudieh/SedSat3.git
```

For a fresh clone:

```
git clone --recurse-submodules https://github.com/ArashMassoudieh/SedSat3.git
```

Authentication is a token, not a password — GitHub stopped accepting passwords
over HTTPS. Confirm what arrived:

```
git log --oneline -3
```

The top commit should be `1d751a4 Merge branch 'fix-log-axis-crash'`, and
`SedSat3CLI.pro` and the `cli/` directory should both be present.

The two unmerged branches in section 1 are **not** on GitHub — they exist only
on the Linux machine. If they are needed on Windows they have to be pushed
first.

---

## 3. Building

### Both builds need `qmake6`, not `qmake`

On the Linux machine, plain `qmake` resolves to Qt 5 and fails immediately.
Windows will differ; use the `qmake.exe` from the Qt 6 kit.

### GUI

```
qmake6 SedSat3.pro
make -j8
```

**Known blocker on Linux:** the link fails. All 84 of the project's own objects
compile, but `thirdparty/QXlsx/libQXlsx.a` is a **prebuilt static library
committed to the repository, dated March 2025**, and its Qt symbols no longer
match the installed Qt 6.4.2 — hundreds of undefined references to things like
`QString::mid(long long, long long) const &` and the `QAnyStringView` overloads.

It needs rebuilding from the submodule rather than using the committed binary.
On macOS the `.pro` already compiles QXlsx inline via
`include(thirdparty/QXlsx/QXlsx/QXlsx.pri)`; doing the same on Linux and Windows
would remove the problem. A committed binary in a source release is also
something the USGS code review is likely to raise.

### CLI

```
qmake6 SedSat3CLI.pro
make -j8
```

This one builds and links cleanly. `SedSat3CLI.pro` lists the analysis half of
`SedSat3.pro` with every interface file left out. Two things about it are worth
knowing:

- It defines `Q_GUI_SUPPORT`. Despite the name, that macro now gates the
  progress-reporting members of `CGA` and `CMCMC` (`include/GA/GA.h:675`,
  `include/MCMC/MCMC.h:665`) rather than anything graphical. Without it the
  build fails with `CGA<SourceSinkData> has no member named SetRunTimeWindow`.
  The macro deserves renaming.
- It still links QtWidgets, because `Interface::ToTable()` returns a
  `QTableWidget*` and `Interface` is a base of every result type. No widget is
  ever created, so the program runs under `QCoreApplication` with no display.

---

## 4. Example data

The example projects used for all the testing below:

- **Linux:** `/home/arash/Dropbox/SourceIDDataForCMBSource/`
- **Windows:** `C:\Users\<your-user>\Dropbox\SourceIDDataForCMBSource\`
  (adjust the user name; if Dropbox is elsewhere, the folder name
  `SourceIDDataForCMBSource` is the thing to search for)

47 `.cmb` projects are in there. The ones that earned their keep:

| file | why it is useful |
|---|---|
| `properties.cmb` | the project Lillian is reviewing; 5 groups, 24 constituents, isotopes C13 and N15 |
| `Isotope_test.cmb` | correctly configured isotopes — C13 based on `TOC` with `TOC` given role `Element` and included. The arrangement the isotope validation recommends. Target group is `Mixture` |
| `simple_example_isotope.cmb` | old file format with no `Include` key — see the trap in section 6 |
| `Virtual.cmb`, `Cara's data.cmb`, `Katskills_4sources.cmb` | additional projects used to confirm the batch fix |

`Isotope_test.cmb` has a property worth knowing: targets `CTAIL2`, `CTAIL3`,
`CTAIL5` are *exactly* the mean profiles of Forest, Pasture and Crop, and
`CTAIL4` is within 0.8% of Bank's. They are validation samples, and the
unmixing recovers each at 99–100% to the right source. That makes the file a
good correctness check, not just a smoke test.

---

## 5. Using the CLI

```
export SEDSAT3_RESOURCES=/path/to/SedSat3/resources     # Linux
set SEDSAT3_RESOURCES=C:\path\to\SedSat3\resources      # Windows cmd
```

```
sedsat3-cli --list-commands
sedsat3-cli --describe "CMB Bayesian-Batch"
sedsat3-cli script.json
sedsat3-cli script.json --quiet --output run.json
```

A script names a project and an ordered array of steps. The array is ordered
because the steps share one dataset and several of them modify it.

```json
{
  "project": "Isotope_test.cmb",
  "output": "report.json",
  "on_error": "stop",
  "steps": [
    { "command": "set-target-group", "arguments": { "group": "Mixture" } },
    { "command": "set-elements",
      "arguments": { "only": ["Aluminum","Barium","Lead","Zinc","TOC","N","C13","N15"] } },
    { "command": "set-element-role",
      "arguments": { "element": "C13", "role": "Isotope",
                     "base element": "TOC", "standard ratio": "0.011113" } },
    { "command": "Levenberg-Marquardt-Batch",
      "arguments": { "Apply size and organic matter correction": "false" } }
  ]
}
```

Notes:

- Command and argument names are exactly the ones the dialogs use. They are
  checked against `resources/forms_structures.json` before anything runs, and a
  mistyped name is reported with a suggestion.
- Anything omitted is filled from the form's default and listed in the report
  under `defaulted`. This matters: the analyses read arguments with
  `std::map::at()` in 199 places, which throws when a key is absent.
- Relative paths resolve against the script's directory. Files an analysis
  writes go to the script's directory unless the script sets `working_folder`.
- Exit status is 0 only if every step succeeded.
- The four setup commands (`set-target-group`, `set-elements`, `set-samples`,
  `set-element-role`) override whatever the project file saved.

`examples/cli-example-script.json` is a starting point.

---

## 6. Open problems, in the order they matter

### 6.1 Result files are blank on Windows — blocking the reviewer

**This is the one to fix first.** Lillian reports that every text output file
from the Bayesian batch is blank. It is not a sample-count problem: running her
exact settings (1000 samples, 8 chains, 100 burn-in) on Linux produced 119
files, 7 per sample for all 17 samples, none of them empty.

The cause is the file names. `Results::Append` (`results.cpp`) keys each result
as `"<index>:" + name`:

```cpp
operator[](aquiutils::numbertostring(int(size())+1)+":"+ ritem.Name()) = ritem;
```

and `MCMC_Batch` uses that key verbatim as a filename
(`src/sourcesinkdata.cpp:4445`):

```cpp
QString file_path = sample_dir.absolutePath() + "/" +
    QString::fromStdString(result_item->first) + ".txt";
```

giving names like `1:MCMC samples.txt`. A colon is legal on Linux, which is why
it works there. On Windows, `name:stream` is NTFS alternate-data-stream syntax,
so the write goes into a hidden stream attached to a zero-length file called
`1`. Explorer shows seven blank files.

It fails silently because neither `sample_dir.mkpath(".")` nor
`output_file.open(...)` has its return value checked
(`src/sourcesinkdata.cpp:4425` and `:4448`).

**Fix:** sanitise the name before using it as a path — replace
`\ / : * ? " < > |` — and check `mkpath` and `open` so a failed write reports
instead of leaving a blank file. **Verify on Windows**, since the whole effect
is Windows-only and cannot be reproduced on Linux.

Confirm the diagnosis on Windows with `dir /r` in one sample folder: it lists
alternate data streams, so the data will appear as `1:MCMC samples.txt:$DATA`
under a 0-byte `1`. The data is recoverable with
`more < "1:MCMC samples.txt" > out.txt`.

### 6.2 `MCMC_Output.txt` is overwritten by every sample

`MCMC_Batch` creates a per-sample directory and then calls `MCMC(...)` passing
the **parent** `working_folder` (`src/sourcesinkdata.cpp:4438`), so the raw
chain log resolves to `working_folder/MCMC_Output.txt` for every sample in turn.
Each sample truncates it — `CMCMC::step` opens it `"w"` and immediately closes
it — so only the last sample's log survives.

Not the cause of 6.1; the per-sample `1:MCMC samples.txt` does preserve each
sample's chains. But it is confusing, and someone looking at
`MCMC_Output.txt` mid-run sees an empty file.

**Fix:** pass the sample directory rather than the working folder.

### 6.3 Levenberg-Marquardt regularisation is ineffective

Running the batch prints bursts of

```
warning: solve(): system is singular (rcond: 1.03627e-19); attempting approx solution
```

These are benign *in the runs observed* — they appear only in the final
iterations, `rcond` is flat across them (the parameters have stopped moving),
and the contributions come out sensible and sum to 1. Armadillo falls back to a
minimum-norm solution applied to an already-negligible step. They have always
happened; Armadillo writes to stderr and the GUI has no console, so they were
invisible until the CLI surfaced them.

The underlying weakness is real, in `OneStepLevenberg_Marquardt_softmax`:

```cpp
jacobian_transpose_jacobian.ScaleDiagonal(1.0 + lambda);     // multiplicative
...
if (det(jacobian_transpose_jacobian) <= 1e-6)
    jacobian_transpose_jacobian += lambda * identity;        // λ is ~0 by then
```

Multiplicative damping cannot lift a diagonal entry that is already ~0, and the
fallback uses a **determinant**, which measures scale rather than conditioning
— scale a well-conditioned 3×3 by 0.01 and its determinant drops a
millionfold. When the fallback does fire it adds `λ·I`, but λ has been divided
by 1.2 on every successful step, so it adds nothing. The guard is weakest
exactly when it is needed.

**Fix:** test `rcond` rather than `det`, and give the additive damping a floor
so it actually regularises. This changes numerical behaviour, so it wants
checking against the example projects before it lands.

### 6.4 Old-format projects silently deselect every constituent

`src/sourcesinkdata.cpp:2638`:

```cpp
elem_info.include_in_analysis = jsonobject[key].toObject()["Include"].toBool();
```

`QJsonValue::Undefined::toBool()` returns `false`, so a project saved before the
`Include` key existed loads with **nothing** included. `simple_example_isotope.cmb`
is such a file. It does not crash — it quietly analyses nothing, which is worse.

**Fix:** default to `true` when the key is absent, matching
`element_information::include_in_analysis`'s own default.

### 6.5 Dead files that do not compile

`drand.cpp` and `src/GA/GA.cpp` both fail to compile — the first has a
`drand48` linkage conflict with `stdlib.h`, the second includes `StringOP.h`,
which does not exist anywhere in the repository. Neither is referenced by
`SedSat3.pro` or `CMakeLists.txt`, so neither is built. Deleting them is free
and removes something a reviewer will ask about.

### 6.6 Four build definitions

`CMakeLists.txt`, `SedSat3.pro`, `CMBSource.pro` and `SedSat3.vcxproj` all
describe the same program, and two of the four are qmake files differing only
by the old and new project name. Adding the CLI meant touching three of them.
Consolidating on CMake would be the moment to also fix the QXlsx problem in
section 3.

---

## 7. Recurring pattern worth naming

Five separate crashes were found and fixed in this round, and four of them were
the same mistake: **a value that can legitimately be absent, consumed without
checking.**

- an isotope's base element distribution — `nullptr` dereferenced
- `lookup()` returning `-1`, with `CVector::operator[]` doing
  `double *p = 0; return *p;` for any out-of-range index
- an uninitialised axis pointer in four plot routines
- `fitted->parameters[0]` on an empty vector, at four sites
- `fopen` returning `nullptr`, passed to `fclose`

`CVector::operator[]` deserves separate attention: a deliberate null
dereference as the out-of-range path means any indexing mistake anywhere in the
codebase is an instant crash with no diagnostic. Worth raising with the code
reviewer as one pattern rather than five tickets.

---

## 8. What the reviewers are waiting on

- **Lillian Gorman Sanisaca** — blank output files (6.1). Until that is fixed
  she can use the results window's own save and the table viewer's Export to
  CSV: both use a file dialog where she types the name, so neither hits the
  colon problem, and both check that the file opened. Her existing output is
  also recoverable from the NTFS streams as described in 6.1. She does not have
  to wait for a new build to continue the review.
- She was also told the README's manual link is stale and would be fixed; it
  still points at a PDF last updated July 2025. The README also still sends
  people to GitHub releases and sedsat.org rather than the GitLab package
  registry, which Cara asked about on 11 September.
- **Cara Peterman-Phipps** — the merge request for `add-gitlab-pages-docs`,
  which gives GitLab its own rendered Doxygen under Deploy → Pages.
- The `v1.1.6` tag still points at a tree from before most of the source
  annotations were added, and still contains ~2,600 files of generated
  documentation. The reserved DOI is `10.5066/P1JF4HJE`. Re-tag before anything
  is issued against it.
