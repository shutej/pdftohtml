//========================================================================
//
// HtmlOutputDev.cc
//
// Copyright 1997-2002 Glyph & Cog, LLC
//
// Changed 1999-2000 by G.Ovtcharov
//
// Changed 2002 by Mikhail Kruk
//
//========================================================================

#ifdef __GNUC__
#pragma implementation
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <ctype.h>
#include <math.h>
#include "GString.h"
#include "gmem.h"
#include "config.h"
#include "Error.h"
#include "GfxState.h"
#include "GlobalParams.h"
#include "HtmlOutputDev.h"
#include "HtmlFonts.h"


int HtmlPage::pgNum=0;
int HtmlOutputDev::imgNum=1;

extern double scale;
extern GBool mode;
extern char extension[5];
extern GBool ignore;
extern GBool printCommands;
extern GBool printHtml;
extern GBool noframes;
extern GBool stout;
extern GBool xml;
extern GBool showHidden;
extern GBool noMerge;

//------------------------------------------------------------------------
// HtmlString
//------------------------------------------------------------------------

GString* basename(GString* str){
  
  char *p=str->getCString();
  int len=str->getLength();
  for (int i=len-1;i>=0;i--)
    if (*(p+i)==SLASH) 
      return new GString((p+i+1),len-i-1);
  return new GString(str);
}

GString* Dirname(GString* str){
  
  char *p=str->getCString();
  int len=str->getLength();
  for (int i=len-1;i>=0;i--)
    if (*(p+i)==SLASH) 
      return new GString(p,i+1);
  return new GString();
} 

HtmlString::HtmlString(GfxState *state, double fontSize, HtmlFontAccu* fonts) {
  GfxFont *font;
  double x, y;

  state->transform(state->getCurX(), state->getCurY(), &x, &y);
  if ((font = state->getFont())) {
    yMin = y - font->getAscent() * fontSize;
    yMax = y - font->getDescent() * fontSize;
    GfxRGB rgb;
    state->getFillRGB(&rgb);
    GString *name = state->getFont()->getName();
    if (!name) name = HtmlFont::getDefaultFont(); //new GString("default");
    HtmlFont hfont=HtmlFont(name, static_cast<int>(fontSize-1), rgb);
    fontpos=fonts->AddFont(hfont);
  } else {
    // this means that the PDF file draws text without a current font,
    // which should never happen
    yMin = y - 0.95 * fontSize;
    yMax = y + 0.35 * fontSize;
    fontpos=0;
  }
  if (yMin == yMax) {
    // this is a sanity check for a case that shouldn't happen -- but
    // if it does happen, we want to avoid dividing by zero later
    yMin = y;
    yMax = y + 1;
  }
  col = 0;
  text = NULL;
  xRight = NULL;
  link = NULL;
  len = size = 0;
  yxNext = NULL;
  xyNext = NULL;
  htext=new GString();
}


HtmlString::~HtmlString() {
  delete text;
  delete htext;
  gfree(xRight);
}

void HtmlString::addChar(GfxState *state, double x, double y,
			 double dx, double dy, Unicode u) {
  if (len == size) {
    size += 16;
    text = (Unicode *)grealloc(text, size * sizeof(Unicode));
    xRight = (double *)grealloc(xRight, size * sizeof(double));
  }
  text[len] = u;
  if (len == 0) {
    xMin = x;
  }
  xMax = xRight[len] = x + dx;
  ++len;
}



//------------------------------------------------------------------------
// HtmlPage
//------------------------------------------------------------------------

HtmlPage::HtmlPage(GBool rawOrder) {
  this->rawOrder = rawOrder;
  curStr = NULL;
  yxStrings = NULL;
  xyStrings = NULL;
  yxCur1 = yxCur2 = NULL;
  fonts=new HtmlFontAccu();
  links=new HtmlLinks();
  pageWidth=0;
  pageHeight=0;
  DocName=NULL;
}

HtmlPage::~HtmlPage() {
  clear();
  if (DocName) delete DocName;
  if (fonts) delete fonts;
  if (links) delete links;
  
}

void HtmlPage::updateFont(GfxState *state) {
  GfxFont *font;
  double *fm;
  char *name;
  int code;
  double w;
  
  // adjust the font size
  fontSize = state->getTransformedFontSize();
  if ((font = state->getFont()) && font->getType() == fontType3) {
    // This is a hack which makes it possible to deal with some Type 3
    // fonts.  The problem is that it's impossible to know what the
    // base coordinate system used in the font is without actually
    // rendering the font.  This code tries to guess by looking at the
    // width of the character 'm' (which breaks if the font is a
    // subset that doesn't contain 'm').
    for (code = 0; code < 256; ++code) {
      if ((name = ((Gfx8BitFont *)font)->getCharName(code)) &&
	  name[0] == 'm' && name[1] == '\0') {
	break;
      }
    }
    if (code < 256) {
      w = ((Gfx8BitFont *)font)->getWidth(code);
      if (w != 0) {
	// 600 is a generic average 'm' width -- yes, this is a hack
	fontSize *= w / 0.6;
      }
    }
    fm = font->getFontMatrix();
    if (fm[0] != 0) {
      fontSize *= fabs(fm[3] / fm[0]);
    }
  }
}

