/*
 * Copyright (c) 2001,2002,2003 Mike Matsnev.  All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice immediately at the beginning of the file, without modification,
 *    this list of conditions, and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Absolutely no warranty of function or purpose is made by the author
 *    Mike Matsnev.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * $Id: Unicode.cpp,v 1.13.2.5 2003/12/17 12:19:58 mike Exp $
 * 
 */

#include <afx.h>

#include "ptr.h"
#include "Unicode.h"

#ifndef	CP_UTF8
#define CP_UTF8	65001
#endif

#ifndef CP_1252
#define	CP_1252 1252
#endif

// string compare
#define	CmpI(s1,s2) \
    (::CompareString(LOCALE_USER_DEFAULT,NORM_IGNORECASE, \
    (s1),-1,(s2),-1)-2)

struct CodePage {
  const TCHAR	*name;
  const TCHAR	*alias1,*alias2;
  UINT		codepage;
  int		(*length)(struct CodePage *cp,const char *mbs,int mblen);
  void		(*convert)(struct CodePage *cp,const char *mbs,int mblen,
      wchar_t *wcs,int wclen);
  const wchar_t	*table;
};

static int    CE_cp_length(struct CodePage *cp,const char *mbs,int mblen) {
  return MultiByteToWideChar(cp->codepage,0,mbs,mblen,0,NULL);
}

static void   CE_cp_convert(struct CodePage *cp,const char *mbs,int mblen,
			    wchar_t *wcs,int wclen)
{
  MultiByteToWideChar(cp->codepage,0,mbs,mblen,wcs,wclen);
}

static int    TB_cp_length(struct CodePage *cp,const char *mbs,int mblen) {
  return mblen;
}

static void   TB_cp_convert(struct CodePage *cp,const char *mbs,int mblen,
			    wchar_t *wcs,int wclen)
{
  const char  *mbe=mbs+min(mblen,wclen);
  while (mbs<mbe)
    *wcs++=cp->table[(unsigned char)*mbs++];
}

static int    WS_cp_length(struct CodePage *cp,const char *mbs,int mblen) {
  return mblen;
}

static void   WS_cp_convert(struct CodePage *cp,const char *mbs,int mblen,
			    wchar_t *wcs,int wclen)
{
  const char  *mbe=mbs+min(mblen,wclen);
  while (mbs<mbe)
    *wcs++=(unsigned char)*mbs++;
}

static int    UTF_cp_length(struct CodePage *cp,const char *mbs,int mblen) {
  const unsigned char *mb=(const unsigned char *)mbs;
  int		      len=0;

  while (mblen>0) {
    unsigned char c=*mb++;
    --mblen;
    if (c<0x80) { // ascii
      ++len;
    } else if (c<0xe0) { // 2-byte seq
      if (mblen==0) // invalid
	break;
      if ((mb[0]&0x80)==0x80) {
	++len;
	++mb;
	--mblen;
      }
    } else if (*mb<0xf0) {
      if (mblen<=1) // invalid
	break;
      if ((mb[0]&0xc0)==0x80 && (mb[1]&0xc0)==0x80) {
	++len;
	mb+=2;
	mblen-=2;
      }
    } else if (*mb<0xf4) {
      if (mblen<=2) // invalid
	break;
      if ((mb[0]&0xc0)==0x80 && (mb[1]&0xc0)==0x80 && (mb[2]&0xc0)==0x80) {
	++len;
	mb+=3;
	mblen-=3;
      }
    } else if (*mb==0xf4) {
      if (mblen<=2) // invalid
	break;
      if ((mb[0]&0xf0)==0x80 && (mb[1]&0xc0)==0x80 && (mb[2]&0xc0)==0x80) {
	++len;
	mb+=3;
	mblen-=3;
      }
    }
  }
  return len;
}

static void   UTF_cp_convert(struct CodePage *cp,const char *mbs,int mblen,
			    wchar_t *wcs,int wclen)
{
  const unsigned char *mb=(const unsigned char *)mbs;
  int		      len=0;
  wchar_t	      *wce=wcs+wclen;

  while (mblen>0 && wcs<wce) {
    unsigned char c=*mb++;
    --mblen;
    if (c<0x80) { // ascii
      *wcs++=c;
    } else if (c<0xe0) { // 2-byte seq
      if (mblen==0) // invalid
	break;
      if ((mb[0]&0x80)==0x80) {
	*wcs++=((wchar_t)(c&0x1f)<<6)|(*mb&0x3f);
	++mb;
	--mblen;
      }
    } else if (*mb<0xf0) {
      if (mblen<=1) // invalid
	break;
      if ((mb[0]&0xc0)==0x80 && (mb[1]&0xc0)==0x80) {
	*wcs++=((wchar_t)(c&0x0f)<<12)|((wchar_t)(mb[0]&0x3f)<<6)|(mb[1]&0x3f);
	mb+=2;
	mblen-=2;
      }
    } else if (*mb<0xf4) {
      if (mblen<=2) // invalid
	break;
      if ((mb[0]&0xc0)==0x80 && (mb[1]&0xc0)==0x80 && (mb[2]&0xc0)==0x80) {
	*wcs++=((wchar_t)(c&0x7)<<18)|((wchar_t)(mb[0]&0x3f)<<12)|
		((wchar_t)(mb[1]&0x3f)<<6)|(mb[2]&0x3f);
	mb+=3;
	mblen-=3;
      }
    } else if (*mb==0xf4) {
      if (mblen<=2) // invalid
	break;
      if ((mb[0]&0xf0)==0x80 && (mb[1]&0xc0)==0x80 && (mb[2]&0xc0)==0x80) {
	*wcs++=((wchar_t)(c&0x7)<<18)|((wchar_t)(mb[0]&0x3f)<<12)|
		((wchar_t)(mb[1]&0x3f)<<6)|(mb[2]&0x3f);
	mb+=3;
	mblen-=3;
      }
    }
  }
}

