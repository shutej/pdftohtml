#include "HtmlFonts.h"
#include "GlobalParams.h"
#include "UnicodeMap.h"
#include <stdio.h>

 struct Fonts{
    char *Fontname;
    char *name;
  };

  static Fonts fonts[]={  
     {"Courier",               "Courier" },
     {"Courier-Bold",           "Courier"},
     {"Courier-BoldOblique",    "Courier"},
     {"Courier-Oblique",        "Courier"},
     {"Helvetica",              "Helvetica"},
     {"Helvetica-Bold",         "Helvetica"},
     {"Helvetica-BoldOblique",  "Helvetica"},
     {"Helvetica-Oblique",      "Helvetica"},
     {"Symbol",                 "Symbol"   },
     {"Times-Bold",             "Times"    },
     {"Times-BoldItalic",       "Times"    },
     {"Times-Italic",           "Times"    },
     {"Times-Roman",            "Times"    },
     {" "          ,            "Times"    },

};

int HtmlFont::leak=0;
#define xoutRound(x) ((int)(x + 0.5))
extern GBool xml;

const int font_num=13;
GString* HtmlFont::DefaultFont=new GString("Times"); // Arial,Helvetica,sans-serif

HtmlFontColor::HtmlFontColor(GfxRGB rgb){
  r=static_cast<int>(255*rgb.r);
  g=static_cast<int>(255*rgb.g);
  b=static_cast<int>(255*rgb.b);
  if (!(Ok(r)&&Ok(b)&&Ok(g))) {printf("Error : Bad color \n");r=0;g=0;b=0;}
}

GString *HtmlFontColor::convtoX(unsigned int xcol) const{
  GString *xret=new GString();
  char tmp;
  unsigned  int k;
  k = (xcol/16);
  if ((k>=0)&&(k<10)) tmp=(char) ('0'+k); else tmp=(char)('a'+k-10);
  xret->append(tmp);
  k = (xcol%16);
  if ((k>=0)&&(k<10)) tmp=(char) ('0'+k); else tmp=(char)('a'+k-10);
  xret->append(tmp);
 return xret;
}

GString *HtmlFontColor::toString() const{
  GString *tmp=new GString("#");
  GString *tmpr=convtoX(r); 
  GString *tmpg=convtoX(g);
  GString *tmpb=convtoX(b);
  tmp->append(tmpr);
  tmp->append(tmpg);
  tmp->append(tmpb);
  delete tmpr;
  delete tmpg;
  delete tmpb;
  return tmp;
} 

HtmlFont::HtmlFont(GString* ftname,int _size,GfxRGB rgb){
  leak++;

  //if (col) color=HtmlFontColor(col); 
  //else color=HtmlFontColor();
  color=HtmlFontColor(rgb);
  GString *fontname=new GString(ftname);
  size=(_size-1);
  italic = gFalse;
  bold = gFalse;
  FontName=new GString(ftname);
  if (fontname){
    if (strstr(fontname->lowerCase()->getCString(),"bold"))  bold=gTrue;
    
    if (strstr(fontname->lowerCase()->getCString(),"italic")||
	strstr(fontname->lowerCase()->getCString(),"oblique"))  italic=gTrue;
    
    int i=0;
    while (strcmp(ftname->getCString(),fonts[i].Fontname)&&(i<font_num)) i++;
    pos=i;
    delete fontname;
  }  
  if (!DefaultFont) DefaultFont=new GString(fonts[font_num].name);

}
 
HtmlFont::HtmlFont(const HtmlFont& x){
   leak++;
   size=x.size;
   italic=x.italic;
   bold=x.bold;
   pos=x.pos;
   color=x.color;
   FontName=new GString(x.FontName);
 }


HtmlFont::~HtmlFont(){
  leak--;
  if (!FontName) delete FontName;
}

HtmlFont& HtmlFont::operator=(const HtmlFont& x){
   if (this==&x) return *this; 
   size=x.size;
   italic=x.italic;
   bold=x.bold;
   pos=x.pos;
   color=x.color;
   if (FontName) delete FontName;
   FontName=new GString(x.FontName);
   return *this;
}

void HtmlFont::clear(){
  if(DefaultFont) delete DefaultFont;
}



GBool HtmlFont::isEqual(const HtmlFont& x) const{
  return ((size==x.size)&&(pos==x.pos)&&(color.isEqual(x.getColor())));
  // (bold==x.bold)&&(italic==x.italic))
}