void HtmlPage::beginString(GfxState *state, GString *s) {
  curStr = new HtmlString(state, fontSize, fonts);
}


void HtmlPage::conv(){
  HtmlString *tmp;

  int linkIndex = 0;
  HtmlFont* h;
  for(tmp=yxStrings;tmp;tmp=tmp->yxNext){
     int pos=tmp->fontpos;
     //  printf("%d\n",pos);
     h=fonts->Get(pos);

     if (tmp->htext) delete tmp->htext; 
     tmp->htext=HtmlFont::simple(h,tmp->text,tmp->len);

     if (links->inLink(tmp->xMin,tmp->yMin,tmp->xMax,tmp->yMax, linkIndex)){
       tmp->link = links->getLink(linkIndex);
       /*GString *t=tmp->htext;
       tmp->htext=links->getLink(k)->Link(tmp->htext);
       delete t;*/
     }
  }

}


void HtmlPage::addChar(GfxState *state, double x, double y,
		       double dx, double dy, Unicode *u, int uLen) {
  double x1, y1, w1, h1, dx2, dy2;
  int n, i;

  state->transform(x, y, &x1, &y1);
  n = curStr->len;
  if (n > 0 &&
      x1 - curStr->xRight[n-1] > 0.1 * (curStr->yMax - curStr->yMin)) {
    endString();
    beginString(state, NULL);
  }
  state->textTransformDelta(state->getCharSpace() * state->getHorizScaling(),
			    0, &dx2, &dy2);
  dx -= dx2;
  dy -= dy2;
  state->transformDelta(dx, dy, &w1, &h1);
  if (uLen != 0) {
    w1 /= uLen;
    h1 /= uLen;
  }
  for (i = 0; i < uLen; ++i) {
    curStr->addChar(state, x1 + i*w1, y1 + i*h1, w1, h1, u[i]);
  }
}

void HtmlPage::endString() {
  HtmlString *p1, *p2;
  double h, y1, y2;

  // throw away zero-length strings -- they don't have valid xMin/xMax
  // values, and they're useless anyway
  if (curStr->len == 0) {
    delete curStr;
    curStr = NULL;
    return;
  }

#if 0 //~tmp
  if (curStr->yMax - curStr->yMin > 20) {
    delete curStr;
    curStr = NULL;
    return;
  }
#endif

  // insert string in y-major list
  h = curStr->yMax - curStr->yMin;
  y1 = curStr->yMin + 0.5 * h;
  y2 = curStr->yMin + 0.8 * h;
  if (rawOrder) {
    p1 = yxCur1;
    p2 = NULL;
  } else if ((!yxCur1 ||
              (y1 >= yxCur1->yMin &&
               (y2 >= yxCur1->yMax || curStr->xMax >= yxCur1->xMin))) &&
             (!yxCur2 ||
              (y1 < yxCur2->yMin ||
               (y2 < yxCur2->yMax && curStr->xMax < yxCur2->xMin)))) {
    p1 = yxCur1;
    p2 = yxCur2;
  } else {
    for (p1 = NULL, p2 = yxStrings; p2; p1 = p2, p2 = p2->yxNext) {
      if (y1 < p2->yMin || (y2 < p2->yMax && curStr->xMax < p2->xMin))
        break;
    }
    yxCur2 = p2;
  }
  yxCur1 = curStr;
  if (p1)
    p1->yxNext = curStr;
  else
    yxStrings = curStr;
  curStr->yxNext = p2;
  curStr = NULL;
}