static struct CodePage	  *codepages;
static int		  curcp,maxcp;
static int		  default_cp;

static int    add_codepage(const TCHAR *name,UINT cp,const TCHAR *alias1=NULL,
			   const TCHAR *alias2=NULL)
{
  if (curcp>=maxcp) {
    maxcp+=16;
    codepages=(struct CodePage *)realloc(codepages,maxcp*sizeof(struct CodePage));
    if (codepages==NULL)
      ExitThread(0);
  }
  codepages[curcp].name=name;
  codepages[curcp].alias1=alias1;
  codepages[curcp].alias2=alias2;
  codepages[curcp].codepage=cp;
  codepages[curcp].length=CE_cp_length;
  codepages[curcp].convert=CE_cp_convert;
  codepages[curcp].table=NULL;
  return curcp++;
}

struct {
  UINT	      cp;
  const TCHAR *name;
  const TCHAR *alias1,*alias2;
} ms_codepages[]={
  { 37, _T("IBM EBCDIC - U.S./Canada"), _T("IBM037"), _T("cp037") },
  { 437, _T("OEM - United States"), _T("IBM437"), _T("cp437") },
  { 500, _T("IBM EBCDIC - International"), _T("IBM500"), _T("cp500") },
  { 737, _T("OEM - Greek 437G") },
  { 775, _T("OEM - Baltic"), _T("IBM775"), _T("cp775") },
  { 850, _T("OEM - Multilingual Latin I"), _T("IBM850"), _T("cp850") },
  { 852, _T("OEM - Latin II"), _T("IBM852"), _T("cp852") },
  { 855, _T("OEM - Cyrillic"), _T("IBM855"), _T("cp855") },
  { 857, _T("OEM - Turkish"), _T("IBM857"), _T("cp857") },
  { 860, _T("OEM - Portuguese"), _T("IBM860"), _T("cp860") },
  { 861, _T("OEM - Icelandic"), _T("IBM861"), _T("cp861") },
  { 863, _T("OEM - Canadian French"), _T("IBM863"), _T("cp863") },
  { 865, _T("OEM - Nordic"), _T("IBM865"), _T("cp865") },
  { 866, _T("OEM - Russian"), _T("IBM866"), _T("cp866") },
  { 869, _T("OEM - Modern Greek"), _T("IBM869"), _T("cp869") },
  { 874, _T("ANSI/OEM - Thai") },
  { 875, _T("IBM EBCDIC - Modern Greek") },
  { 932, _T("ANSI/OEM - Japanese Shift-JIS"), _T("Shift_JIS") },
  { 936, _T("ANSI/OEM - Simplified Chinese GBK"), _T("GBK") },
  { 949, _T("ANSI/OEM - Korean") },
  { 950, _T("ANSI/OEM - Traditional Chinese Big5"), _T("Big5") },
  { 1026, _T("IBM EBCDIC - Turkish (Latin-5)"), _T("cp1026") },
  { 1250, _T("ANSI - Central Europe"), _T("windows-1250") },
  { 1251, _T("ANSI - Cyrillic"), _T("windows-1251") },
  { 1252, _T("ANSI - Latin I"), _T("windows-1252"), _T("ISO-8859-1") }, // XXX
  { 1253, _T("ANSI - Greek"), _T("windows-1253") },
  { 1254, _T("ANSI - Turkish"), _T("windows-1254") },
  { 1255, _T("ANSI - Hebrew"), _T("windows-1255") },
  { 1256, _T("ANSI - Arabic"), _T("windows-1256") },
  { 1257, _T("ANSI - Baltic"), _T("windows-1257") },
  { 1258, _T("ANSI/OEM - Viet Nam"), _T("windows-1258") },
  { 10000, _T("MAC - Roman") },
  { 10006, _T("MAC - Greek I") },
  { 10007, _T("MAC - Cyrillic") },
  { 10010, _T("MAC - Romania") },
  { 10017, _T("MAC - Ukraine") },
  { 10029, _T("MAC - Latin II") },
  { 10079, _T("MAC - Icelandic") },
  { 10081, _T("MAC - Turkish") },
  { 10082, _T("MAC - Croatia") },
  { 20127, _T("US-ASCII") },
  { 20261, _T("T.61") },
  { 20866, _T("Russian - KOI8"), _T("koi8-r") },
  { 21866, _T("Ukrainian - KOI8-U"), _T("koi8-u") },
  { 28591, _T("ISO 8859-1 Latin I"), _T("ISO-8859-1") },
  { 28592, _T("ISO 8859-2 Central Europe"), _T("ISO-8859-2") },
  { 28594, _T("ISO 8859-4 Baltic"), _T("ISO-8859-4") },
  { 28595, _T("ISO 8859-5 Cyrillic"), _T("ISO-8859-5") },
  { 28597, _T("ISO 8859-7 Greek"), _T("ISO-8859-7") },
  { 28599, _T("ISO 8859-9 Latin 5"), _T("ISO-8859-9") },
  { 28605, _T("ISO 8859-15 Latin 9"), _T("ISO-8859-15") },
  { 65000, _T("UTF-7") },
  { 65001, _T("UTF-8") },
};
#define	NUM_MSCP    (sizeof(ms_codepages)/sizeof(ms_codepages[0]))

