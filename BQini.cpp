#include "stdafx.h"
#include "BQini.h"
#include "BQclasses.h"
#include "ZipFile.h"

BQini::BQini(void){
	bufAllocated = 0;
	isBible            = 0;
	hasOldTestament    = 0;
	hasNewTestament    = 0;
	hasApocrypha       = 0;
	isGreek            = 0;
	StrongNumbers      = 0;
	DesiredFontCharset = 0;
	ChapterSign        = 0;
	VerseSign          = 0;
	BookQty            = 0;
	BibleShortNames[0]  = "Gen";
	BibleShortNames[1]  = "Ex";
	BibleShortNames[2]  = "Lev";
	BibleShortNames[3]  = "Nu";
	BibleShortNames[4]  = "De";
	BibleShortNames[5]  = "Jos";
	BibleShortNames[6]  = "Jdg";
	BibleShortNames[7]  = "Ru";
	BibleShortNames[8]  = "1Sa";
	BibleShortNames[9]  = "2Sa";
	BibleShortNames[10] = "1Ki";
	BibleShortNames[11] = "2Ki";
	BibleShortNames[12] = "1Ch";
	BibleShortNames[13] = "2Ch";
	BibleShortNames[14] = "Ezr";
	BibleShortNames[15] = "Ne";
	BibleShortNames[16] = "Esth";
	BibleShortNames[17] = "Job";
	BibleShortNames[18] = "Ps";
	BibleShortNames[19] = "Pr";
	BibleShortNames[20] = "Ec";
	BibleShortNames[21] = "Song";
	BibleShortNames[22] = "Is";
	BibleShortNames[23] = "Je";
	BibleShortNames[24] = "La";
	BibleShortNames[25] = "Ez";
	BibleShortNames[26] = "Da";
	BibleShortNames[27] = "Ho";
	BibleShortNames[28] = "Joel";
	BibleShortNames[29] = "Am";
	BibleShortNames[30] = "Ob";
	BibleShortNames[31] = "Jon";
	BibleShortNames[32] = "Mi";
	BibleShortNames[33] = "Na";
	BibleShortNames[34] = "Hab";
	BibleShortNames[35] = "Zep";
	BibleShortNames[36] = "Hag";
	BibleShortNames[37] = "Ze";
	BibleShortNames[38] = "Mal";
	BibleShortNames[39] = "Mt";
	BibleShortNames[40] = "Mk";
	BibleShortNames[41] = "Lk";
	BibleShortNames[42] = "Jn";
	BibleShortNames[43] = "Ac";
	BibleShortNames[44] = "Jas";
	BibleShortNames[45] = "1Pe";
	BibleShortNames[46] = "2Pe";
	BibleShortNames[47] = "1Jn";
	BibleShortNames[48] = "2Jn";
	BibleShortNames[49] = "3Jn";
	BibleShortNames[50] = "Jud";
	BibleShortNames[51] = "Ro";
	BibleShortNames[52] = "1Co";
	BibleShortNames[53] = "2Co";
	BibleShortNames[54] = "Ga";
	BibleShortNames[55] = "Eph";
	BibleShortNames[56] = "Php";
	BibleShortNames[57] = "Col";
	BibleShortNames[58] = "1Th";
	BibleShortNames[59] = "2Th";
	BibleShortNames[60] = "1Ti";
	BibleShortNames[61] = "2Ti";
	BibleShortNames[62] = "Tit";
	BibleShortNames[63] = "Phm";
	BibleShortNames[64] = "He";
	BibleShortNames[65] = "Rev";
	BibleShortNames[66] = "1Mak";
	BibleShortNames[67] = "2Mak";
	BibleShortNames[68] = "3Mak";
	BibleShortNames[69] = "Bar";
	BibleShortNames[70] = "2Ez";
	BibleShortNames[71] = "3Ez";
	BibleShortNames[72] = "Judith";
	BibleShortNames[73] = "???";
	BibleShortNames[74] = "???";
	BibleShortNames[75] = "???";
	BibleShortNames[76] = "???";

}
BQini::~BQini(void){
	if(bufAllocated!=0) delete buf;
}
int BQini::loadFromPath(CString path){
	izZipped = 0;
	CString PATH = path + CString("\\bibleqt.ini");
	CFile iniFile;
	CFileException fileException;
	if(iniFile.Open(PATH, CFile::modeRead, &fileException)) {
		Path = path;
		ULONGLONG size = iniFile.GetLength();
		if(bufAllocated!=0) delete buf; buf = new char[(size+1+2)*sizeof(char)]; bufAllocated = 1;
		int read = iniFile.Read(buf, size); buf[read++] = '\r';buf[read++] = '\n';buf[read] = 0;
		parseBuffer(read);
		iniFile.Close();
		return 1;
	}
	return 0;
}
int BQini::loadFromZip(CString path){
	izZipped = 1;
	ZipFile zf(path);
	zf.SetDir(CString("\\"));
	zf.ReadZip();
	zf.Open(CString("bibleqt.ini"));
	long size = zf.size();
	if(size==0) return 0;
	Path = path;
	if(bufAllocated!=0) delete buf;
	buf = new char[(size+1+2)*sizeof(char)]; bufAllocated = 1;
	long read = zf.read(buf,size+1+2);
	buf[read++] = '\r';buf[read++] = '\n';buf[read] = 0;
	parseBuffer(read);
	return 1;
}
int BQini::parseBuffer(int read){
	char* tmpDesiredFontName;
	CString varName;
	CString varValue;
	isBible            = 0;
	hasOldTestament    = 0;
	hasNewTestament    = 0;
	hasApocrypha       = 0;
	hasChapterZero     = 0;
	isGreek            = 0;
	tmpDesiredFontName = 0;
	DesiredFontCharset = 0;
	ChapterSign        = 0;
	VerseSign          = 0;
	BookQty            = 0;
	int currentBookIndex = 0;
	char *pstart, *pend, *peq1, *peq;
	pstart = buf;
	while(pstart!=0 && pstart-buf<read && currentBookIndex<=BookQty){
		if((pend  = strstr(pstart, "\r\n"))==0) return 0;
		*(pend) = 0;
		if((pstart[0]>='A'&&pstart[0]<='Z')||(pstart[0]>='a'&&pstart[0]<='z')){
			for(peq = pstart;(*peq>='A'&&*peq<='Z')||(*peq>='a'&&*peq<='z');peq++);
			for(peq1 = peq;*peq!='=';peq++);for(peq++;*peq==' ';peq++);*peq1 = 0;
			if     (strcmp("BibleName", pstart)==0)          charFullName    = peq;
			else if(strcmp("BibleShortName", pstart)==0)     charShortName   = peq;
			else if(strcmp("Bible", pstart)==0)              isBible         = (*peq=='Y')?1:0;
			else if(strcmp("OldTestament", pstart)==0)       hasOldTestament = (*peq=='Y')?1:0;
			else if(strcmp("NewTestament", pstart)==0)       hasNewTestament = (*peq=='Y')?1:0;
			else if(strcmp("Apocrypha", pstart)==0)          hasApocrypha    = (*peq=='Y')?1:0;
			else if(strcmp("ChapterZero", pstart)==0)        hasChapterZero  = (*peq=='Y')?1:0;
			else if(strcmp("Greek", pstart)==0)              isGreek         = (*peq=='Y')?1:0;
			else if(strcmp("StrongNumbers", pstart)==0)      StrongNumbers   = (*peq=='Y')?1:0;
			else if(strcmp("Alphabet", pstart)==0)           charAlphabet    = peq;
			else if(strcmp("HTMLFilter", pstart)==0)         HTMLFilter      = peq;
			else if(strcmp("DesiredFontName", pstart)==0)    tmpDesiredFontName = peq;
			else if(strcmp("DesiredFontCharset", pstart)==0) DesiredFontCharset = atoi(peq);
			else if(strcmp("ChapterSign", pstart)==0)        ChapterSign = peq;
			else if(strcmp("VerseSign", pstart)==0)	         VerseSign = peq;
			else if(strcmp("BookQty", pstart)==0)            BookQty = atoi(peq);

			else if(BookQty>0){
				if     (strcmp("PathName", pstart)==0)   charBookPathName  [currentBookIndex++] = peq;
				else if(strcmp("FullName", pstart)==0)   charBookFullName  [currentBookIndex-1] = peq;
				else if(strcmp("ShortName", pstart)==0)  charBookShortName [currentBookIndex-1] = peq;
				else if(strcmp("ChapterQty", pstart)==0) BookChapterQty[currentBookIndex-1] = atoi(peq);
			}
		}
		pstart = pend + 2;
	}
	Encoding  = (DesiredFontCharset==204)? 1251 : 1251;
	FullName  = txt2uni(charFullName, Encoding);
	ShortName = txt2uni(charShortName, Encoding);
	return 0;
}

