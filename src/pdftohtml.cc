//========================================================================
//
// pdftohtml.cc
//
//
// Copyright 1999-2000 G. Ovtcharov
//========================================================================

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "parseargs.h"
#include "GString.h"
#include "gmem.h"
#include "Object.h"
#include "Stream.h"
#include "Array.h"
#include "Dict.h"
#include "XRef.h"
#include "Catalog.h"
#include "Page.h"
#include "PDFDoc.h"
#include "HtmlOutputDev.h"
#include "GlobalParams.h"
#include "Error.h"
#include "config.h"

static int firstPage = 1;
static int lastPage = 0;
static GBool useASCII7 = gFalse;
#if JAPANESE_SUPPORT
static GBool useEUCJP = gFalse;
#endif
static GBool rawOrder = gTrue;
GBool printCommands = gTrue;
static GBool printHelp = gFalse;
GBool printHtml = gFalse;
GBool mode=gFalse;
GBool ignore=gFalse;
char extension[5]=".png";
double scale=1.5;
GBool noframes=gFalse;
GBool stout=gFalse;
GBool xml=gFalse;
GBool errQuiet=gFalse;

static ArgDesc argDesc[] = {
  {"-f",      argInt,      &firstPage,     0,
   "first page to convert"},
  {"-l",      argInt,      &lastPage,      0,
   "last page to convert"},
#if JAPANESE_SUPPORT
  {"-eucjp",  argFlag,     &useEUCJP,      0,
   "convert Japanese text to EUC-JP"},
#endif
  {"-q",      argFlag,     &errQuiet,      0,
   "don't print any messages or errors"},
  {"-h",      argFlag,     &printHelp,     0,
   "print usage information"},
  {"-help",   argFlag,     &printHelp,     0,
   "print usage information"},
  {"-p",      argFlag,     &printHtml,     0,
   "exchange .pdf links by .html"}, 
  {"-c",      argFlag,     &mode,          0,
   "generate complex document"},
  {"-i",      argFlag,     &ignore,        0,
   "ignore images"},
  {"-noframes", argFlag,   &noframes,      0,
   "generate no frames"},
  {"-stdout"  ,argFlag,    &stout,         0,
   "use standard output"},
  {"-zoom"   ,argFP,    &scale,         0,
   "zoom the pdf document (default 1.5)"},
  {"-xml"  ,argFlag,    &xml,         0,
   "output for XML post-processing"},
  {NULL}
};

int main(int argc, char *argv[]) {
  PDFDoc *doc;
  GString *fileName;
  GString *htmlFileName;
  HtmlOutputDev *htmlOut;
  GBool ok;
  char *p;

  // parse args
  ok = parseArgs(argDesc, &argc, argv);
  if (!ok || argc < 2 || argc > 3 || printHelp) {
    fprintf(stderr, "pdftohtml.bin version %s\n", "0.32a");
    fprintf(stderr, "%s\n", "Copyright 1999-2002 Gueorgui Ovtcharov and Rainer Dorsch");
    printUsage("pdftohtml", "<PDF-file> [<html-file> <xml-file>]", argDesc);
    exit(1);
  }
  fileName = new GString(argv[1]);

  // init error file
  //errorInit();

  // read config file
  globalParams = new GlobalParams("");
  //initParams(xpdfConfigFile);

  if (errQuiet) {
    globalParams->setErrQuiet(errQuiet);
    printCommands = gFalse; // I'm not 100% what is the differecne between them
  }


  // open PDF file
  //xref = NULL;
  doc = new PDFDoc(fileName, NULL, NULL);
  if (!doc->isOk()) {
    goto err1;
  }

  // check for copy permission
  if (!doc->okToCopy()) {
    error(-1, "Copying of text from this document is not allowed.");
    goto err2;
  }

  // construct text file name
  if (argc == 3) {
    GString* tmp = new GString(argv[2]);
    p=tmp->getCString()+tmp->getLength()-5;
    if (!xml)
      if (!strcmp(p, ".html") || !strcmp(p, ".HTML"))
	htmlFileName = new GString(tmp->getCString(),
				   tmp->getLength() - 5);
      else htmlFileName =new GString(tmp);
    else   
      if (!strcmp(p, ".xml") || !strcmp(p, ".XML"))
	htmlFileName = new GString(tmp->getCString(),
				   tmp->getLength() - 5);
      else htmlFileName =new GString(tmp);
    
    delete tmp;
  } else {
    p = fileName->getCString() + fileName->getLength() - 4;
    if (!strcmp(p, ".pdf") || !strcmp(p, ".PDF"))
      htmlFileName = new GString(fileName->getCString(),
				 fileName->getLength() - 4);
    else
      htmlFileName = fileName->copy();
    //   htmlFileName->append(".html");
  }
  
   if (scale>3.0) scale=3.0;
   if (scale<0.5) scale=0.5;
   
   if (mode) {
     noframes=gFalse;
     stout=gFalse;
   } 

   if (stout) {
     noframes=gTrue;
     mode=gFalse;
   }

   if (xml)
     { 
       mode = gTrue;
       noframes = gTrue;
     }

  // get page range
  if (firstPage < 1)
    firstPage = 1;
  if (lastPage < 1 || lastPage > doc->getNumPages())
    lastPage = doc->getNumPages();

  // write text file
#if JAPANESE_SUPPORT
  useASCII7 |= useEUCJP;
#endif
  htmlOut = new HtmlOutputDev(htmlFileName->getCString(), useASCII7, rawOrder);
  if (htmlOut->isOk())  
    doc->displayPages(htmlOut, firstPage, lastPage, static_cast<int>(72*scale), 0, gTrue);
  
  delete htmlOut;

  // clean up
  delete htmlFileName;
 err2:
  delete doc;
 err1:
  delete globalParams;

  // check for memory leaks
  Object::memCheck(stderr);
  gMemReport(stderr);

  return 0;
}