static int  get_mscp_num(UINT cp) {
  int	  i=0,j=NUM_MSCP-1;
  while (i<=j) {
    int	  m=(i+j)>>1;
    if (cp<ms_codepages[m].cp)
      j=m-1;
    else if (cp>ms_codepages[m].cp)
      i=m+1;
    else
      return m;
  }
  return -1;
}

static struct {
  BYTE	  distmap[256];
  wchar_t unimap[256];
  UINT	  cp;
} builtin_encodings[]={
{{
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03,
0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
0x1e, 0x1f, 0x20, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
},{
0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009,
0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f, 0x0010, 0x0011, 0x0012, 0x0013,
0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d,
0x001e, 0x001f, 0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f, 0x0030, 0x0031,
0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b,
0x003c, 0x003d, 0x003e, 0x003f, 0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045,
0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059,
0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f, 0x0060, 0x0061, 0x0062, 0x0063,
0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d,
0x006e, 0x006f, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f, 0x0402, 0x0403,
0x201a, 0x0453, 0x201e, 0x2026, 0x2020, 0x2021, 0x20ac, 0x2030, 0x0409, 0x2039,
0x040a, 0x040c, 0x040b, 0x040f, 0x0452, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022,
0x2013, 0x2014, 0x00a0, 0x2122, 0x0459, 0x203a, 0x045a, 0x045c, 0x045b, 0x045f,
0x00a0, 0x040e, 0x045e, 0x0408, 0x00a4, 0x0490, 0x00a6, 0x00a7, 0x0401, 0x00a9,
0x0404, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x0407, 0x00b0, 0x00b1, 0x0406, 0x0456,
0x0491, 0x00b5, 0x00b6, 0x00b7, 0x0451, 0x2116, 0x0454, 0x00bb, 0x0458, 0x0405,
0x0455, 0x0457, 0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
0x0418, 0x0419, 0x041a, 0x041b, 0x041c, 0x041d, 0x041e, 0x041f, 0x0420, 0x0421,
0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428, 0x0429, 0x042a, 0x042b,
0x042c, 0x042d, 0x042e, 0x042f, 0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435,
0x0436, 0x0437, 0x0438, 0x0439, 0x043a, 0x043b, 0x043c, 0x043d, 0x043e, 0x043f,
0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0449,
0x044a, 0x044b, 0x044c, 0x044d, 0x044e, 0x044f,
},1251},
{{
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x01, 0x02,
0x17, 0x05, 0x06, 0x15, 0x04, 0x16, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
0x10, 0x20, 0x11, 0x12, 0x13, 0x14, 0x07, 0x03, 0x1d, 0x1c, 0x08, 0x19, 0x1e,
0x1a, 0x18, 0x1b, 0x1f, 0x01, 0x02, 0x17, 0x05, 0x06, 0x15, 0x04, 0x16, 0x09,
0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x20, 0x11, 0x12, 0x13, 0x14, 0x07,
0x03, 0x1d, 0x1c, 0x08, 0x19, 0x1e, 0x1a, 0x18, 0x1b,
},{
0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
0x2500, 0x2502, 0x250c, 0x2510, 0x2514, 0x2518, 0x251c, 0x2524,
0x252c, 0x2534, 0x253c, 0x2580, 0x2584, 0x2588, 0x258c, 0x2590,
0x2591, 0x2592, 0x2593, 0x2320, 0x25a0, 0x2219, 0x221a, 0x2248,
0x2264, 0x2265, 0x00a0, 0x2321, 0x00b0, 0x00b2, 0x00b7, 0x00f7,
0x2550, 0x2551, 0x2552, 0x0451, 0x2553, 0x2554, 0x2555, 0x2556,
0x2557, 0x2558, 0x2559, 0x255a, 0x255b, 0x255c, 0x255d, 0x255e,
0x255f, 0x2560, 0x2561, 0x0401, 0x2562, 0x2563, 0x2564, 0x2565,
0x2566, 0x2567, 0x2568, 0x2569, 0x256a, 0x256b, 0x256c, 0x00a9,
0x044e, 0x0430, 0x0431, 0x0446, 0x0434, 0x0435, 0x0444, 0x0433,
0x0445, 0x0438, 0x0439, 0x043a, 0x043b, 0x043c, 0x043d, 0x043e,
0x043f, 0x044f, 0x0440, 0x0441, 0x0442, 0x0443, 0x0436, 0x0432,
0x044c, 0x044b, 0x0437, 0x0448, 0x044d, 0x0449, 0x0447, 0x044a,
0x042e, 0x0410, 0x0411, 0x0426, 0x0414, 0x0415, 0x0424, 0x0413,
0x0425, 0x0418, 0x0419, 0x041a, 0x041b, 0x041c, 0x041d, 0x041e,
0x041f, 0x042f, 0x0420, 0x0421, 0x0422, 0x0423, 0x0416, 0x0412,
0x042c, 0x042b, 0x0417, 0x0428, 0x042d, 0x0429, 0x0427, 0x042a,
}, 20866},
{{
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02,
0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c,
0x1d, 0x1e, 0x1f, 0x20, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,
0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x06, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
},{
0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
0x0418, 0x0419, 0x041a, 0x041b, 0x041c, 0x041d, 0x041e, 0x041f,
0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427,
0x0428, 0x0429, 0x042a, 0x042b, 0x042c, 0x042d, 0x042e, 0x042f,
0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437,
0x0438, 0x0439, 0x043a, 0x043b, 0x043c, 0x043d, 0x043e, 0x043f,
0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
0x2555, 0x2563, 0x2551, 0x2557, 0x255d, 0x255c, 0x255b, 0x2510,
0x2514, 0x2534, 0x252c, 0x251c, 0x2500, 0x253c, 0x255e, 0x255f,
0x255a, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256c, 0x2567,
0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256b,
0x256a, 0x2518, 0x250c, 0x2588, 0x2584, 0x258c, 0x2590, 0x2580,
0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447,
0x0448, 0x0449, 0x044a, 0x044b, 0x044c, 0x044d, 0x044e, 0x044f,
0x0401, 0x0451, 0x0404, 0x0454, 0x0407, 0x0457, 0x040e, 0x045e,
0x00b0, 0x2219, 0x00b7, 0x221a, 0x2116, 0x00a4, 0x25a0, 0x00a0,
}, 866},
{{
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
   17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
   17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    0,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
},{
  0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,
  0x0008,0x0009,0x000a,0x000b,0x000c,0x000d,0x000e,0x000f,
  0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,
  0x0018,0x0019,0x001a,0x001b,0x001c,0x001d,0x001e,0x001f,
  0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
  0x0028,0x0029,0x002a,0x002b,0x002c,0x002d,0x002e,0x002f,
  0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
  0x0038,0x0039,0x003a,0x003b,0x003c,0x003d,0x003e,0x003f,
  0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
  0x0048,0x0049,0x004a,0x004b,0x004c,0x004d,0x004e,0x004f,
  0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
  0x0058,0x0059,0x005a,0x005b,0x005c,0x005d,0x005e,0x005f,
  0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
  0x0068,0x0069,0x006a,0x006b,0x006c,0x006d,0x006e,0x006f,
  0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
  0x0078,0x0079,0x007a,0x007b,0x007c,0x007d,0x007e,0x007f,
  0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,
  0x0088,0x0089,0x008a,0x008b,0x008c,0x008d,0x008e,0x008f,
  0x0090,0x0091,0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,
  0x0098,0x0099,0x009a,0x009b,0x009c,0x009d,0x009e,0x009f,
  0x00a0,0x0401,0x0402,0x0403,0x0404,0x0405,0x0406,0x0407,
  0x0408,0x0409,0x040a,0x040b,0x040c,0x00ad,0x040e,0x040f,
  0x0410,0x0411,0x0412,0x0413,0x0414,0x0415,0x0416,0x0417,
  0x0418,0x0419,0x041a,0x041b,0x041c,0x041d,0x041e,0x041f,
  0x0420,0x0421,0x0422,0x0423,0x0424,0x0425,0x0426,0x0427,
  0x0428,0x0429,0x042a,0x042b,0x042c,0x042d,0x042e,0x042f,
  0x0430,0x0431,0x0432,0x0433,0x0434,0x0435,0x0436,0x0437,
  0x0438,0x0439,0x043a,0x043b,0x043c,0x043d,0x043e,0x043f,
  0x0440,0x0441,0x0442,0x0443,0x0444,0x0445,0x0446,0x0447,
  0x0448,0x0449,0x044a,0x044b,0x044c,0x044d,0x044e,0x044f,
  0x2116,0x0451,0x0452,0x0453,0x0454,0x0455,0x0456,0x0457,
  0x0458,0x0459,0x045a,0x045b,0x045c,0x00a7,0x045e,0x045f,
}, 28595},
{{
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
},{
  0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,
  0x0008,0x0009,0x000a,0x000b,0x000c,0x000d,0x000e,0x000f,
  0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,
  0x0018,0x0019,0x001a,0x001b,0x001c,0x001d,0x001e,0x001f,
  0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
  0x0028,0x0029,0x002a,0x002b,0x002c,0x002d,0x002e,0x002f,
  0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
  0x0038,0x0039,0x003a,0x003b,0x003c,0x003d,0x003e,0x003f,
  0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
  0x0048,0x0049,0x004a,0x004b,0x004c,0x004d,0x004e,0x004f,
  0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
  0x0058,0x0059,0x005a,0x005b,0x005c,0x005d,0x005e,0x005f,
  0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
  0x0068,0x0069,0x006a,0x006b,0x006c,0x006d,0x006e,0x006f,
  0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
  0x0078,0x0079,0x007a,0x007b,0x007c,0x007d,0x007e,0x007f,
  0x20ac,0x00a0,0x201a,0x0192,0x201e,0x2026,0x2020,0x2021,
  0x02c6,0x2030,0x0160,0x2039,0x0152,0x00a0,0x017d,0x00a0,
  0x00a0,0x2018,0x2019,0x201c,0x201d,0x2022,0x2013,0x2014,
  0x02dc,0x2122,0x0161,0x203a,0x0153,0x00a0,0x017e,0x0178,
  0x00a0,0x00a1,0x00a2,0x00a3,0x00a4,0x00a5,0x00a6,0x00a7,
  0x00a8,0x00a9,0x00aa,0x00ab,0x00ac,0x00ad,0x00ae,0x00af,
  0x00b0,0x00b1,0x00b2,0x00b3,0x00b4,0x00b5,0x00b6,0x00b7,
  0x00b8,0x00b9,0x00ba,0x00bb,0x00bc,0x00bd,0x00be,0x00bf,
  0x00c0,0x00c1,0x00c2,0x00c3,0x00c4,0x00c5,0x00c6,0x00c7,
  0x00c8,0x00c9,0x00ca,0x00cb,0x00cc,0x00cd,0x00ce,0x00cf,
  0x00d0,0x00d1,0x00d2,0x00d3,0x00d4,0x00d5,0x00d6,0x00d7,
  0x00d8,0x00d9,0x00da,0x00db,0x00dc,0x00dd,0x00de,0x00df,
  0x00e0,0x00e1,0x00e2,0x00e3,0x00e4,0x00e5,0x00e6,0x00e7,
  0x00e8,0x00e9,0x00ea,0x00eb,0x00ec,0x00ed,0x00ee,0x00ef,
  0x00f0,0x00f1,0x00f2,0x00f3,0x00f4,0x00f5,0x00f6,0x00f7,
  0x00f8,0x00f9,0x00fa,0x00fb,0x00fc,0x00fd,0x00fe,0x00ff,
},1252},
};
#define	NUM_BUILTIN_ENCODINGS (sizeof(builtin_encodings)/sizeof(builtin_encodings[0]))