void HtmlPage::coalesce() {
  HtmlString *str1, *str2;
  HtmlFont *hfont1, *hfont2;
  double space, d, vertSpace;
  GBool addSpace, addLineBreak;
  int n, i;
  double curX, curY;

#if 0 //~ for debugging
  for (str1 = yxStrings; str1; str1 = str1->yxNext) {
    printf("x=%3d..%3d  y=%3d..%3d  size=%2d '",
	   (int)str1->xMin, (int)str1->xMax, (int)str1->yMin, (int)str1->yMax,
	   (int)(str1->yMax - str1->yMin));
    for (i = 0; i < str1->len; ++i) {
      fputc(str1->text[i] & 0xff, stdout);
    }
    printf("'\n");
  }
  printf("\n------------------------------------------------------------\n\n");
#endif
  str1 = yxStrings;

  if( !str1 ) return;

  hfont1 = getFont(str1);
  if( hfont1->isBold() )
    str1->htext->insert(0,"<b>",3);
  if( hfont1->isItalic() )
    str1->htext->insert(0,"<i>",3);
  if( str1->getLink() != NULL ) {
    GString *ls = str1->getLink()->getLinkStart();
    str1->htext->insert(0, ls);
    delete ls;
  }
  curX = str1->xMin; curY = str1->yMin;

  while (str1 && (str2 = str1->yxNext)) {
    hfont2 = getFont(str2);
    space = str1->yMax - str1->yMin;
    d = str2->xMin - str1->xMax;
    addLineBreak = !noMerge && (fabs(str1->xMin - str2->xMin) < 0.4);
    vertSpace = str2->yMin - str1->yMax;
    if (((((rawOrder &&
	  ((str2->yMin >= str1->yMin && str2->yMin <= str1->yMax) ||
	   (str2->yMax >= str1->yMin && str2->yMax <= str1->yMax))) ||
	 (!rawOrder && str2->yMin < str1->yMax)) &&
	d > -0.5 * space && d < space) ||
       (vertSpace >= 0 && vertSpace < 0.5 * space && 
	addLineBreak)) &&
	(hfont1->isEqualIgnoreBold(*hfont2))
 	) 
    {
	n = str1->len + str2->len;
	if ((addSpace = d > 0.1 * space)) {
	    ++n;
	}
	if (addLineBreak) {
	    ++n;
	}

      str1->size = (n + 15) & ~15;
      str1->text = (Unicode *)grealloc(str1->text,
				       str1->size * sizeof(Unicode));
      str1->xRight = (double *)grealloc(str1->xRight,
					str1->size * sizeof(double));
      if (addSpace) {
	str1->text[str1->len] = 0x20;
	str1->htext->append(" ");
	str1->xRight[str1->len] = str2->xMin;
	++str1->len;
      }
      if (addLineBreak) {
	  str1->text[str1->len] = '\n';
	  str1->htext->append("<br>");
	  str1->xRight[str1->len] = str2->xMin;
	  ++str1->len;
	  str1->yMin = str2->yMin;
	  str1->yMax = str2->yMax;
	  str1->xMax = str2->xMax;
	  int fontLineSize = hfont1->getLineSize();
	  int curLineSize = (int)(vertSpace + space); 
	  if( curLineSize != fontLineSize )
	  {
	      HtmlFont *newfnt = new HtmlFont(*hfont1);
	      newfnt->setLineSize(curLineSize);
	      str1->fontpos = fonts->AddFont(*newfnt);
	      delete newfnt;
	      hfont1 = getFont(str1);
	      // we have to reget hfont2 because it's location could have
	      // changed on resize
	      hfont2 = getFont(str2); 
	  }
      }
      for (i = 0; i < str2->len; ++i) {
	str1->text[str1->len] = str2->text[i];
	str1->xRight[str1->len] = str2->xRight[i];
	++str1->len;
      }

      /* fix <i> and <b> if str1 and str2 differ */
      if( hfont1->isBold() && !hfont2->isBold() )
	str1->htext->append("</b>", 4);
      if( hfont1->isItalic() && !hfont2->isItalic() )
	str1->htext->append("</i>", 4);
      if( !hfont1->isBold() && hfont2->isBold() )
	str1->htext->append("<b>", 3);
      if( !hfont1->isItalic() && hfont2->isItalic() )
	str1->htext->append("<i>", 3);

      /* now handle switch of links */
      HtmlLink *hlink1 = str1->getLink();
      HtmlLink *hlink2 = str2->getLink();
      if( !hlink1 || !hlink2 || !hlink1->isEqualDest(*hlink2) ) {
	if(hlink1 != NULL )
	  str1->htext->append("</a>");
	if(hlink2 != NULL ) {
	  GString *ls = hlink2->getLinkStart();
	  str1->htext->append(ls);
	  delete ls;
	}
      }

      str1->htext->append(str2->htext);
      // str1 now contains href for link of str2 (if it is defined)
      str1->link = str2->link; 
      hfont1 = hfont2;
      if (str2->xMax > str1->xMax) {
	str1->xMax = str2->xMax;
      }
      if (str2->yMax > str1->yMax) {
	str1->yMax = str2->yMax;
      }
      str1->yxNext = str2->yxNext;
      delete str2;
    } else {
      if( hfont1->isBold() )
	str1->htext->append("</b>",4);
      if( hfont1->isItalic() )
	str1->htext->append("</i>",4);
      if(str1->getLink() != NULL )
	str1->htext->append("</a>");
     
      str1->xMin = curX; str1->yMin = curY; 
      str1 = str2;
      curX = str1->xMin; curY = str1->yMin;
      hfont1 = hfont2;
      if( hfont1->isBold() )
	str1->htext->insert(0,"<b>",3);
      if( hfont1->isItalic() )
	str1->htext->insert(0,"<i>",3);
      if( str1->getLink() != NULL ) {
	GString *ls = str1->getLink()->getLinkStart();
	str1->htext->insert(0, ls);
	delete ls;
      }
    }
  }
  str1->xMin = curX; str1->yMin = curY;
  if( hfont1->isBold() )
    str1->htext->append("</b>",4);
  if( hfont1->isItalic() )
    str1->htext->append("</i>",4);
  if(str1->getLink() != NULL )
    str1->htext->append("</a>");

#if 0 //~ for debugging
  for (str1 = yxStrings; str1; str1 = str1->yxNext) {
    printf("x=%3d..%3d  y=%3d..%3d  size=%2d ",
	   (int)str1->xMin, (int)str1->xMax, (int)str1->yMin, (int)str1->yMax,
	   (int)(str1->yMax - str1->yMin));
    printf("'%s'\n", str1->htext->getCString());  
  }
  printf("\n------------------------------------------------------------\n\n");
#endif

}

