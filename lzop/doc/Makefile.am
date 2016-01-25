## Process this file with automake to produce Makefile.in
#
# Copyright (C) 1996-2010 Markus F.X.J. Oberhumer
#

SUFFIXES = .1 .html .man .pod .ps .tex .texi .txt

man_MANS = lzop.1

BUILT_SOURCES = lzop.1 lzop.html lzop.man lzop.ps lzop.tex lzop.txt

EXTRA_DIST = lzop.pod magic $(BUILT_SOURCES)


all-local: $(BUILT_SOURCES)

doc-clean:
	-rm -f $(BUILT_SOURCES)


.pod.1:
	pod2man --center=" " --release="$(PACKAGE) $(VERSION)" --date="2010-11-01" $< > $@

.pod.html:
	pod2html --noindex $< > $@
	-rm -f pod2htm*

.pod.tex:
	pod2latex $<

.pod.txt:
	pod2text < $< > $@

.1.man:
	nroff -man $< > $@

.1.ps:
	groff -man $< > $@