#define NUMLET 33
static unsigned short russian_distrib[NUMLET*NUMLET]={
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 77, 259, 55, 298, 90, 111, 296, 12, 96, 384, 709,
284, 340, 1, 101, 381, 393, 422, 4, 1, 88, 6, 84, 49, 32, 0, 0, 0, 0, 86, 181,
0, 52, 0, 4, 0, 1, 180, 2, 0, 91, 0, 18, 86, 3, 45, 237, 0, 196, 13, 2, 73, 0,
3, 0, 0, 2, 18, 11, 271, 2, 0, 0, 36, 0, 416, 1, 7, 0, 20, 530, 0, 65, 283, 0,
12, 80, 9, 158, 548, 19, 63, 254, 15, 73, 0, 3, 2, 4, 93, 0, 0, 220, 16, 0, 0,
22, 0, 98, 0, 0, 0, 109, 30, 0, 0, 58, 0, 15, 194, 0, 59, 616, 0, 83, 0, 1, 49,
0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 412, 3, 76, 1, 5, 433, 40, 0, 214, 0, 29,
63, 2, 252, 358, 9, 186, 22, 10, 132, 0, 5, 7, 4, 3, 0, 0, 60, 42, 0, 2, 21, 0,
5, 102, 200, 260, 255, 140, 83, 113, 9, 281, 170, 544, 400, 776, 18, 84, 631,
427, 403, 10, 0, 43, 31, 105, 85, 55, 0, 0, 0, 0, 11, 15, 0, 126, 0, 0, 1, 96,
347, 1, 0, 109, 0, 10, 4, 0, 126, 8, 0, 0, 0, 0, 27, 0, 0, 0, 6, 0, 0, 0, 0,
3, 0, 0, 0, 0, 488, 17, 94, 45, 109, 49, 2, 2, 37, 0, 20, 24, 40, 154, 77, 0,
34, 0, 1, 33, 0, 0, 0, 0, 0, 0, 1, 27, 12, 0, 0, 16, 0, 4, 45, 233, 40, 120,
213, 33, 191, 33, 88, 217, 485, 226, 277, 10, 16, 85, 269, 433, 0, 0, 170, 76,
96, 64, 16, 0, 0, 0, 0, 18, 124, 0, 0, 0, 0, 8, 16, 0, 0, 0, 0, 0, 3, 0, 8, 21,
2, 0, 115, 32, 34, 0, 2, 0, 15, 17, 5, 0, 0, 0, 0, 0, 0, 0, 0, 540, 0, 6, 0,
0, 43, 1, 0, 328, 0, 55, 63, 0, 42, 800, 0, 160, 40, 39, 152, 0, 0, 3, 0, 1, 0,
0, 0, 0, 0, 0, 0, 0, 849, 3, 2, 34, 52, 501, 36, 1, 629, 0, 22, 25, 0, 37,
568, 5, 0, 166, 4, 125, 0, 0, 0, 6, 0, 0, 0, 69, 373, 0, 86, 164, 0, 248, 1,
0, 8, 0, 346, 0, 0, 234, 0, 12, 28, 1, 145, 330, 4, 6, 9, 0, 144, 0, 0, 1, 7,
0, 0, 0, 105, 7, 0, 0, 45, 0, 998, 0, 0, 1, 86, 878, 2, 30, 831, 0, 34, 0, 0,
258, 1016, 0, 6, 40, 139, 350, 1, 0, 20, 14, 0, 14, 0, 374, 77, 0, 8, 109, 0,
0, 395, 640, 446, 407, 157, 241, 123, 95, 310, 212, 633, 487, 707, 25, 108,
652, 587, 575, 6, 1, 71, 9, 157, 59, 35, 0, 0, 0, 1, 12, 61, 0, 122, 0, 0, 0,
0, 226, 0, 0, 63, 0, 6, 112, 0, 26, 909, 5, 636, 0, 4, 61, 0, 0, 0, 0, 0, 0, 0,
49, 1, 0, 0, 16, 0, 880, 4, 33, 12, 82, 654, 31, 5, 417, 0, 28, 11, 18, 111,
751, 9, 18, 29, 55, 260, 21, 21, 3, 8, 17, 2, 0, 159, 45, 0, 7, 98, 0, 239, 4,
131, 14, 21, 237, 7, 1, 111, 0, 278, 256, 86, 87, 279, 205, 17, 130, 960, 65,
3, 20, 2, 29, 4, 0, 1, 29, 317, 4, 10, 392, 0, 528, 4, 203, 0, 6, 510, 0, 0,
321, 0, 63, 36, 6, 90, 1026, 15, 248, 119, 3, 97, 0, 0, 1, 5, 1, 2, 0, 173,
553, 0, 1, 70, 0, 15, 51, 81, 80, 144, 13, 102, 32, 0, 9, 65, 227, 82, 26, 0,
65, 36, 117, 146, 0, 0, 39, 0, 77, 50, 24, 0, 0, 0, 1, 105, 6, 0, 50, 0, 0, 0,
0, 20, 0, 0, 29, 0, 0, 1, 0, 0, 27, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0,
0, 0, 0, 0, 28, 0, 20, 0, 0, 4, 0, 0, 16, 0, 0, 11, 11, 38, 212, 0, 15, 7, 1,
7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 50, 0, 7, 0, 0, 59, 0, 0, 24, 0, 0,
0, 0, 0, 27, 0, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0, 19, 0, 0, 0, 0, 0, 185, 0, 5,
0, 0, 226, 0, 0, 162, 0, 19, 0, 0, 94, 8, 0, 1, 0, 268, 44, 0, 0, 0, 0, 15, 0,
0, 0, 24, 0, 0, 0, 0, 66, 0, 2, 0, 0, 162, 0, 0, 155, 0, 53, 40, 0, 19, 19, 0,
1, 0, 2, 18, 0, 0, 0, 0, 0, 0, 0, 0, 58, 0, 0, 0, 0, 42, 0, 0, 0, 0, 137, 0,
0, 88, 0, 0, 0, 0, 10, 0, 0, 1, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0,
0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 9, 0, 0, 33, 75, 14, 10, 117, 6, 9, 0, 157, 11, 167, 130, 9,
0, 12, 25, 57, 76, 0, 0, 109, 0, 16, 34, 0, 0, 0, 0, 0, 0, 1, 0, 0, 7, 0, 1,
0, 27, 0, 22, 2, 0, 82, 0, 9, 129, 0, 0, 0, 104, 3, 0, 0, 0, 15, 13, 56, 0, 0,
0, 0, 0, 42, 27, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 0, 2, 6, 0, 0, 0, 0, 149,
0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 25, 0, 3, 46, 0, 26, 1, 0, 0, 0,
0, 2, 10, 0, 0, 1, 11, 43, 0, 0, 0, 0, 5, 0, 47, 0, 0, 0, 0, 2, 0, 0, 0, 0, 44,
14, 69, 10, 17, 23, 4, 19, 7, 80, 55, 57, 0, 0, 11, 63, 100, 0, 0, 21, 1, 6,
2, 39, 0, 0, 0, 0, 12, 9,
};

