## Process this file with automake to produce Makefile.in
#
# Copyright (C) 1996-2010 Markus F.X.J. Oberhumer
#

SUBDIRS = src doc

dist-hook:
	-rm -rf $(distdir)/autoconf/
	cp -pR $(srcdir)/B/ $(srcdir)/autoconf/ $(distdir)/
	find $(distdir)/ -type l -print0 | xargs -0r rm -f

@MAINT@-include $(top_srcdir)/Makefile.maint
