//========================================================================
//
// HtmlOutputDev.cc
//
// Copyright 1997 Derek B. Noonburg
//
// Changed 1999-2000 by G.Ovtcharov
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
//#include "GlobalParams.h"
//#include "UnicodeMap.h"
//#include "FontEncoding.h"
#include "HtmlOutputDev.h"
//#include "GVector.h" 
#include "HtmlFonts.h"


//#include "TextOutputFontInfo.h"

#define xoutRound(x) ((int)(x + 0.5))

int HtmlPage::pgNum=0;
int HtmlOutputDev::imgNum=1;
//------------------------------------------------------------------------
// Character substitutions
//------------------------------------------------------------------------

static char *isoLatin1Subst[] = {
  "L",                          // Lslash
  "OE",                         // OE
  "S",                          // Scaron
  "Y",                          // Ydieresis
  "Z",                          // Zcaron
  "fi", "fl",                   // ligatures
  "ff", "ffi", "ffl",           // ligatures
  "i",                          // dotlessi
  "l",                          // lslash
  "oe",                         // oe
  "s",                          // scaron
  "z",                          // zcaron
  "*",                          // bullet
  "...",                        // ellipsis
  "-", "-",                     // emdash, hyphen
  "\"", "\"",                   // quotedblleft, quotedblright
  "'",                          // quotesingle
  "TM"                          // trademark
};

static char *ascii7Subst[] = {
  "A", "A", "A", "A",           // A{acute,circumflex,dieresis,grave}
  "A", "A",                     // A{ring,tilde}
  "AE",                         // AE
  "C",                          // Ccedilla
  "E", "E", "E", "E",           // E{acute,circumflex,dieresis,grave}
  "I", "I", "I", "I",           // I{acute,circumflex,dieresis,grave}
  "L",                          // Lslash
  "N",                          // Ntilde
  "O", "O", "O", "O",           // O{acute,circumflex,dieresis,grave}
  "O", "O",                     // O{slash,tilde}
  "OE",                         // OE
  "S",                          // Scaron
  "U", "U", "U", "U",           // U{acute,circumflex,dieresis,grave}
  "Y", "Y",                     // T{acute,dieresis}
  "Z",                          // Zcaron
  "a", "a", "a", "a",           // a{acute,circumflex,dieresis,grave}
  "a", "a",                     // a{ring,tilde}
  "ae",                         // ae
  "c",                          // ccedilla
  "e", "e", "e", "e",           // e{acute,circumflex,dieresis,grave}
  "fi", "fl",                   // ligatures
  "ff", "ffi", "ffl",           // ligatures
  "i",                          // dotlessi
  "i", "i", "i", "i",           // i{acute,circumflex,dieresis,grave}
  "l",                          // lslash
  "n",                          // ntilde
  "o", "o", "o", "o",           // o{acute,circumflex,dieresis,grave}
  "o", "o",                     // o{slash,tilde}
  "oe",                         // oe
  "s",                          // scaron
  "u", "u", "u", "u",           // u{acute,circumflex,dieresis,grave}
  "y", "y",                     // t{acute,dieresis}
  "z",                          // zcaron
  "|",                          // brokenbar
  "*",                          // bullet
  "...",                        // ellipsis
  "-", "-", "-",                // emdash, endash, hyphen
  "\"", "\"",                   // quotedblleft, quotedblright
  "'",                          // quotesingle
  "(R)",                        // registered
  "TM"                          // trademark
};

//------------------------------------------------------------------------
// 16-bit fonts
//------------------------------------------------------------------------

#if JAPANESE_SUPPORT