BOOL CALLBACK EnumCodePagesProc(LPTSTR name) {
  UINT		id;
  int		msnum;

  if ((id=_tcstoul(name,NULL,10))!=0 && (msnum=get_mscp_num(id))>=0)
    add_codepage(ms_codepages[msnum].name,id,ms_codepages[msnum].alias1,
	ms_codepages[msnum].alias2);
  return TRUE;
}

static int    _cdecl enc_cmp(const void *v1,const void *v2) {
  const struct CodePage	*cp1=(const struct CodePage *)v1;
  const struct CodePage	*cp2=(const struct CodePage *)v2;
  return cp1->codepage<cp2->codepage ? -1 : cp1->codepage>cp2->codepage ? 1 : 0;
}

static struct InitUnicode {
  InitUnicode();
} InitUnicode;

InitUnicode::InitUnicode() {
  // fetch system codepages
  EnumSystemCodePages((CODEPAGE_ENUMPROC)EnumCodePagesProc,CP_INSTALLED);
  qsort(codepages,curcp,sizeof(struct CodePage),enc_cmp);
  // and add our own
  DWORD	  mask;
  int	  i;
  for (i=mask=0;i<NUM_BUILTIN_ENCODINGS;++i) {
    int icp=Unicode::GetIntCodePage(builtin_encodings[i].cp);
    if (icp<0)
      mask|=1<<i;
    else
      codepages[icp].table=builtin_encodings[i].unimap;
  }
  bool need_utf8=Unicode::GetIntCodePage(CP_UTF8)<0;
  for (i=0;i<NUM_BUILTIN_ENCODINGS;++i)
    if (mask&(1<<i)) {
      int msnum=get_mscp_num(builtin_encodings[i].cp);
      if (msnum>=0) {
	int cp=add_codepage(ms_codepages[msnum].name,ms_codepages[msnum].cp,
			ms_codepages[msnum].alias1,ms_codepages[msnum].alias2);
	codepages[cp].length=TB_cp_length;
	codepages[cp].convert=TB_cp_convert;
	codepages[cp].table=builtin_encodings[i].unimap;
      }
    }
  if (need_utf8) {
    int	cp=add_codepage(_T("UTF-8"),CP_UTF8);
    codepages[cp].length=UTF_cp_length;
    codepages[cp].convert=UTF_cp_convert;
  }
  if (mask || need_utf8)
    qsort(codepages,curcp,sizeof(struct CodePage),enc_cmp);
  default_cp=Unicode::GetIntCodePage(1251); // XXX hardcoded
}