void HtmlPage::dumpAsXML(FILE* f,int page){  
  fprintf(f, "<page number=\"%d\" position=\"absolute\"", page);
  fprintf(f," top=\"0\" left=\"0\" height=\"%d\" width=\"%d\">\n", pageHeight,pageWidth);
    
  for(int i=0;i!=fonts->size();i++) {
    GString *fontCSStyle = fonts->CSStyle(i, page);
    fprintf(f,"\t%s\n",fontCSStyle->getCString());
    delete fontCSStyle;
  }
  
  GString *str, *str1;
  for(HtmlString *tmp=yxStrings;tmp;tmp=tmp->yxNext){
    if (tmp->htext){
      str=new GString(tmp->htext);
      fprintf(f,"<text top=\"%d\" left=\"%d\" ",xoutRound(tmp->yMin),xoutRound(tmp->xMin));
      fprintf(f,"width=\"%d\" height=\"%d\" ",xoutRound(tmp->xMax-tmp->xMin),xoutRound(tmp->yMax-tmp->yMin));
      fprintf(f,"font=\"%d-%d\">", page, tmp->fontpos);
      if (tmp->fontpos!=-1){
	str1=fonts->getCSStyle(tmp->fontpos, page, str);
      }
      fputs(str1->getCString(),f);
      delete str;
      delete str1;
      fputs("</text>\n",f);
    }
  }
  fputs("</page>\n",f);
}


void HtmlPage::dumpComplex(FILE *file, int page){
  FILE* pageFile;
  GString* tmp;
  
  if( !noframes )
  {
      GString* pgNum=GString::IntToStr(page);
      tmp = new GString(DocName);
      tmp->append('-')->append(pgNum)->append(".html");
      delete pgNum;
  
      if (!(pageFile = fopen(tmp->getCString(), "w"))) {
	  error(-1, "Couldn't open html file '%s'", tmp->getCString());
	  delete tmp;
	  return;
      } 
      delete tmp;

      fprintf(pageFile,"<html>\n<head>\n<title>Page %d</title>\n\n",page);
      fprintf(pageFile, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=%s\">\n", globalParams->getTextEncodingName()->getCString());
      fprintf(pageFile, "%s\n", GENERATOR);
  }
  else 
  {
      pageFile = file;
      fprintf(pageFile,"<!-- Page %d -->\n", page);
      fprintf(pageFile,"<div style=\"position:relative;\">\n");
  } 

  tmp=basename(DocName);
   
  fputs("<style type=\"text/css\">\n<!--\n",pageFile);
  for(int i=0;i!=fonts->size();i++) {
    GString *fontCSStyle = fonts->CSStyle(i, page);
    fprintf(pageFile,"\t%s\n",fontCSStyle->getCString());
    delete fontCSStyle;
  }
 
  fputs("-->\n</style>\n",pageFile);
  
  if( !noframes )
  {  
      fputs("</head>\n<body vlink=\"blue\" link=\"blue\">\n",pageFile); 
  }
  
  if( !ignore ) {
    //fputs("<div style=\"position:absolute;top:0;left:0\">",pageFile);
    fprintf(pageFile,
	    "<img width=\"%d\" height=\"%d\" src=\"%s%03d.png\">",
	    pageWidth,pageHeight,tmp->getCString(),page);
    //fputs("</div>",pageFile);
  }
  
  delete tmp;
  
  GString *str, *str1;
  for(HtmlString *tmp1=yxStrings;tmp1;tmp1=tmp1->yxNext){
    if (tmp1->htext){
      str=new GString(tmp1->htext);
      fprintf(pageFile,
	      "<div style=\"position:absolute;top:%d;left:%d\">",
	      xoutRound(tmp1->yMin),
	      xoutRound(tmp1->xMin));
      fputs("<nobr>",pageFile); 
      if (tmp1->fontpos!=-1){
	str1=fonts->getCSStyle(tmp1->fontpos, page, str);  
      }
      //printf("%s\n", str1->getCString());
      fputs(str1->getCString(),pageFile);
      
      delete str;      
      delete str1;
      fputs("</nobr></div>\n",pageFile);
    }
  }
  
  if( !noframes )
  {
      fputs("</body>\n</html>\n",pageFile);
      fclose(pageFile);
  }
  else
  {
      fputs("</div>\n", pageFile);
  }
}


