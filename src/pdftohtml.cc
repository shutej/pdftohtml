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
#include <aconf.h>
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
#include "PSOutputDev.h"
#include "GlobalParams.h"
#include "Error.h"
#include "config.h"
#include "gfile.h"

static int firstPage = 1;
static int lastPage = 0;
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

GBool showHidden = gFalse;
GBool noMerge = gFalse;
static char ownerPassword[33] = "";
static char userPassword[33] = "";
static GBool printVersion = gFalse;

static GString* getInfoString(Dict *infoDict, char *key);

static char textEncName[128] = "";

static ArgDesc argDesc[] = {
  {"-f",      argInt,      &firstPage,     0,
   "first page to convert"},
  {"-l",      argInt,      &lastPage,      0,
   "last page to convert"},
  /*{"-raw",    argFlag,     &rawOrder,      0,
    "keep strings in content stream order"},*/
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
  {"-zoom",   argFP,    &scale,         0,
   "zoom the pdf document (default 1.5)"},
  {"-xml",    argFlag,    &xml,         0,
   "output for XML post-processing"},
  {"-hidden", argFlag,   &showHidden,   0,
   "output hidden text"},
  {"-nomerge", argFlag, &noMerge, 0,
   "do not merge paragraphs"},   
  {"-enc",    argString,   textEncName,    sizeof(textEncName),
   "output text encoding name"},
  {"-v",      argFlag,     &printVersion,  0,
   "print copyright and version info"},
  {"-opw",    argString,   ownerPassword,  sizeof(ownerPassword),
   "owner password (for encrypted files)"},
  {"-upw",    argString,   userPassword,   sizeof(userPassword),
   "user password (for encrypted files)"},
  {NULL}
};

int main(int argc, char *argv[]) {
  PDFDoc *doc = NULL;
  GString *fileName = NULL;
  GString *docTitle = NULL;
  GString *htmlFileName = NULL;
  GString *psFileName = NULL;
  HtmlOutputDev *htmlOut = NULL;
  PSOutputDev *psOut = NULL;
  GBool ok;
  char *p;
  GString *ownerPW, *userPW;
  Object info;

  // parse args
  ok = parseArgs(argDesc, &argc, argv);
  if (!ok || argc < 2 || argc > 3 || printHelp || printVersion) {
    fprintf(stderr, "pdftohtml version %s http://pdftohtml.sourceforge.net/\n", "0.33b");
    fprintf(stderr, "%s\n", "Copyright 1999-2002 Gueorgui Ovtcharov and Rainer Dorsch");
    fprintf(stderr, "based on Xpdf version %s\n", xpdfVersion);
    fprintf(stderr, "%s\n\n", xpdfCopyright);
    if (!printVersion) {
      printUsage("pdftohtml", "<PDF-file> [<html-file> <xml-file>]", argDesc);
    }
    exit(1);
  }
  fileName = new GString(argv[1]);

  // init error file
  //errorInit();

  // read config file
  globalParams = new GlobalParams("");

  if (errQuiet) {
    globalParams->setErrQuiet(errQuiet);
    printCommands = gFalse; // I'm not 100% what is the differecne between them
  }

  if (textEncName[0]) {
    globalParams->setTextEncoding(textEncName);
  }

  // open PDF file
  if (ownerPassword[0]) {
    ownerPW = new GString(ownerPassword);
  } else {
    ownerPW = NULL;
  }
  if (userPassword[0]) {
    userPW = new GString(userPassword);
  } else {
    userPW = NULL;
  }
  doc = new PDFDoc(fileName, ownerPW, userPW);
  if (userPW) {
    delete userPW;
  }
  if (ownerPW) {
    delete ownerPW;
  }
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

  doc->getDocInfo(&info);
  if (info.isDict()) {
    docTitle = getInfoString(info.getDict(), "Title");
    if( !docTitle ) docTitle = new GString(htmlFileName);
  }
  info.free();

  // write text file
  htmlOut = new HtmlOutputDev(htmlFileName->getCString(), docTitle, rawOrder);
  delete docTitle;

  if (htmlOut->isOk())  
    doc->displayPages(htmlOut, firstPage, lastPage, static_cast<int>(72*scale), 0, gTrue);
  
  if( mode && !xml && !ignore ) {
    int h=xoutRound(htmlOut->getPageHeight()/scale);
    int w=xoutRound(htmlOut->getPageWidth()/scale);
    //int h=xoutRound(doc->getPageHeight(1)/scale);
    //int w=xoutRound(doc->getPageWidth(1)/scale);

    psFileName = new GString(htmlFileName->getCString());
    psFileName->append(".ps");

    globalParams->setPSPaperWidth(w);
    globalParams->setPSPaperHeight(h);
    globalParams->setPSNoText(gTrue);
    psOut = new PSOutputDev(psFileName->getCString(), doc->getXRef(),
			    doc->getCatalog(), firstPage, lastPage, psModePS);
    doc->displayPages(psOut, firstPage, lastPage, 72, 0, gFalse);
    delete psOut;

    /*sprintf(buf, "%s -sDEVICE=png16m -dBATCH -dNOPROMPT -dNOPAUSE -r72 -sOutputFile=%s%%03d.png -g%dx%d -q %s", GHOSTSCRIPT, htmlFileName->getCString(), w, h,
      psFileName->getCString());*/
    
    GString *gsCmd = new GString(GHOSTSCRIPT);
    GString *tw, *th;
    gsCmd->append(" -sDEVICE=png16m -dBATCH -dNOPROMPT -dNOPAUSE -r72 -sOutputFile=");
    gsCmd->append("\"");
    gsCmd->append(htmlFileName);
    gsCmd->append("%03d.png\" -g");
    tw = GString::IntToStr(w);
    gsCmd->append(tw);
    gsCmd->append("x");
    th = GString::IntToStr(h);
    gsCmd->append(th);
    gsCmd->append(" -q \"");
    gsCmd->append(psFileName);
    gsCmd->append("\"");
    // printf("running: %s\n", gsCmd->getCString());
    if( !executeCommand(gsCmd->getCString()) && !errQuiet) {
      error(-1, "Failed to launch Ghostscript!\n");
    }
    unlink(psFileName->getCString());
    delete tw;
    delete th;
    delete gsCmd;
    delete psFileName;
  }
  
  delete htmlOut;

  // clean up
 err2:
 err1:
  if(doc) delete doc;
  if(globalParams) delete globalParams;

  if(htmlFileName) delete htmlFileName;
  HtmlFont::clear();
  
  // check for memory leaks
  Object::memCheck(stderr);
  gMemReport(stderr);

  return 0;
}

static GString* getInfoString(Dict *infoDict, char *key) {
  Object obj;
  GString *s1 = NULL;

  if (infoDict->lookup(key, &obj)->isString()) {
    s1 = new GString(obj.getString());
  }
  obj.free();
  return s1;
}
