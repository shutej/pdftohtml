#========================================================================
#
# Main xpdf Makefile.
#
# Copyright 1996-2003 Glyph & Cog, LLC
#
#========================================================================

SHELL = /bin/sh

DESTDIR =

prefix = /usr/local
exec_prefix = ${prefix}
srcdir = .

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644

EXE =

all:
	cd goo; $(MAKE)
	cd fofi; $(MAKE)
	cd splash; $(MAKE)
	cd xpdf; $(MAKE)
	cd src; $(MAKE)

dummy:

install: dummy
	-mkdir -p $(DESTDIR)${exec_prefix}/bin
	$(INSTALL_PROGRAM) src/pdftohtml$(EXE) $(DESTDIR)${exec_prefix}/bin/pdftohtml$(EXE)

clean:
	-cd goo; $(MAKE) clean
	-cd fofi; $(MAKE) clean
	-cd splash; $(MAKE) clean
	-cd xpdf; $(MAKE) clean
	-cd src; $(MAKE) clean

distclean: clean
	rm -f config.log config.status config.cache
	rm -f aconf.h
	rm -f Makefile goo/Makefile xpdf/Makefile
	rm -f goo/Makefile.dep fofi/Makefile.dep splash/Makefile.dep xpdf/Makefile.dep
	rm -f goo/Makefile.in.bak fofi/Makefile.in.bak splash/Makefile.in.bak xpdf/Makefile.in.bak
	touch goo/Makefile.dep
	touch fofi/Makefile.dep
	touch splash/Makefile.dep
	touch xpdf/Makefile.dep

depend:
	cd goo; $(MAKE) depend
	cd fofi; $(MAKE) depend
	cd splash; $(MAKE) depend
	cd xpdf; $(MAKE) depend