// CID 0 .. 96
static Gushort japan12Map[96] = {
  0x2120, 0x2120, 0x212a, 0x2149, 0x2174, 0x2170, 0x2173, 0x2175, // 00 .. 07
  0x2147, 0x214a, 0x214b, 0x2176, 0x215c, 0x2124, 0x213e, 0x2123, // 08 .. 0f
  0x213f, 0x2330, 0x2331, 0x2332, 0x2333, 0x2334, 0x2335, 0x2336, // 10 .. 17
  0x2337, 0x2338, 0x2339, 0x2127, 0x2128, 0x2163, 0x2161, 0x2164, // 18 .. 1f
  0x2129, 0x2177, 0x2341, 0x2342, 0x2343, 0x2344, 0x2345, 0x2346, // 20 .. 27
  0x2347, 0x2348, 0x2349, 0x234a, 0x234b, 0x234c, 0x234d, 0x234e, // 28 .. 2f
  0x234f, 0x2350, 0x2351, 0x2352, 0x2353, 0x2354, 0x2355, 0x2356, // 30 .. 37
  0x2357, 0x2358, 0x2359, 0x235a, 0x214e, 0x216f, 0x214f, 0x2130, // 38 .. 3f
  0x2132, 0x2146, 0x2361, 0x2362, 0x2363, 0x2364, 0x2365, 0x2366, // 40 .. 47
  0x2367, 0x2368, 0x2369, 0x236a, 0x236b, 0x236c, 0x236d, 0x236e, // 48 .. 4f
  0x236f, 0x2370, 0x2371, 0x2372, 0x2373, 0x2374, 0x2375, 0x2376, // 50 .. 57
  0x2377, 0x2378, 0x2379, 0x237a, 0x2150, 0x2143, 0x2151, 0x2141  // 58 .. 5f
};

// CID 325 .. 421
static Gushort japan12KanaMap1[97] = {
  0x2131, 0x2121, 0x2123, 0x2156, 0x2157, 0x2122, 0x2126, 0x2572,
  0x2521, 0x2523, 0x2525, 0x2527, 0x2529, 0x2563, 0x2565, 0x2567,
  0x2543, 0x213c, 0x2522, 0x2524, 0x2526, 0x2528, 0x252a, 0x252b,
  0x252d, 0x252f, 0x2531, 0x2533, 0x2535, 0x2537, 0x2539, 0x253b,
  0x253d, 0x253f, 0x2541, 0x2544, 0x2546, 0x2548, 0x254a, 0x254b,
  0x254c, 0x254d, 0x254e, 0x254f, 0x2552, 0x2555, 0x2558, 0x255b,
  0x255e, 0x255f, 0x2560, 0x2561, 0x2562, 0x2564, 0x2566, 0x2568,
  0x2569, 0x256a, 0x256b, 0x256c, 0x256d, 0x256f, 0x2573, 0x212b,
  0x212c, 0x212e, 0x2570, 0x2571, 0x256e, 0x2575, 0x2576, 0x2574,
  0x252c, 0x252e, 0x2530, 0x2532, 0x2534, 0x2536, 0x2538, 0x253a,
  0x253c, 0x253e, 0x2540, 0x2542, 0x2545, 0x2547, 0x2549, 0x2550,
  0x2551, 0x2553, 0x2554, 0x2556, 0x2557, 0x2559, 0x255a, 0x255c,
  0x255d
};

// CID 501 .. 598
static Gushort japan12KanaMap2[98] = {
  0x212d, 0x212f, 0x216d, 0x214c, 0x214d, 0x2152, 0x2153, 0x2154,
  0x2155, 0x2158, 0x2159, 0x215a, 0x215b, 0x213d, 0x2121, 0x2472,
  0x2421, 0x2423, 0x2425, 0x2427, 0x2429, 0x2463, 0x2465, 0x2467,
  0x2443, 0x2422, 0x2424, 0x2426, 0x2428, 0x242a, 0x242b, 0x242d,
  0x242f, 0x2431, 0x2433, 0x2435, 0x2437, 0x2439, 0x243b, 0x243d,
  0x243f, 0x2441, 0x2444, 0x2446, 0x2448, 0x244a, 0x244b, 0x244c,
  0x244d, 0x244e, 0x244f, 0x2452, 0x2455, 0x2458, 0x245b, 0x245e,
  0x245f, 0x2460, 0x2461, 0x2462, 0x2464, 0x2466, 0x2468, 0x2469,
  0x246a, 0x246b, 0x246c, 0x246d, 0x246f, 0x2473, 0x2470, 0x2471,
  0x246e, 0x242c, 0x242e, 0x2430, 0x2432, 0x2434, 0x2436, 0x2438,
  0x243a, 0x243c, 0x243e, 0x2440, 0x2442, 0x2445, 0x2447, 0x2449,
  0x2450, 0x2451, 0x2453, 0x2454, 0x2456, 0x2457, 0x2459, 0x245a,
  0x245c, 0x245d
};