void HtmlPage::dump(FILE *f) {
  static int nump=0;

  nump++;
  if (mode){
    if (xml) dumpAsXML(f,nump);
    if (!xml) dumpComplex(f, nump);  
  }
  else{
    fprintf(f,"<a name=%d></a>",nump);
    GString* fName=basename(DocName); 
    for (int i=1;i<HtmlOutputDev::imgNum;i++)
      fprintf(f,"<img src=\"%s-%d_%d.jpg\"><br>\n",fName->getCString(),nump,i);
    HtmlOutputDev::imgNum=1;
    delete fName;

    GString* str;
    for(HtmlString *tmp=yxStrings;tmp;tmp=tmp->yxNext){
      if (tmp->htext){
	str=new GString(tmp->htext); 
	fputs(str->getCString(),f);
	delete str;      
	fputs("<br>\n",f);  
      }
    }   
  }
}



void HtmlPage::clear() {
  HtmlString *p1, *p2;

  if (curStr) {
    delete curStr;
    curStr = NULL;
  }
  for (p1 = yxStrings; p1; p1 = p2) {
    p2 = p1->yxNext;
    delete p1;
  }
  yxStrings = NULL;
  xyStrings = NULL;
  yxCur1 = yxCur2 = NULL;

  if( !noframes )
  {
      delete fonts;
      fonts=new HtmlFontAccu();
  }

  delete links;
  links=new HtmlLinks();
 

}

void HtmlPage::setDocName(char *fname){
  DocName=new GString(fname);
}

//------------------------------------------------------------------------
// HtmlOutputDev
//------------------------------------------------------------------------

void HtmlOutputDev::doFrame(){
  GString* fName=new GString(Docname);
  fName->append(".html");

  if (!(f = fopen(fName->getCString(), "w"))){
    delete fName;
    error(-1, "Couldn't open html file '%s'", fName->getCString());
    return;
  }
  
  delete fName;
    
  fName=basename(Docname);
  fputs("<html>",f);
  fputs("<head>",f);
  fprintf(f,"<title>%s</title>",docTitle->getCString());//fName->getCString());
  fprintf(f, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=%s\">\n", globalParams->getTextEncodingName()->getCString());
  fprintf(f, "%s\n", GENERATOR);
  fprintf(f, "</head>\n");
  fputs("<frameset border=3 frameborder=\"yes\" framespacing=2 cols=\" 100,* \"\n>",f);
  fprintf(f,"<frame name=\"links\" src=\"%s_ind.html\" target=\"rechts\">\n",fName->getCString());
  fputs("<frame name=\"rechts\" src=",f); 
  if (mode) fprintf(f,"\"%s-1.html\"",fName->getCString());
    else
      fprintf(f,"\"%ss.html\"",fName->getCString());
  
  fputs(">\n</frameset>\n</html>\n",f);
 
  delete fName;
  fclose(f);  

}

