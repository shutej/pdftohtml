

SHELL = /bin/sh

prefix = /usr/local
exec_prefix = ${prefix}
srcdir = .


EXE = 

all:
	cd goo; $(MAKE)
	cd xpdf; $(MAKE) all
	cd src; $(MAKE)
	cd src; mv pdftohtml ../pdftohtml.bin
	cd xpdf; mv pdftops ../pdftops.bin

clean:
	rm -f pdftohtml.bin
	rm -f pdftops.bin
	cd goo; $(MAKE) clean
	cd xpdf; $(MAKE) clean
	cd src; $(MAKE) clean

distdepend:
	rm -f pdftohtml.bin
	rm -f pdftops.bin
	cd goo; $(MAKE) distdepend
	cd xpdf; $(MAKE) distdepend