static char *japan12Roman[10] = {
  "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X"
};

static char *japan12Abbrev1[6] = {
  "mm", "cm", "km", "mg", "kg", "cc"
};

#endif


extern double scale;
extern GBool mode;
extern char extension[5];
extern GBool ignore;
extern GBool printCommands;
extern GBool printHtml;
extern GBool noframes;
extern GBool stout;
extern GBool xml;


//------------------------------------------------------------------------
// HtmlString
//------------------------------------------------------------------------

#ifdef WIN32
#  define SLASH '\\'
#else
#  define SLASH '/'
#endif


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

HtmlString::HtmlString(GfxState *state, GBool hexCodes1,HtmlFontAccu* fonts) {
  double x, y, h;
  int f;
  state->transform(state->getCurX(), state->getCurY(), &x, &y);
  h = state->getTransformedFontSize();
  //~ yMin/yMax computation should use font ascent/descent values
  yMin = y - 0.95 * h;
  yMax = yMin + 1.3 * h;
  col = 0;
  text = NULL; //new GString();
  if (h<7) h=7;
  if (state->getFont()->getName()) {
    GfxRGB rgb;
    state->getFillRGB(&rgb);
    HtmlFont hfont=HtmlFont(state->getFont()->getName(),static_cast<int>(h-1),rgb);
    fontpos=fonts->AddFont(hfont);
  }
  else  fontpos=0;
  xRight = NULL;
  yxNext = NULL;
  xyNext = NULL;
  hexCodes = hexCodes1;
  htext=new GString();
  len = size = 0;
}