HtmlOutputDev::HtmlOutputDev(char *fileName, GString *title, 
			     GBool rawOrder) {
  f=NULL;
  docTitle = new GString(title);
  pages = NULL;
  dumpJPEG=gTrue;
  //write = gTrue;
  this->rawOrder = rawOrder;
  ok = gTrue;
  imgNum=1;
  pageNum=1;
  // open file
  needClose = gFalse;
  pages = new HtmlPage(rawOrder);

  maxPageWidth = 0;
  maxPageHeight = 0;

  pages->setDocName(fileName);
  Docname=new GString (fileName);

  /*GString *tmp=basename(Docname);
  if (tmp->getLength()==0) {
    printf("Error : illegal output file");
    exit(1);
    }*/

  //Complex and simple doc with frames
  if(!xml&&!noframes){
     GString* left=new GString(fileName);
     left->append("_ind.html");
     doFrame();
   
     if (!(f=fopen(left->getCString(), "w"))){
        error(-1, "Couldn't open html file '%s'", left->getCString());
	delete left;
        return;
     }
     delete left;
     fputs("<html>\n<head>\n<title></title>\n</head>\n<body>\n",f);
     
     if(!mode){
       GString* right=new GString(fileName);
       right->append("s.html");

       if (!(page=fopen(right->getCString(),"w"))){
        error(-1, "Couldn't open html file '%s'", right->getCString());
        delete right;
	return;
       }
       delete right;
       fputs("<html>\n<head>\n<title></title>\n</head>\n<body>\n",page);
     }
  }

  if (noframes) {
    if (stout) page=stdout;
    else {
      GString* right=new GString(fileName);
      if (!xml) right->append(".html");
      if (xml) right->append(".xml");
      if (!(page=fopen(right->getCString(),"w"))){
	delete right;
	error(-1, "Couldn't open html file '%s'", right->getCString());
	return;
      }  
      delete right;
    }
    if (xml) {
      fputs("<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n", page);
      fputs("<!DOCTYPE pdf2xml SYSTEM \"pdf2xml.dtd\">\n\n", page);
      fputs("<pdf2xml>\n",page);
    } else {
      fprintf(page,"<html>\n<head>\n<title>%s</title>\n",docTitle->getCString());//tmp->getCString());
      fprintf(page, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=%s\">\n", globalParams->getTextEncodingName()->getCString());
      fprintf(page, "%s\n", GENERATOR);
      fprintf(page,"</head>\n");
      fprintf(page,"<body bgcolor=\"#A0A0A0\" vlink=\"blue\" link=\"blue\">\n");
    }
  }    
  
  //delete tmp;
}

HtmlOutputDev::~HtmlOutputDev() {
  /*if (mode&&!xml){
    int h=xoutRound(pages->pageHeight/scale);
    int w=xoutRound(pages->pageWidth/scale);
    fprintf(tin,"%s=%03d\n","PAPER_WIDTH",w);
    fprintf(tin,"%s=%03d\n","PAPER_HEIGHT",h);
    fclose(tin);
    }*/

    HtmlFont::clear(); 
    
    delete Docname;
    delete docTitle;

    if (f){
      fputs("</body>\n</html>\n",f);  
      fclose(f);
    }
    if (xml) {
      fputs("</pdf2xml>\n",page);  
      fclose(page);
    } else
    if (!mode||xml){ 
      fputs("</body>\n</html>\n",page);  
      fclose(page);
    }
    if (pages)
      delete pages;
}



void HtmlOutputDev::startPage(int pageNum, GfxState *state) {
  /*if (mode&&!xml){
    if (write){
      write=gFalse;
      GString* fname=Dirname(Docname);
      fname->append("image.log");
      if((tin=fopen(fname->getCString(),"w"))==NULL){
	printf("Error : can not open %s",fname);
	exit(1);
      }
      delete fname;
    // if(state->getRotation()!=0) 
    //  fprintf(tin,"ROTATE=%d rotate %d neg %d neg translate\n",state->getRotation(),state->getX1(),-state->getY1());
    // else 
      fprintf(tin,"ROTATE=%d neg %d neg translate\n",state->getX1(),state->getY1());  
    }
  }*/

  GString *str=basename(Docname);
  pages->clear(); 
  if(!noframes){
    if (f){
      if (mode)
	fprintf(f,"<a href=\"%s-%d.html\"",str->getCString(),pageNum);
      else 
	fprintf(f,"<a href=\"%ss.html#%d\"",str->getCString(),pageNum);
      fprintf(f," target=\"rechts\" >Page %d</a>\n",pageNum);
    }
  }

  pages->pageWidth=static_cast<int>(state->getPageWidth());
  pages->pageHeight=static_cast<int>(state->getPageHeight());

  delete str;
} 


void HtmlOutputDev::endPage() {
  pages->conv();
  pages->coalesce();
  pages->dump(page);
  
  // I don't yet know what to do in the case when there are pages of different
  // sizes and we want complex output: running ghostscript many times 
  // seems very inefficient. So for now I'll just use last page's size
  maxPageWidth = pages->pageWidth;
  maxPageHeight = pages->pageHeight;
  
  if(!noframes&&!xml) fputs("<br>", f);
  if(!stout && !globalParams->getErrQuiet()) printf("Page-%d\n",(pageNum));
  pageNum++ ;
}

void HtmlOutputDev::updateFont(GfxState *state) {
  pages->updateFont(state);
}

void HtmlOutputDev::beginString(GfxState *state, GString *s) {
  pages->beginString(state, s);
}