int   Unicode::WCLength(int codepage,const char *mbstr,int mblen) {
  if (codepage>=0 && codepage<curcp)
    return codepages[codepage].length(codepages+codepage,mbstr,mblen);
  return 0;
}

void  Unicode::ToWC(int codepage,const char *mbstr,int mblen,
		    wchar_t *wcstr,int wclen)
{
  if (codepage>=0 && codepage<curcp)
    codepages[codepage].convert(codepages+codepage,mbstr,mblen,wcstr,wclen);
}

int   Unicode::GetNumCodePages() {
  return curcp;
}

const TCHAR  *Unicode::GetCodePageName(int num) {
  if (num>=0 && num<curcp)
    return codepages[num].name;
  return NULL;
}

int   Unicode::GetIntCodePage(UINT mscp) {
  int	i=0,j=curcp;

  while (i<=j) {
    int	  m=(i+j)>>1;
    if (mscp<codepages[m].codepage)
      j=m-1;
    else if (mscp>codepages[m].codepage)
      i=m+1;
    else
      return m;
  }
  return -1;
}

static UINT   detect_encoding(const unsigned char *mbs,unsigned mblen) {
  unsigned	i,j;
  int		enc=0;
  int		sv,msv=0;
  int		hist[NUMLET*NUMLET];
  unsigned int	prev;
  unsigned char	*lettermap;

  if (mblen<3) /* detection needs at least a few letters :) */
    return CP_1252;
  if (mbs[0]=='\xef' && mbs[1]=='\xbb' && mbs[2]=='\xbf') // utf8 bom
    return CP_UTF8;
  if (mblen>1024) /* don't waste too much time */
    mblen=1024;
  for (i=0;i<NUM_BUILTIN_ENCODINGS;++i) {
    memset(hist,0,sizeof(int)*NUMLET*NUMLET);
    lettermap=builtin_encodings[i].distmap;
    for (j=prev=0;j<mblen;++j) {
      unsigned int next=lettermap[mbs[j]];
      if (next && prev)
        ++hist[prev*NUMLET+next];
      prev=next;
    }
    for (j=sv=0;j<NUMLET*NUMLET;++j)
      sv+=hist[j]*russian_distrib[j];
    if (sv>msv) {
      enc=i;
      msv=sv;
    }
  }
  if (msv<5) /* no cyrillic letters found */
    return CP_1252;
  return builtin_encodings[enc].cp;
}

