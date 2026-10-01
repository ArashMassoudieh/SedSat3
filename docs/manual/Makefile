# Build the SedSAT3 command line user's manual.
#
#   make          build the PDF
#   make commands regenerate the command reference from the form definitions
#   make clean    remove the build products, keeping the PDF
#   make distclean also remove the PDF and the generated reference
#
# The command reference in commands.tex is generated from
# resources/forms_structures.json, the file the graphical interface builds its
# dialogs from. Regenerate it whenever a command or an argument changes, so
# that the manual and the software cannot describe them differently.

MANUAL  := sedsat3-cli-manual
SCHEMA  := ../../resources/forms_structures.json
GENERATE := generate-command-reference.py

.PHONY: all commands clean distclean

all: $(MANUAL).pdf

$(MANUAL).pdf: $(MANUAL).tex commands.tex
	latexmk -pdf -interaction=nonstopmode -halt-on-error $(MANUAL).tex

# Regenerated automatically when the form definitions are newer.
commands.tex: $(SCHEMA) $(GENERATE)
	python3 $(GENERATE) > $@

commands:
	python3 $(GENERATE) > commands.tex

clean:
	latexmk -c $(MANUAL).tex
	rm -f $(MANUAL).ind $(MANUAL).ilg $(MANUAL).idx

distclean: clean
	rm -f $(MANUAL).pdf commands.tex