CString BQini::getBookPathName(int num){
	if(num>0 && num<=BookQty) return txt2uni(charBookPathName[num-1], Encoding);
	return CString("");
}
CString BQini::getBookFullName(int num){
	if(num>0 && num<=BookQty) return txt2uni(charBookFullName[num-1], Encoding);
	return CString("");
}
CString BQini::getBookShortName(int num){
	if(num>0 && num<=BookQty){
		CString ret;
		char* ptr = charBookShortName[num-1];
		char* ptr2 = strstr(ptr, " ");
		if(ptr2){
			int size = ptr2-ptr;
			char* tmpbuf = new char[(size+1)*sizeof(char)];
			strncpy(tmpbuf,ptr,size); tmpbuf[size]=0;
			ret = txt2uni(tmpbuf, Encoding);
			delete tmpbuf;
		}
		else ret = txt2uni(ptr, Encoding);
		return ret;
	}
	return CString("");
}
CString BQini::getBookShortNames(int num){
	if(num>0 && num<=BookQty) return txt2uni(charBookShortName[num-1], Encoding);
	return CString("");
}
int BQini::getBookChapterQty(int num){
	if(num>0 && num<=BookQty) return BookChapterQty[num-1];
	return 0;
}
int BQini::getBookAbsoluteIndex(int num){
	if(num<0)return -1;
	char * p;
	for(int i=0;i<77;i++){
		p = strstr(charBookShortName[num], BibleShortNames[i]);
		int l = strlen(BibleShortNames[i]);
		while(p!=0){
			if((p==charBookShortName[num]||*(p-1)==' ')&&(*(p+l)==' '||*(p+l)==0)){
				return i;
			}
			p = strstr(p+1, BibleShortNames[i]);
		}
	}
	return -1;
}
int BQini::getBookIndexByAbsolute(int idx){
	if(idx<0||idx>=77)return -1;
	char * p;
	int l = strlen(BibleShortNames[idx]);
	for(int i=0;i<BookQty;i++){
		p = strstr(charBookShortName[i], BibleShortNames[idx]);
		while(p!=0){
			if((p>0||*(p-1)==' ')&&(*(p+l)==' '||*(p+l)==0)){
				return i;
			}
			p = strstr(p+1, BibleShortNames[idx]);
		}
	}
	return -1;
}