int   Unicode::DetectCodePage(const char *mbs,int mblen) {
  UINT	  cp=detect_encoding((const unsigned char *)mbs,mblen);
  int	  lcp=GetIntCodePage(cp);

  return lcp<0 ? GetIntCodePage(CP_1252) : lcp; // 1252 should always be present
}

UINT  Unicode::GetMSCodePage(int cp) {
  if (cp>=0 && cp<curcp)
    return codepages[cp].codepage;
  return 1251; // XXX hardcoded
}

int   Unicode::FindCodePage(const TCHAR *name) {
  for (int i=0;i<curcp;++i)
    if (!CmpI(name,codepages[i].name) ||
	(codepages[i].alias1 && !CmpI(name,codepages[i].alias1)) ||
	(codepages[i].alias2 && !CmpI(name,codepages[i].alias2)))
      return i;
  return -1;
}

int   Unicode::DefaultCodePage() {
  return default_cp;
}

const wchar_t *Unicode::GetTable(int cp) {
  if (cp>=0 && cp<curcp && codepages[cp].table)
    return codepages[cp].table;
  return NULL;
}

int	Unicode::MBLength(const wchar_t *wcstr,int wclen) {
  return WideCharToMultiByte(CP_ACP,0,wcstr,wclen,NULL,0,NULL,NULL);
}

