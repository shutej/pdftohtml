# Generated automatically from Makefile.in by configure.
#========================================================================
#
# Xpdf Makefile
#
# Copyright 1996 Derek B. Noonburg
#
#========================================================================

SHELL = /bin/sh

prefix = /usr/local
srcdir = .

GOOSRCDIR = $(srcdir)/../goo
GOOLIBDIR = ../goo

CXXFLAGS = -g -O2 -DHAVE_CONFIG_H -I.. -I$(GOOSRCDIR)  -I$(srcdir)  

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
	$(srcdir)/Array.cc \
	$(srcdir)/BuiltinFont.cc \
	$(srcdir)/BuiltinFontTables.cc \
	$(srcdir)/CMap.cc \
	$(srcdir)/Catalog.cc \
	$(srcdir)/CharCodeToUnicode.cc \
	$(srcdir)/Decrypt.cc \
	$(srcdir)/Dict.cc \
	$(srcdir)/Error.cc \
	$(srcdir)/FTFont.cc \
	$(srcdir)/FontEncodingTables.cc \
	$(srcdir)/FontFile.cc \
	$(srcdir)/FormWidget.cc \
	$(srcdir)/Function.cc \
	$(srcdir)/Gfx.cc \
	$(srcdir)/GfxFont.cc \
	$(srcdir)/GfxState.cc \
	$(srcdir)/GlobalParams.cc \
	$(srcdir)/ImageOutputDev.cc \
	$(srcdir)/Lexer.cc \
	$(srcdir)/Link.cc \
	$(srcdir)/NameToCharCode.cc \
	$(srcdir)/Object.cc \
	$(srcdir)/OutputDev.cc \
	$(srcdir)/PBMOutputDev.cc \
	$(srcdir)/PDFDoc.cc \
	$(srcdir)/PSOutputDev.cc \
	$(srcdir)/Page.cc \
	$(srcdir)/Parser.cc \
	$(srcdir)/SFont.cc \
	$(srcdir)/Stream.cc \
	$(srcdir)/T1Font.cc \
	$(srcdir)/TTFont.cc \
	$(srcdir)/TextOutputDev.cc \
	$(srcdir)/UnicodeMap.cc \
	$(srcdir)/XOutputDev.cc \
	$(srcdir)/XRef.cc \
	$(srcdir)/pdftops.cc 

#------------------------------------------------------------------------

all: pdftops$(EXE) 

#------------------------------------------------------------------------

XPDF_OBJS = Array.o BuiltinFont.o BuiltinFontTables.o Catalog.o \
	CharCodeToUnicode.o CMap.o Decrypt.o Dict.o Error.o FTFont.o \
	FontEncodingTables.o FontFile.o FormWidget.o Function.o Gfx.o \
	GfxFont.o GfxState.o GlobalParams.o Lexer.o Link.o \
	NameToCharCode.o Object.o OutputDev.o Page.o Parser.o PDFDoc.o \
	PSOutputDev.o SFont.o Stream.o T1Font.o TTFont.o TextOutputDev.o \
	UnicodeMap.o XOutputDev.o XRef.o 

XPDF_LIBS =  -L$(GOOLIBDIR) -lGoo $(XLIBS) $(OTHERLIBS) -lm

#------------------------------------------------------------------------

PDFTOPS_OBJS = Array.o BuiltinFont.o BuiltinFontTables.o Catalog.o \
	CharCodeToUnicode.o CMap.o Decrypt.o Dict.o Error.o \
	FontEncodingTables.o FontFile.o FormWidget.o Function.o Gfx.o \
	GfxFont.o GfxState.o GlobalParams.o Lexer.o Link.o \
	NameToCharCode.o Object.o OutputDev.o Page.o Parser.o PDFDoc.o \
	PSOutputDev.o Stream.o UnicodeMap.o XRef.o pdftops.o
PDFTOPS_LIBS = -L$(GOOLIBDIR) -lGoo $(OTHERLIBS) -lm

pdftops$(EXE): $(PDFTOPS_OBJS) $(GOOLIBDIR)/$(LIBPREFIX)Goo.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o pdftops$(EXE) $(PDFTOPS_OBJS) \
		$(PDFTOPS_LIBS)

#------------------------------------------------------------------------

clean:
	rm -f $(PDFTOPS_OBJS) pdftops$(EXE)

#------------------------------------------------------------------------

depend:
	$(CXX) $(CXXFLAGS) -MM $(CXX_SRC) >Makefile.dep

-include Makefile.dep
