

SHELL = /bin/sh

srcdir = .
xpdfsrcdir = ../xpdf/

GOOSRCDIR = $(xpdfsrcdir)/../goo
GOOLIBDIR = ../goo


CXXFLAGS = -g -DHAVE_DIRENT_H=1 -DHAVE_REWINDDIR=1 -DHAVE_POPEN=1   -I$(GOOSRCDIR) -I$(srcdir) -I$(xpdfsrcdir)

LDFLAGS = 

OTHERLIBS = 

CXX = c++

LIBPREFIX = lib
EXE = 

#------------------------------------------------------------------------

.SUFFIXES: .cc

.cc.o:
	$(CXX) $(CXXFLAGS) -c $<

#------------------------------------------------------------------------

CXX_SRC = \
	$(srcdir)/pdftohtml.cc \
	$(srcdir)/HtmlOutputDev.cc \
	$(srcdir)/HtmlFonts.cc \
	$(srcdir)/HtmlLinks.cc 

#------------------------------------------------------------------------

all: pdftohtml$(EXE)

#-------------------------------------------------------------------------

PDFTOHTML_OBJS = $(xpdfsrcdir)/Array.o  $(xpdfsrcdir)Catalog.o \
	$(xpdfsrcdir)Dict.o $(xpdfsrcdir)Error.o \
	$(xpdfsrcdir)FontFile.o $(xpdfsrcdir)Gfx.o $(xpdfsrcdir)GfxFont.o \
	$(xpdfsrcdir)GfxState.o $(xpdfsrcdir)Lexer.o $(xpdfsrcdir)Link.o \
	$(xpdfsrcdir)Object.o $(xpdfsrcdir)OutputDev.o $(xpdfsrcdir)Page.o \
	$(xpdfsrcdir)Parser.o $(xpdfsrcdir)PDFDoc.o $(xpdfsrcdir)Stream.o\
	$(xpdfsrcdir)XRef.o $(xpdfsrcdir)GlobalParams.o \
	$(xpdfsrcdir)FontEncodingTables.o \
	$(xpdfsrcdir)CharCodeToUnicode.o \
	$(xpdfsrcdir)BuiltinFontTables.o \
	$(xpdfsrcdir)BuiltinFont.o \
	$(xpdfsrcdir)CMap.o \
	$(xpdfsrcdir)Function.o \
	$(xpdfsrcdir)UnicodeMap.o \
	$(xpdfsrcdir)NameToCharCode.o \
	$(xpdfsrcdir)Decrypt.o \
	$(xpdfsrcdir)FormWidget.o \
	HtmlOutputDev.o HtmlFonts.o HtmlLinks.o pdftohtml.o
PDFTOHTML_LIBS = -L$(GOOLIBDIR) -lGoo $(OTHERLIBS) -lm

pdftohtml$(EXE): $(PDFTOHTML_OBJS) $(GOOLIBDIR)/$(LIBPREFIX)Goo.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o pdftohtml$(EXE) $(PDFTOHTML_OBJS) \
		$(PDFTOHTML_LIBS)


#-------------------------------------------------------------------------
clean:
	rm -f $(PDFTOHTML_OBJS) pdftohtml$(EXE)

#------------------------------------------------------------------------

distdepend:
	cp Makefile.in Makefile.in.bak
	sed '/^#----- dependences -----/q' Makefile.in.bak >Makefile.in
	$(CXX) $(CXXFLAGS) -MM $(CXX_SRC) >>Makefile.in