void HtmlOutputDev::endString(GfxState *state) {
  pages->endString();
}

void HtmlOutputDev::drawChar(GfxState *state, double x, double y,
	      double dx, double dy,
	      double originX, double originY,
	      CharCode code, Unicode *u, int uLen) 
{
  if ( !showHidden && (state->getRender() & 3) == 3) {
    return;
  }
  pages->addChar(state, x, y, dx, dy, u, uLen);
}

void HtmlOutputDev::drawImageMask(GfxState *state, Object *ref, Stream *str,
			      int width, int height, GBool invert,
			      GBool inlineImg) {

  int i, j;
  
  if (ignore||mode) {
    OutputDev::drawImageMask(state, ref, str, width, height, invert, inlineImg);
    return;
  }
  
  FILE *f1;
  int c;
  
  int x0, y0;			// top left corner of image
  int w0, h0, w1, h1;		// size of image
  double xt, yt, wt, ht;
  GBool rotate, xFlip, yFlip;
  GBool dither;
  int x, y;
  int ix, iy;
  int px1, px2, qx, dx;
  int py1, py2, qy, dy;
  Gulong pixel;
  int nComps, nVals, nBits;
  double r1, g1, b1;
 
  // get image position and size
  state->transform(0, 0, &xt, &yt);
  state->transformDelta(1, 1, &wt, &ht);
  if (wt > 0) {
    x0 = xoutRound(xt);
    w0 = xoutRound(wt);
  } else {
    x0 = xoutRound(xt + wt);
    w0 = xoutRound(-wt);
  }
  if (ht > 0) {
    y0 = xoutRound(yt);
    h0 = xoutRound(ht);
  } else {
    y0 = xoutRound(yt + ht);
    h0 = xoutRound(-ht);
  }
  state->transformDelta(1, 0, &xt, &yt);
  rotate = fabs(xt) < fabs(yt);
  if (rotate) {
    w1 = h0;
    h1 = w0;
    xFlip = ht < 0;
    yFlip = wt > 0;
  } else {
    w1 = w0;
    h1 = h0;
    xFlip = wt < 0;
    yFlip = ht > 0;
  }

  // dump JPEG file
  if (dumpJPEG  && str->getKind() == strDCT) {
    GString *fName=new GString(Docname);
    fName->append("-");
    GString *pgNum=GString::IntToStr(pageNum);
    GString *imgnum=GString::IntToStr(imgNum);
    // open the image file
    fName->append(pgNum)->append("_")->append(imgnum)->append(".jpg");
    ++imgNum;
    if (!(f1 = fopen(fName->getCString(), "wb"))) {
      error(-1, "Couldn't open image file '%s'", fName->getCString());
      return;
    }

    // initialize stream
    str = ((DCTStream *)str)->getRawStream();
    str->reset();

    // copy the stream
    while ((c = str->getChar()) != EOF)
      fputc(c, f1);

    fclose(f1);
   
  if (pgNum) delete pgNum;
  if (imgnum) delete imgnum;
  if (fName) delete fName;
  }
  else {
    OutputDev::drawImageMask(state, ref, str, width, height, invert, inlineImg);
  }
}

void HtmlOutputDev::drawImage(GfxState *state, Object *ref, Stream *str,
			  int width, int height, GfxImageColorMap *colorMap,
			  int *maskColors, GBool inlineImg) {

  int i, j;
   
  if (ignore||mode) {
    OutputDev::drawImage(state, ref, str, width, height, colorMap, 
			 maskColors, inlineImg);
    return;
  }

  FILE *f1;
  ImageStream *imgStr;
  Guchar pixBuf[4];
  GfxColor color;
  int c;
  
  int x0, y0;			// top left corner of image
  int w0, h0, w1, h1;		// size of image
  double xt, yt, wt, ht;
  GBool rotate, xFlip, yFlip;
  GBool dither;
  int x, y;
  int ix, iy;
  int px1, px2, qx, dx;
  int py1, py2, qy, dy;
  Gulong pixel;
  int nComps, nVals, nBits;
  double r1, g1, b1;
 
  // get image position and size
  state->transform(0, 0, &xt, &yt);
  state->transformDelta(1, 1, &wt, &ht);
  if (wt > 0) {
    x0 = xoutRound(xt);
    w0 = xoutRound(wt);
  } else {
    x0 = xoutRound(xt + wt);
    w0 = xoutRound(-wt);
  }
  if (ht > 0) {
    y0 = xoutRound(yt);
    h0 = xoutRound(ht);
  } else {
    y0 = xoutRound(yt + ht);
    h0 = xoutRound(-ht);
  }
  state->transformDelta(1, 0, &xt, &yt);
  rotate = fabs(xt) < fabs(yt);
  if (rotate) {
    w1 = h0;
    h1 = w0;
    xFlip = ht < 0;
    yFlip = wt > 0;
  } else {
    w1 = w0;
    h1 = h0;
    xFlip = wt < 0;
    yFlip = ht > 0;
  }

   
  /*if( !globalParams->getErrQuiet() )
    printf("image stream of kind %d\n", str->getKind());*/
  // dump JPEG file
  if (dumpJPEG && str->getKind() == strDCT) {
    GString *fName=new GString(Docname);
    fName->append("-");
    GString *pgNum= GString::IntToStr(pageNum);
    GString *imgnum= GString::IntToStr(imgNum);  
    
    // open the image file
    fName->append(pgNum)->append("_")->append(imgnum)->append(".jpg");
    ++imgNum;
    
    if (!(f1 = fopen(fName->getCString(), "wb"))) {
      error(-1, "Couldn't open image file '%s'", fName->getCString());
      return;
    }

    // initialize stream
    str = ((DCTStream *)str)->getRawStream();
    str->reset();

    // copy the stream
    while ((c = str->getChar()) != EOF)
      fputc(c, f1);
    
    fclose(f1);
  
    delete fName;
    delete pgNum;
    delete imgnum;
  }
  else {
    OutputDev::drawImage(state, ref, str, width, height, colorMap,
			 maskColors, inlineImg);
  }
}



