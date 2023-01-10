LATEXRUN_BIN ?= latexrun
PDFLATEX_BIN ?= pdflatex 
PDFLATEX_ARGS ?= -shell-escape -synctex=3

SHELL := /bin/bash

ifeq ($(shell which $(LATEXRUN_BIN) >/dev/null 4>&1; echo $$?), 1)
$(warning Note: ${LATEXRUN_BIN} missing. Installing now...)
$(shell sudo wget https://raw.githubusercontent.com/aclements/latexrun/master/latexrun -O /usr/local/bin/latexrun)
$(shell sudo chmod +x /usr/local/bin/latexrun)
endif

.PHONY: FORCE
report.pdf: FORCE
	@${LATEXRUN_BIN} \
		--latex-cmd=${PDFLATEX_BIN} \
		--latex-args='${PDFLATEX_ARGS}' \
		-o $@ \
		main.tex


.PHONY: clean
clean: 
	@${LATEXRUN_BIN} \
		--clean-all