HtmlString::~HtmlString() {
  delete text;
  delete htext ;
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

/*void HtmlString::addChar(GfxState *state, double x, double y,
                         double dx, double dy,
                         Guchar c, GBool useASCII7) {
  char *charName, *sub;
  int c1;
  int i, j, n, m;

  // get current index
  i = str->getLength();

  // append translated character(s) to string
  sub = NULL;
  n = 1;
  if ((charName = state->getFont()->getCharName(c))) {
    if (useASCII7)
      c1 = ascii7Encoding.getCharCode(charName);
    else
      c1 = isoLatin1Encoding.getCharCode(charName);
    if (c1 < 0) {
      m = strlen(charName);
      if (hexCodes && m == 3 &&
          (charName[0] == 'B' || charName[0] == 'C' ||
           charName[0] == 'G') &&
          isxdigit(charName[1]) && isxdigit(charName[2])) {
        sscanf(charName+1, "%x", &c1);
      } else if (!hexCodes && m >= 2 && m <= 3 &&
                 isdigit(charName[0]) && isdigit(charName[1])) {
        c1 = atoi(charName);
        if (c1 >= 256)
          c1 = -1;
      } else if (!hexCodes && m >= 3 && m <= 5 && isdigit(charName[1])) {
        c1 = atoi(charName+1);
        if (c1 >= 256)
          c1 = -1;
      }
      //~ this is a kludge -- is there a standard internal encoding
      //~ used by all/most Type 1 fonts?
      if (c1 == 262)            // hyphen
        c1 = 45;
      else if (c1 == 266)       // emdash
        c1 = 208;
      if (useASCII7)
        c1 = ascii7Encoding.getCharCode(isoLatin1Encoding.getCharName(c1));
    }
    if (useASCII7) {
      if (c1 >= 128) {
        sub = ascii7Subst[c1 - 128];
        n = strlen(sub);
      }
    } else {
      if (c1 >= 256) {
        sub = isoLatin1Subst[c1 - 256];
        n = strlen(sub);
      }
    }
  } else {
    c1 = -1;
  }
  if (sub)
    text->append(sub);
  else if (c1 >= 0)
    text->append((char)c1);
  else
    text->append(' ');

  // update position information
  if (i+n > ((i+15) & ~15))
    xRight = (double *)grealloc(xRight, ((i+n+15) & ~15) * sizeof(double));
  if (i == 0)
    xMin = x;
  for (j = 0; j < n; ++j)
    xRight[i+j] = x + ((j+1) * dx) / n;
  xMax = x + dx;
}

void HtmlString::addChar16(GfxState *state, double x, double y,
                           double dx, double dy ) {
			   // int c, GfxFontCharSet16 charSet) {
  int c1, t1, t2;
  int sub[8];
  char *p;
  int *q;
  int i, j, n;

  // get current index
  i = text->getLength();

  // convert the 16-bit character
  c1 = 0;
  sub[0] = 0;
  switch (charSet) {

  // convert Adobe-Japan1-2 to JIS X 0208-1983
  case font16AdobeJapan12:
#if JAPANESE_SUPPORT
    if (c <= 96) {
      c1 = 0x8080 + japan12Map[c];
    } else if (c <= 632) {
      if (c <= 230)
        c1 = 0;
      else if (c <= 324)
        c1 = 0x8080 + japan12Map[c - 230];
      else if (c <= 421)
        c1 = 0x8080 + japan12KanaMap1[c - 325];
      else if (c <= 500)
        c1 = 0;
      else if (c <= 598)
        c1 = 0x8080 + japan12KanaMap2[c - 501];
      else
        c1 = 0;
    } else if (c <= 1124) {
      if (c <= 779) {
        if (c <= 726)
          c1 = 0xa1a1 + (c - 633);
        else if (c <= 740)
          c1 = 0xa2a1 + (c - 727);
        else if (c <= 748)
          c1 = 0xa2ba + (c - 741);
        else if (c <= 755)
          c1 = 0xa2ca + (c - 749);
        else if (c <= 770)
          c1 = 0xa2dc + (c - 756);
        else if (c <= 778)
          c1 = 0xa2f2 + (c - 771);
        else
          c1 = 0xa2fe;
      } else if (c <= 841) {
        if (c <= 789)
          c1 = 0xa3b0 + (c - 780);
        else if (c <= 815)
          c1 = 0xa3c1 + (c - 790);
        else
          c1 = 0xa3e1 + (c - 816);
      } else if (c <= 1010) {
        if (c <= 924)
          c1 = 0xa4a1 + (c - 842);
        else
          c1 = 0xa5a1 + (c - 925);
      } else {
        if (c <= 1034)
          c1 = 0xa6a1 + (c - 1011);
        else if (c <= 1058)
          c1 = 0xa6c1 + (c - 1035);
        else if (c <= 1091)
          c1 = 0xa7a1 + (c - 1059);
        else
          c1 = 0xa7d1 + (c - 1092);
      }
    } else if (c <= 4089) {
      t1 = (c - 1125) / 94;
      t2 = (c - 1125) % 94;
      c1 = 0xb0a1 + (t1 << 8) + t2;
    } else if (c <= 7477) {
      t1 = (c - 4090) / 94;
      t2 = (c - 4090) % 94;
      c1 = 0xd0a1 + (t1 << 8) + t2;
    } else if (c <= 7554) {
      c1 = 0;
    } else if (c <= 7563) {     // circled Arabic numbers 1..9
      c1 = 0xa3b1 + (c - 7555);
    } else if (c <= 7574) {     // circled Arabic numbers 10..20
      t1 = c - 7564 + 10;
      sub[0] = 0xa3b0 + (t1 / 10);
      sub[1] = 0xa3b0 + (t1 % 10);
      sub[2] = 0;
      c1 = -1;
    } else if (c <= 7584) {     // Roman numbers I..X
      for (p = japan12Roman[c - 7575], q = sub; *p; ++p, ++q) {
        *q = 0xa380 + *p;
      }
      *q = 0;
      c1 = -1;
    } else if (c <= 7632) {
      if (c <= 7600) {
        c1 = 0;
      } else if (c <= 7606) {
        for (p = japan12Abbrev1[c - 7601], q = sub; *p; ++p, ++q) {
          *q = 0xa380 + *p;
        }
        *q = 0;
        c1 = -1;
      } else {
        c1 = 0;
      }
    } else {
      c1 = 0;
    }
#endif // JAPANESE_SUPPORT
    break;
  }

  // append converted character to string
  if (c1 == 0) {
#if 0 //~
    error(-1, "Unsupported Adobe-Japan1-2 character: %d", c);
#endif
    text->append(' ');
    n = 1;
  } else if (c1 > 0) {
    text->append(c1 >> 8);
    text->append(c1 & 0xff);
    n = 2;
  } else {
    n = 0;
    for (q = sub; *q; ++q) {
      text->append(*q >> 8);
      text->append(*q & 0xff);
      n += 2;
    }
  }

  // update position information
  if (i+n > ((i+15) & ~15)) {
    xRight = (double *)grealloc(xRight, ((i+n+15) & ~15) * sizeof(double));
  }
  if (i == 0) {
    xMin = x;
  }
  for (j = 0; j < n; ++j) {
    xRight[i+j] = x + dx;
  }
  xMax = x + dx;
}
*/




//------------------------------------------------------------------------
// HtmlPage
//------------------------------------------------------------------------

HtmlPage::HtmlPage(GBool useASCII7, GBool rawOrder) {
  this->useASCII7 = useASCII7;
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

void HtmlPage::beginString(GfxState *state, GString *s, GBool hexCodes) {
  curStr = new HtmlString(state, hexCodes,fonts);
  if(!pageWidth) {
      pageWidth=static_cast<int>(state->getPageWidth());
      pageHeight=static_cast<int>(state->getPageHeight());
  }
}


void HtmlPage::conv(){
  HtmlString *tmp;
  GString* str;
  int k=0;
  HtmlFont h;
  for(tmp=yxStrings;tmp;tmp=tmp->yxNext){
     int pos=tmp->fontpos;
     //  printf("%d\n",pos);
     h=fonts->Get(pos);
     str=h.getFullName();
     if (tmp->htext) delete tmp->htext; 
     tmp->htext=HtmlFont::simple(str,tmp->text,tmp->len);
     delete str; 
     if (links->inLink(tmp->xMin,tmp->yMin,tmp->xMax,tmp->yMax,k)){
       GString *t=tmp->htext;
       tmp->htext=links->getLink(k)->Link(tmp->htext);
       delete t;
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
    beginString(state, NULL, false);
  }
  state->textTransformDelta(state->getCharSpace() * state->getHorizScaling(),
			    0, &dx2, &dy2);
  dx -= dx2;
  dy -= dy2;
  state->transformDelta(dx, dy, &w1, &h1);
  w1 /= uLen;
  h1 /= uLen;
  for (i = 0; i < uLen; ++i) {
    curStr->addChar(state, x1 + i*w1, y1 + i*h1, w1, h1, u[i]);
  }
}

/*
void HtmlPage::addChar16(GfxState *state, double x, double y,
                         double dx, double dy, int c,
                         GfxFontCharSet16 charSet) {
  double x1, y1, w1, h1, dx2, dy2;
  int n;
  GBool hexCodes;

  state->transform(x, y, &x1, &y1);
  state->textTransformDelta(state->getCharSpace(), 0, &dx2, &dy2);
  dx -= dx2;
  dy -= dy2;
  state->transformDelta(dx, dy, &w1, &h1);
  n = curStr->text->getLength();
  if (n > 0 &&
      x1 - curStr->xRight[n-1] > 0.1 * (curStr->yMax - curStr->yMin)) {
    hexCodes = curStr->hexCodes;
    endString();
    beginString(state, NULL, hexCodes);
  }
  curStr->addChar16(state, x1, y1, w1, h1, c, charSet);
}
*/
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
  double space, d;
  GBool addSpace;
  int n, i;

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
  while (str1 && (str2 = str1->yxNext)) {
    space = str1->yMax - str1->yMin;
    d = str2->xMin - str1->xMax;
    if (((rawOrder &&
	  ((str2->yMin >= str1->yMin && str2->yMin <= str1->yMax) ||
	   (str2->yMax >= str1->yMin && str2->yMax <= str1->yMax))) ||
	 (!rawOrder && str2->yMin < str1->yMax)) &&
	d > -0.5 * space && d < space &&
	(str1->fontpos==str2->fontpos)
	) {
      n = str1->len + str2->len;
      if ((addSpace = d > 0.1 * space)) {
	++n;
      }
      str1->size = (n + 15) & ~15;
      str1->text = (Unicode *)grealloc(str1->text,
				       str1->size * sizeof(Unicode));
      str1->xRight = (double *)grealloc(str1->xRight,
					str1->size * sizeof(double));
      if (addSpace) {
	str1->text[str1->len] = 0x20;
	str1->htext->append(" \n");
	str1->xRight[str1->len] = str2->xMin;
	++str1->len;
      }
      for (i = 0; i < str2->len; ++i) {
	str1->text[str1->len] = str2->text[i];
	str1->xRight[str1->len] = str2->xRight[i];
	++str1->len;
      }
      str1->htext->append(str2->htext);
      if (str2->xMax > str1->xMax) {
	str1->xMax = str2->xMax;
      }
      if (str2->yMax > str1->yMax) {
	str1->yMax = str2->yMax;
      }
      str1->yxNext = str2->yxNext;
      delete str2;
    } else {
      str1 = str2;
    }
  }
}

/*void HtmlPage::coalesce() {
  HtmlString *str1, *str2;
  double space, d;
  int n, i;

#if 0 //~ for debugging
  for (str1 = yxStrings; str1; str1 = str1->yxNext) {
    printf("x=%3d..%3d  y=%3d..%3d  size=%2d '%s'\n",
           (int)str1->xMin, (int)str1->xMax, (int)str1->yMin, (int)str1->yMax,
           (int)(str1->yMax - str1->yMin), str1->text->getCString());
  }
  printf("\n------------------------------------------------------------\n\n");
#endif
  str1 = yxStrings;
  while (str1 && (str2 = str1->yxNext)) {
    space = str1->yMax - str1->yMin;
    d = str2->xMin - str1->xMax;
#if 0 //~tmp
    if (((rawOrder &&
          ((str2->yMin >= str1->yMin && str2->yMin <= str1->yMax) ||
           (str2->yMax >= str1->yMin && str2->yMax <= str1->yMax))) ||
         (!rawOrder && str2->yMin < str1->yMax)) &&
        d > -0.1 * space && d < 0.2 * space &&(str1->fontpos==str2->fontpos)){
#else
    if (((rawOrder &&
          ((str2->yMin >= str1->yMin && str2->yMin <= str1->yMax) ||
           (str2->yMax >= str1->yMin && str2->yMax <= str1->yMax))) ||
         (!rawOrder && str2->yMin < str1->yMax)) &&
        d > -0.5 * space && d < space&&(str1->fontpos==str2->fontpos)) {
#endif
      n = str1->len;
      
            
      if (d > 0.1 * space){
	str1->text->append(" ");
        str1->htext->append(" \n");
      }
      
        str1->text->append(str2->text);
        str1->htext->append(str2->htext);
        str1->xRight = (double *)
	grealloc(str1->xRight, str1->len * sizeof(double));
      if (d > 0.1 * space)
	str1->xRight[n++] = str2->xMin;
      for (i = 0; i < str2->len; ++i)
	str1->xRight[n++] = str2->xRight[i];
      if (str2->xMax > str1->xMax)
	str1->xMax = str2->xMax;
      if (str2->yMax > str1->yMax)
	str1->yMax = str2->yMax;
      str1->yxNext = str2->yxNext;
      delete str2;
      
    } else {
      str1 = str2;
    }
          
 }
}
*/

void HtmlPage::dumpAsXML(FILE* f,int page){  
  fprintf(f, "<page number=\"%d\" position=\"absolute\"", page);
  fprintf(f," top=\"0\" left=\"0\" height=\"%d\" width=\"%d\">\n", pageHeight,pageWidth);
    
  for(int i=0;i!=fonts->size();i++)
    fprintf(f,"\t%s\n",fonts->CSStyle(i)->getCString());
  
  GString* str;
  for(HtmlString *tmp=yxStrings;tmp;tmp=tmp->yxNext){
    if (tmp->htext){
      str=new GString(tmp->htext);
      fprintf(f,"<text top=\"%d\" left=\"%d\" ",xoutRound(tmp->yMin),xoutRound(tmp->xMin));
      fprintf(f,"width=\"%d\" height=\"%d\" ",xoutRound(tmp->xMax-tmp->xMin),xoutRound(tmp->yMax-tmp->yMin));
      fprintf(f,"font=\"%d\">", tmp->fontpos);
      if (tmp->fontpos!=-1){
	str=fonts->getCSStyle(tmp->fontpos,str);
      }
      fputs(str->getCString(),f);
      delete str;      
      fputs("</text>\n",f);
    }
  }
  fputs("</page>\n",f);
}


void HtmlPage::dumpComplex(int page){
  FILE* f;
  /*UnicodeMap *uMap;
  char buf[8];
  int n;
  
  // get the output encoding
  if (!(uMap = globalParams->getTextEncoding())) {
    return;
    }*/
  GString* tmp=new GString(DocName);   
  GString* pgNum=GString::IntToStr(page);
  tmp->append('-')->append(pgNum)->append(".html");
  delete pgNum;

  if (!(f = fopen(tmp->getCString(), "w"))) {
    error(-1, "Couldn't open html file '%s'", tmp->getCString());
    delete tmp;
    return;
  }    
  delete tmp;
  tmp=basename(DocName);
  
  fprintf(f,"<html>\n<head>\n<title>Page %d</title>\n\n",page);
   
  
  fputs("<style type=\"text/css\">\n<!--\n",f);
    
  for(int i=0;i!=fonts->size();i++)
    fprintf(f,"\t%s\n",fonts->CSStyle(i)->getCString());
    
  fputs("-->\n</style>\n",f);
  fputs("</head>\n<body vlink=\"blue\" link=\"blue\">\n",f); 

  fputs("<div style=\"position:absolute;top:0;left:0\">",f);
  fprintf(f,"<img width=\"%d\" height=\"%d\" src=\"%s%03d.png\">",pageWidth,pageHeight,tmp->getCString(),page);
  fputs("</div>",f);
  
  delete tmp;
  
  GString* str;
  for(HtmlString *tmp1=yxStrings;tmp1;tmp1=tmp1->yxNext){
    if (tmp1->htext){
      str=new GString(tmp1->htext);
      fprintf(f,"<div style=\"position:absolute;top:%d;left:%d\">",xoutRound(tmp1->yMin),xoutRound(tmp1->xMin));
      fputs("<nobr>",f); 
      if (tmp1->fontpos!=-1){
	str=fonts->getCSStyle(tmp1->fontpos,str);  
      }
      fputs(str->getCString(),f);
      
      /*// print the string 
      for (int i = 0; i < tmp1->len; ++i) {
	if ((n = uMap->mapUnicode(tmp1->text[i], buf, sizeof(buf))) > 0) {
	  fwrite(buf, 1, n, f);
	}
	}*/

      delete str;      
      fputs("</nobr></div>\n",f);
    }
  }
  fputs("</body>\n</html>\n",f);
  fclose(f);
}


void HtmlPage::dump(FILE *f) {
  static int nump=0;
  /*UnicodeMap *uMap;
  char buf[8];
  int n;
  
  // get the output encoding
  if (!(uMap = globalParams->getTextEncoding())) {
    return;
    }*/

  nump++;
  if (mode){
    if (xml) dumpAsXML(f,nump);
    if (!xml) dumpComplex(nump);  
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
	/*// print the string 
	for (int i = 0; i < tmp->len; ++i) {
	  if ((n = uMap->mapUnicode(tmp->text[i], buf, sizeof(buf))) > 0) {
	    fwrite(buf, 1, n, f);
	  }
	  }*/

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
  delete fonts;
  delete links;

  fonts=new HtmlFontAccu();
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
  fprintf(f,"<title>%s</title>",fName->getCString());
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

HtmlOutputDev::HtmlOutputDev(char *fileName, GBool useASCII7, GBool rawOrder) {
  f=NULL;
  pages = NULL;
  dumpJPEG=gTrue;
  write = gTrue;
  this->rawOrder = rawOrder;
  ok = gTrue;
  imgNum=1;
  pageNum=1;
  // open file
  needClose = gFalse;
  pages = new HtmlPage(useASCII7, rawOrder);

  pages->setDocName(fileName);
  Docname=new GString (fileName);

  GString *tmp=basename(Docname);
  if (tmp->getLength()==0) {
    printf("Error : illegal output file");
    exit(1);
  }

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
     
     GString* right=new GString(fileName);
     right->append("s.html");
     if(!mode){
       if (!(page=fopen(right->getCString(),"w"))){
        delete right;
        error(-1, "Couldn't open html file '%s'", right->getCString());
	return;
       }
     delete right;
     fputs("<html>\n<head>\n<title></title>\n</head>\n<body>\n",page);
    }
  }

  if (noframes){
     if (stout) page=stdout;
     else{
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
       fputs("<?XML version=\"1.0\" encoding=\"iso-8859-1\"?>\n", page);
       fputs("<!DOCTYPE pdf2xml PUBLIC \"pdf2xml\">\n\n", page);
       fputs("<pdf2xml>\n",page);
     } else {
     
       fprintf(page,"<html>\n<head>\n<title>%s</title>\n</head>\n",tmp->getCString());
       fputs("<body bgcolor=\"#A0A0A0>\" vlink=\"blue\" link=\"blue\">\n",page);
     }
   }    

  delete tmp;
}

HtmlOutputDev::~HtmlOutputDev() {
  if (mode&&!xml){
    int h=xoutRound(pages->pageHeight/scale);
    int w=xoutRound(pages->pageWidth/scale);
    fprintf(tin,"%s=%03d\n","PAPER_WIDTH",w);
    fprintf(tin,"%s=%03d\n","PAPER_HEIGHT",h);
    fclose(tin);
  }

    HtmlFont::clear(); 
    
    delete Docname;
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
  if (mode&&!xml){
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
  }
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
    delete str;
} 


void HtmlOutputDev::endPage() {
  pages->conv();
  pages->coalesce();
  pages->dump(page);
  if(!noframes&&!xml) fputs("<br>", f);
  if(!stout) printf("Page-%d\n",(pageNum));
  pageNum++ ;
}

/*void HtmlOutputDev::updateFont(GfxState *state) {
  GfxFont *font;
  char *charName;
  int c;

  // look for hex char codes in subsetted font
  hexCodes = gFalse;
  if ((font = state->getFont()) && !font->is16Bit()) {
    for (c = 0; c < 256; ++c) {
      if ((charName = font->getCharName(c))) {
	if ((charName[0] == 'B' || charName[0] == 'C' ||
	     charName[0] == 'G') &&
	    strlen(charName) == 3 &&
	    ((charName[1] >= 'a' && charName[1] <= 'f') ||
	     (charName[1] >= 'A' && charName[1] <= 'F') ||
	     (charName[2] >= 'a' && charName[2] <= 'f') ||
	     (charName[2] >= 'A' && charName[2] <= 'F'))) {
	  hexCodes = gTrue;
	  break;
	}
      }
    }
  }
}
*/

void HtmlOutputDev::beginString(GfxState *state, GString *s) {
  pages->beginString(state, s, hexCodes);
}

void HtmlOutputDev::endString(GfxState *state) {
  pages->endString();
}

void HtmlOutputDev::drawChar(GfxState *state, double x, double y,
	      double dx, double dy,
	      double originX, double originY,
	      CharCode code, Unicode *u, int uLen) 
{
  pages->addChar(state, x, y, dx, dy, u, uLen);
}

/*void HtmlOutputDev::drawChar16(GfxState *state, double x, double y,
			       double dx, double dy, int c) {
  pages->addChar16(state, x, y, dx, dy, c, state->getFont()->getCharSet16());
}
*/

void HtmlOutputDev::drawImageMask(GfxState *state, Object *ref, Stream *str,
			      int width, int height, GBool invert,
			      GBool inlineImg) {
if (ignore||mode) return;
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
  int i, j;

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
}

void HtmlOutputDev::drawImage(GfxState *state, Object *ref, Stream *str,
			  int width, int height, GfxImageColorMap *colorMap,
			  int *maskColors, GBool inlineImg) {

if (ignore||mode) return;
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
  int i, j;

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
    else dest=ha->getDest();
    if (dest){ 
      if (dest->isPageRef()){
	Ref pageref=dest->getPageRef();
	page=catalog->findPage(pageref.num,pageref.gen);
      }
      else  page=dest->getPageNum();
      
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
    if (ha->getDest()!=NULL)  dest=ha->getDest();
    if (dest&&file){
      if (!(dest->isPageRef()))  page=dest->getPageNum();
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