void HtmlOutputDev::drawLink(Link* link,Catalog *cat){
  double _x1,_y1,_x2,_y2,w;
  int x1,y1,x2,y2;
  
  link->getBorder(&_x1,&_y1,&_x2,&_y2,&w);
  cvtUserToDev(_x1,_y1,&x1,&y1);
  
  cvtUserToDev(_x2,_y2,&x2,&y2); 


  GString* _dest=getLinkDest(link,cat);
  HtmlLink t((double) x1,(double) y2,(double) x2,(double) y1,_dest);
  pages->AddLink(t);
  delete _dest;
}

GString *HtmlOutputDev::getLinkDest(Link *link,Catalog* catalog){
  char *p;
  switch(link->getAction()->getKind()) {
  case actionGoTo:{ 
    GString* file=basename(Docname);
    int page=1;
    LinkGoTo *ha=(LinkGoTo *)link->getAction();
    LinkDest *dest=NULL;
    if (ha->getDest()==NULL) dest=catalog->findDest(ha->getNamedDest());
    else dest=ha->getDest()->copy();
    if (dest){ 
      if (dest->isPageRef()){
	Ref pageref=dest->getPageRef();
	page=catalog->findPage(pageref.num,pageref.gen);
      }
      else {
	page=dest->getPageNum();
      }
      
      delete dest;

      GString *str=GString::IntToStr(page);
      if (mode) file->append("-");
      else{ 
	if (!noframes) file->append("s");
	file->append(".html#");
      }
      file->append(str);
      if (mode) file->append(".html");
      if (printCommands) printf(" page %d ",page);
      delete str;
      return file;
    }
    else return new GString();
  }
  
  case actionGoToR:{              
    LinkGoToR *ha=(LinkGoToR *) link->getAction();
    LinkDest *dest=NULL;
    int page=1;
    GString *file=new GString();
    if (ha->getFileName()){
      delete file;
      file=new GString(ha->getFileName()->getCString());
    }
    if (ha->getDest()!=NULL)  dest=ha->getDest()->copy();
    if (dest&&file){
      if (!(dest->isPageRef()))  page=dest->getPageNum();
      delete dest;

      if (printCommands) printf(" page %d ",page);
      if (printHtml){
	p=file->getCString()+file->getLength()-4;
	if (!strcmp(p, ".pdf") || !strcmp(p, ".PDF")){
	  file->del(file->getLength()-4,4);
	  file->append(".html");
	}
	file->append('#');
	file->append(GString::IntToStr(page));
      }
    }
    if (printCommands) printf("filename %s\n",file->getCString());
    return file;
  }
  case actionURI: {
    LinkURI *ha=(LinkURI *) link->getAction();
    GString* file=new GString(ha->getURI()->getCString());
    // printf("uri : %s\n",file->getCString());
    return file;
  }
       
  case actionLaunch: {
    LinkLaunch *ha=(LinkLaunch *) link->getAction();
    GString* file=new GString(ha->getFileName()->getCString());
    if (printHtml) { 
      p=file->getCString()+file->getLength()-4;
      if (!strcmp(p, ".pdf") || !strcmp(p, ".PDF")){
	file->del(file->getLength()-4,4);
	file->append(".html");
      }
      if (printCommands) printf("filename %s",file->getCString());
    }
    return file;      
  }
  default:
    return new GString();
  }
}