void	Unicode::ToMB(const wchar_t *wcstr,int wclen,char *mbstr,int mblen) {
  WideCharToMultiByte(CP_ACP,0,wcstr,wclen,mbstr,mblen,NULL,NULL);
}

Buffer<wchar_t>	  Unicode::ToWCbuf(int codepage,const char *mbstr,int mblen) {
  int	len=WCLength(codepage,mbstr,mblen);
  Buffer<wchar_t> ret(len);
  ToWC(codepage,mbstr,mblen,ret,len);
  return ret;
}

Buffer<char>	  Unicode::ToMBbuf(const wchar_t *wcstr,int wclen) {
  int	len=MBLength(wcstr,wclen);
  Buffer<char>	ret(len);
  ToMB(wcstr,wclen,ret,len);
  return ret;
}

CString		  Unicode::ToCS(const wchar_t *wcstr,int wclen) {
  return CString(wcstr,wclen);
}

Buffer<wchar_t>	  Unicode::ToWCbuf(const CString& str) {
  return Buffer<wchar_t>(str,str.GetLength());
}

// cstrings are implicitly nul terminated, so we can just up the size
Buffer<wchar_t>	  Unicode::ToWCbufZ(const CString& str) {
  return Buffer<wchar_t>(str,str.GetLength()+1);
}

const wchar_t	  *Unicode::GetCodePageNameW(int num) {
  return GetCodePageName(num);
}

Buffer<wchar_t> Unicode::Lower(const Buffer<wchar_t>& str) {
  int	  rlen=LCMapString(LOCALE_USER_DEFAULT,LCMAP_LOWERCASE,str,str.size(),NULL,0);
  Buffer<wchar_t>   ret(rlen);
  LCMapString(LOCALE_USER_DEFAULT,LCMAP_LOWERCASE,str,str.size(),ret,ret.size());
  return ret;
}

Buffer<char>   Unicode::SortKey(LCID lcid,const wchar_t *str,int len) {
  int	rlen=LCMapString(lcid,LCMAP_SORTKEY|NORM_IGNORECASE,str,len,NULL,0);
  Buffer<char>	ret(rlen);
  LCMapString(lcid,LCMAP_SORTKEY|NORM_IGNORECASE,str,len,(wchar_t*)(char*)ret,rlen);
  if (rlen>0) // don't include terminating 0
    ret.setsize(rlen-1);
  return ret;
}

Buffer<char>  Unicode::ToUtf8(const CString& cs) {
  // determine length
  int		utflen;
  const wchar_t	*cp=cs;
  int		i;
  int		max=cs.GetLength();

  for (i=utflen=0;i<max;++i) {
    DWORD c;
    if (cp[i]>=0xd800 && cp[i]<=0xdbff && i<max-1 && cp[i+1]>=0xdc00 && cp[i+1]<=0xdfff) {
      c=((DWORD)(cp[i]-0xd800)<<10) + (cp[i+1]-0xdc00) + 0x10000;
      ++i;
    } else
      c=cp[i];

    if (c<128)
      ++utflen;
    else if (c<2048)
      utflen+=2;
    else if (c<65536)
      utflen+=3;
    else
      utflen+=4;
  }

  Buffer<char>	ret(utflen);
  char		*dp=ret;

  for (i=0;i<max;++i) {
    DWORD c;
    if (cp[i]>=0xd800 && cp[i]<=0xdbff && i<max-1 && cp[i+1]>=0xdc00 && cp[i+1]<=0xdfff) {
      c=((DWORD)(cp[i]-0xd800)<<10) + (cp[i+1]-0xdc00) + 0x10000;
      ++i;
    } else
      c=cp[i];
    if (c<128)
      *dp++=(char)c;
    else if (c<2048) {
      *dp++=(char)(0xc0 | (c>>6));
      *dp++=(char)(0x80 | (c&0x3f));
    } else if (c<65536) {
      *dp++=(char)(0xe0 | (c>>12));
      *dp++=(char)(0x80 | ((c>>6)&0x3f));
      *dp++=(char)(0x80 | (c&0x3f));
    } else {
      *dp++=(char)(0xf0 | ((c>>18) & 0x07));
      *dp++=(char)(0x80 | ((c>>12) & 0x3f));
      *dp++=(char)(0x80 | ((c>>6) & 0x3f));
      *dp++=(char)(0x80 | (c&0x3f));
    }
  }

  return ret;
}