GString* HtmlFont::getFontName(){
   if (pos!=font_num) return new GString(fonts[pos].name);
    else return new GString(DefaultFont);
}

GString* HtmlFont::getFullName(){
  return new GString(FontName);
} 

void HtmlFont::setDefaultFont(GString* defaultFont){
  if (DefaultFont) delete DefaultFont;
  DefaultFont=new GString(defaultFont);
}


GString* HtmlFont::getDefaultFont(){
  return DefaultFont;
}

// this method if plain wrong todo
GString* HtmlFont::HtmlFilter(Unicode* u, int uLen) { //char* s){
  GString *tmp = new GString();
  UnicodeMap *uMap;
  char buf[8];
  int n;
  
  // get the output encoding
  if (!(uMap = globalParams->getTextEncoding())) {
    return NULL;
  }

  for (int i = 0; i < uLen; ++i) {
    if ((n = uMap->mapUnicode(u[i], buf, sizeof(buf))) > 0) {
      tmp->append(buf, n); 
      //fwrite(buf, 1, n, f);
    }
  }
  /*while (*s!='\0')
    { 
      switch (*s)
	{ 
	case '"': tmp->append("&quot;");  break;
	case '&': tmp->append("&amp;");  break;
	case '<': tmp->append("&lt;");  break;
	case '>': tmp->append("&gt;");  break;
	default:  
	  {
	    // convert unicode to string
	    tmp->append(*s);
	  }
	}
      s++;
      }*/
  return tmp;
}

GString* HtmlFont::simple(GString* ftname, Unicode* content, int uLen){
  GString *cont=HtmlFilter (content, uLen); // tuta (content->getCString());
  GString *fontname=new GString(ftname);
  
  GBool b=gFalse;
  GBool i=gFalse;
  
  if (fontname){
  
  if (strstr(fontname->lowerCase()->getCString(),"bold")) b=gTrue;
 
  if (strstr(fontname->lowerCase()->getCString(),"italic")||
      strstr(fontname->lowerCase()->getCString(),"oblique")) i=gTrue;
  }
  if (b) {
    cont->insert(0,"<b>",3);
    cont->append("</b>",4);
  }
  if (i) {
    cont->insert(0,"<i>",3);
    cont->append("</i>",4);
  } 
  delete fontname;
  return cont;
}

HtmlFontAccu::HtmlFontAccu(){
  accu=new GVector<HtmlFont>();
}

HtmlFontAccu::~HtmlFontAccu(){
  if (accu) delete accu;
}

int HtmlFontAccu::AddFont(const HtmlFont& font){
 GVector<HtmlFont>::iterator i; 
 for (i=accu->begin();i!=accu->end();i++)
    if (font.isEqual(*i)) return (int)(i-(accu->begin())) ;
  accu->push_back(font);
  return (accu->size()-1);
}


GString* HtmlFontAccu::getCSStyle(int i,GString* content){
  GString *tmp;
  GString *iStr=GString::IntToStr(i);

  if (!xml) {
    tmp = new GString("<span class=\"ft");
    tmp->append(iStr);
    tmp->append("\">");
    tmp->append(content);
    tmp->append("</span>");
  } else {
    tmp = new GString("");
    tmp->append(content);
  }

  delete iStr;
  return tmp;
}

GString* HtmlFontAccu::CSStyle(int i){
   GString *tmp=new GString();
   GString *iStr=GString::IntToStr(i);

   GVector<HtmlFont>::iterator g=accu->begin();
   g+=i;
   HtmlFont font=*g;
   GString *Size=GString::IntToStr(font.getSize());
   GString *colorStr=font.getColor().toString();
   
   if(!xml){
     tmp->append(".ft");
     tmp->append(iStr);
     tmp->append("{font-size:");
     tmp->append(Size);
     tmp->append("px;font-family:");
     tmp->append(font.getFontName());
     tmp->append(";color:");
     tmp->append(colorStr);
     tmp->append(";}");
   }
   if (xml) {
     tmp->append("<fontspec id=\"");
     tmp->append(iStr);
     tmp->append("\" size=\"");
     tmp->append(Size);
     tmp->append("\" family=\"");
     tmp->append(font.getFontName());
     tmp->append("\" color=\"");
     tmp->append(colorStr);
     tmp->append("\"/>");
   }

   delete colorStr;
   delete iStr;
   delete Size;
   return tmp;
}
 


