#include "stdafx.h"
#include "BQclasses.h"
#include "ZipFile.h"

int     BQBible::loadFromPath(CString path) {return ini->loadFromPath(path);}
CString BQBible::getPath()                  {return ini->getPath();}
CString BQBible::getFullName()              {return ini->getFullName();}
CString BQBible::getShortName()             {return ini->getShortName();}
int     BQBible::getBookQty()               {return ini->getBookQty();}
CString BQBible::getBookPathName(int num)   {return ini->getBookPathName(num);}
CString BQBible::getBookFullName(int num)   {return ini->getBookFullName(num);}
CString BQBible::getBookShortName(int num)  {return ini->getBookShortName(num);}
int     BQBible::getBookChapterQty(int num) {return ini->getBookChapterQty(num);}
CString convert2uni(char* src, int limit, UINT from, bool hideStrongs);
/*
int     BQBible::getBookNumByName(CString name)  {
	return ini->getBookChapterQty(num);
}
*/

BQBible::BQBible(){
	bufAllocated = 0;
	chapterCount = 0;
	foundCount   = 0;
}
BQBible::~BQBible(){
	if(bufAllocated!=0) delete buf;
	if(chapterCount){
		for(int i=0; i<chapterCount; i++){
			if(verseCount[i]!=-1) delete verses[i];
		}
		delete chapters;
		delete verseCount;
		delete verses;
	}
}
int BQBible::loadAbsBook(int idx){
	int num = ini->getBookIndexByAbsolute(idx)+1;
	if(num<0) {
		return loadBook(1);
	}
	return loadBook(num);
}
int BQBible::loadBook(int num){
	if(currentBook == num) return 1;
	absIndex = ini->getBookAbsoluteIndex(num-1);
	CString fileName = ini->getPath()+CString("\\")+ini->getBookPathName(num);
	CFile bookFile;
	CFileException fileException;
	if(chapterCount){
		for(int i=0; i<chapterCount; i++){
			delete verses[i];
		}
		delete chapters;
		delete verseCount;
		delete verses;
	}
	chapterCount = ini->getBookChapterQty(num);
	chapters   = new PCHAR[(chapterCount+1)*sizeof(PCHAR)];
	if(chapters==0) {chapterCount=0; return -1;}
	verseCount = new int[(chapterCount+1)*sizeof(int)];
	if(verseCount==0) {chapterCount=0; delete chapters; return -2;}
	verses = new PPCHAR[(chapterCount+1)*sizeof(PPCHAR)];
	if(verses==0) {chapterCount=0; delete chapters; delete verseCount; return -3;}
	for(int i=0; i<chapterCount; i++)verseCount[i] = -1;
	if (ini->getIsZipped()){
		ZipFile zf(ini->getPath());
		zf.SetDir(CString("\\"));
		zf.ReadZip();
		zf.Open(ini->getBookPathName(num));
		long size = zf.size();
		if(size==0) {
			chapterCount = 0;
			return -4;
		}
		currentBook = num;
		if(bufAllocated==0 || (bufAllocated>0 && bufAllocated<=size)){
			if(bufAllocated!=0) delete buf;
			buf = new char[(size+1)*sizeof(char)];
			if(buf==0) {chapterCount=0; delete chapters; delete verseCount; delete verses; return -4;}
			bufAllocated = size+1;
		}
		long read = zf.read(buf, size+1); buf[size] = 0;
		markBook();
	}
	else{
		if ( bookFile.Open( fileName, CFile::modeRead, &fileException ) ) {
			currentBook = num;
			long size = bookFile.GetLength();
			if(size==0) {
				chapterCount = 0;
				bookFile.Close();
				return -4;
			}
			if(bufAllocated==0 || (bufAllocated>0 && bufAllocated<=size)){
				if(bufAllocated!=0) delete buf;
				buf = new char[(size+1)*sizeof(char)];
				if(buf==0) {chapterCount=0; delete chapters; delete verseCount; delete verses; bookFile.Close(); return -4;}
				bufAllocated = size+1;
			}
			long read = bookFile.Read(buf, size); buf[size] = 0;
			bookFile.Close();
			markBook();
		}
		else {
//			AfxMessageBox( CString("Error opening file") );
		}
	}
	for(int i=0; i<chapterCount; i++){
		markChapter(i);
	}
	return 1;
}
int BQBible::markBook(){
	char *CHBR=buf, *LNBR=buf, *ptr=buf;
	int i=0;
	while((CHBR = strstr(CHBR, ini->getChapterSign()))!=0 && i<ini->getBookChapterQty(currentBook)){
		while((ptr=strstr(ptr, "\r\n"))!=0 && ptr<CHBR){
			LNBR = ++++ptr;
		}
		if(i>0)*(LNBR-2) = 0;
		chapters[i++] = LNBR;
		CHBR++;
	}
	chapters[ini->getBookChapterQty(currentBook)] = buf+bufAllocated-1;
	return 1;
}
int BQBible::markChapter(int num){
	char *VBR=chapters[num], *LNBR=chapters[num], *ptr=chapters[num];
	PCHAR* tmpbuf; int tmpbufSize = 1000; tmpbuf = new PCHAR[(tmpbufSize+1)*sizeof(PCHAR)];
	int vCount=0;
	while((VBR = strstr(VBR, ini->getVerseSign()))!=0){
		while((ptr=strstr(ptr, "\r\n"))!=0 && ptr<VBR){
			LNBR = ++++ptr;
		}
		if(vCount>0)*(LNBR-2) = 0;
		tmpbuf[vCount++] = LNBR;
		if(vCount>=tmpbufSize-1){
			PCHAR* tmpbuf2 = new PCHAR[(tmpbufSize+100)*sizeof(PCHAR)];
			if(tmpbuf2==0) return -1;
			for(int j=0; j<tmpbufSize; j++) *tmpbuf2 = *tmpbuf;
			delete tmpbuf;
			tmpbufSize+=100;
			tmpbuf = tmpbuf2;
		}
		VBR++;
	}
	tmpbuf[vCount] = chapters[num+1];
	verses[num] = tmpbuf;
	verseCount[num] = vCount;
	return 1;
}
CString BQBible::getChapter(int chNumber, bool showStrongs){
	CString ret = CString("");
	if(chNumber>0 && chNumber<=ini->getBookChapterQty(currentBook)){
		for(int i=1; i<=verseCount[chNumber-1]; i++){
			ret+= getVerse(chNumber, i, showStrongs);
			ret+= CString("\r\n");
		}
		return ret;
	}
	return CString("");
}
CString BQBible::getVerse(int chNumber, int vNumber, bool showStrongs){
	bool hideStrongs;
	hideStrongs = (!showStrongs && this->getIni()->getHasStrongNumbers());
	if(chNumber>0 && chNumber<=ini->getBookChapterQty(currentBook)){
		if(vNumber>0 && vNumber<=verseCount[chNumber-1]){
			int size = verses[chNumber-1][vNumber] - verses[chNumber-1][vNumber-1];
			CString ret = convert2uni(verses[chNumber-1][vNumber-1], size+size, ini->getEncoding(), hideStrongs);
			return ret;
		}
	}
	return CString(TEXT(""));
}
int BQBible::convert(char* src, char* dest, int limit){
	int   i=0;
	char* ptr;
	ptr = src;
	do{
		if(*ptr=='<') {
			ptr = strchr(ptr, '>');
			if(ptr==0){
				*(dest+i) = 0;
				return 1;
			}
			ptr++;
		}
		else if(*ptr=='&') {
			ptr = strchr(ptr, ';');
			if(ptr==0){
				*(dest+i) = 0;
				return 1;
			}
			if(strncmp(ptr+1, "nbsp;", 5)==0){
				*(dest + i++ ) = ' ';
			}
			else if(strncmp(ptr+1, "qt;", 3)==0){
				*(dest + i++ ) = '"';
			}
			else if(strncmp(ptr+1, "lt;", 3)==0){
				*(dest + i++ ) = '<';
			}
			else if(strncmp(ptr+1, "gt;", 3)==0){
				*(dest + i++ ) = '>';
			}
			ptr++;
		}
		else {
			*(dest + i++ ) = *(ptr++);
		}		
		if(i>=limit) {
			*(dest+limit-1)=0;
			return 0;
		}
	}while(*ptr != 0);
	*(dest+i) = 0;
	return 1;
}
int BQBible::find(CString what, const BQfindOptions* options){
	CString whatWord[20];
	int whatCount=0;
	what.Trim();
	int pos;
	if(options->CaseSensitive==0) what.MakeLower();
	if(options->ExactPhrase==0){
		while((pos = what.Find(CString(" ")))!=-1 && whatCount<20){
			whatWord[whatCount++] = what.Left(pos).Trim();
			what = what.Right(what.GetLength() - pos).Trim();
		}
	}
	whatWord[whatCount++] = what;
	if(whatCount){
		int tmpFoundSize = 0;
		if(foundCount) delete foundList;
		foundCount = 0;
		int booStart = options->BookStart; if(booStart<=0) booStart = 1;
		int booEnd = options->BookEnd; if(booEnd<=0||booEnd>=ini->getBookQty()) booEnd = ini->getBookQty();
		for(int boo=booStart; boo<=booEnd; boo++){
			int ret=loadBook(boo);
			if(ret>=0){
				for(int cha=1; cha<=ini->getBookChapterQty(boo); cha++){
					for(int ver=1; ver<=verseCount[cha-1]; ver++){
						int verStart = ver, verEnd = ver;
						CString HiStack = getVerse(cha, ver);
						if(options->VerseGroups!=0){
							CString str;
							int i=0;
							while((HiStack[HiStack.GetLength()-1]==',' ||
								HiStack[HiStack.GetLength()-1]   ==':' ||
								HiStack[HiStack.GetLength()-1]   =='-' ||
								(HiStack[HiStack.GetLength()-1]  =='"' && 
								HiStack[HiStack.GetLength()-2]  !='.' &&
								HiStack[HiStack.GetLength()-2]  !='!' &&
								HiStack[HiStack.GetLength()-2]  !='?' ))&&
								(str = getVerse(cha, verEnd+1))!=CString("") &&
								i++<20){
								HiStack+= CString(" ") + str; verEnd+=1;
							}
						}
						if(options->CaseSensitive==0) HiStack.MakeLower();
						int crit;
						crit = (options->AllWords==0)?0:1;
						for(int i=0; i<whatCount; i++){
							int findPos = HiStack.Find(whatWord[i], 0);
							if(findPos!=-1 && options->WholeWords!=0){
								int len0 = HiStack.GetLength();
								int len1 = whatWord[i].GetLength();
								if(  (findPos==0
									|| HiStack[findPos-1]==' '
									|| HiStack[findPos-1]=='.'
									|| HiStack[findPos-1]==','
									|| HiStack[findPos-1]==':'
									|| HiStack[findPos-1]==';'
									|| HiStack[findPos-1]=='!'
									|| HiStack[findPos-1]=='?'
									|| HiStack[findPos-1]=='"'
									|| HiStack[findPos-1]=='\''
									|| HiStack[findPos-1]=='-'
									|| HiStack[findPos-1]=='('
									|| HiStack[findPos-1]==')'
									)
								 &&(findPos+len1==len0
									  || HiStack[findPos+len1]==' '
									  || HiStack[findPos+len1]=='.'
									  || HiStack[findPos+len1]==','
									  || HiStack[findPos+len1]==':'
									  || HiStack[findPos+len1]==';'
									  || HiStack[findPos+len1]=='!'
									  || HiStack[findPos+len1]=='?'
									  || HiStack[findPos+len1]=='"'
									  || HiStack[findPos+len1]=='\''
									  || HiStack[findPos+len1]=='-'
									  || HiStack[findPos+len1]=='('
									  || HiStack[findPos+len1]==')'
									  ));
								else findPos = -1;
							}
							if(options->AllWords==0 && findPos!=-1) crit=1;
							if(options->AllWords!=0 && findPos==-1) crit=0;
							if(crit==((options->AllWords==0)?1:0)) break;
						}
						if(crit){
							if(foundCount>=tmpFoundSize){
								BQskatch* tmpList; tmpList = new BQskatch[(tmpFoundSize+100)*sizeof(BQskatch)];
								if(tmpList==0) return -1;
								if(tmpFoundSize>0){
									for(int i=0; i<tmpFoundSize; i++){
										tmpList[i].Book    = foundList[i].Book;
										tmpList[i].Chapter = foundList[i].Chapter;
										tmpList[i].Verse   = foundList[i].Verse;
										tmpList[i].toVerse = foundList[i].toVerse;
									}
									delete foundList;
								}
								foundList = tmpList;
								tmpFoundSize+=100;
							}
							foundCount++;
							foundList[foundCount-1].Book    = boo;
							foundList[foundCount-1].Chapter = cha;
							foundList[foundCount-1].Verse   = ver;
							foundList[foundCount-1].toVerse = (verEnd==ver)? 0 : verEnd;
							if(foundCount>5000) return -1;
						}
						ver = verEnd;
					}
				}
			}
		}
		return 1;
	}
	return 0;
}
CString BQBible::getFoundTitle(int num){
	if(num<foundCount){
		BQskatch* item = foundList+num;
		return getVerseTitle(item->Book, item->Chapter, item->Verse, item->toVerse);
	}
	return CString("");
}
CString BQBible::getVerseTitle(int book, int chapter, int verse, int toVerse){
	CString Book, Chapter, Verse;
	Book = getBookShortName(book);
	Chapter.Format(_T("%d"), chapter);
	if(toVerse) Verse.Format(_T("%d-%d"),verse,toVerse);
	else Verse.Format(_T("%d"),verse);
	return Book+CString(" ")+Chapter+CString(":")+Verse;
}
CString BQBible::getFoundContent(int num){
	if(num<foundCount){
		BQskatch* item = foundList+num;
		loadBook(item->Book);
		if(item->toVerse){
			CString ret; ret = CString("");
			for(int i=item->Verse; i<=item->toVerse; i++){
				ret+= getVerse(item->Chapter, i) + ((i<item->toVerse) ? CString("\r\n") : CString(""));
			}
			return ret;
		}
		else return getVerse(item->Chapter, item->Verse);
	}
	return CString("");
}


CString txt2uni(char* buf, UINT from){
	int nRet = MultiByteToWideChar(from, MB_PRECOMPOSED, buf, -1, NULL, 0);
	wchar_t* convBuffer = new wchar_t[(nRet+2)*sizeof(wchar_t)];
	MultiByteToWideChar(from, MB_PRECOMPOSED, buf, -1, convBuffer, nRet+2);
	CString ret = CString(convBuffer);
	delete convBuffer;
	return ret;
}

CString uni2txt(CString s){
	int len = s.GetLength();
	char* convBuffer = new char[(len+1)*sizeof(char)];
	
	WideCharToMultiByte(
	  CP_ACP,            // code page
	  0,            // performance and mapping flags
	  s.GetBuffer(),    // wide-character string
	  len,          // number of chars in string.
	  (LPSTR)convBuffer,     // buffer for new string
	  len+len,          // size of buffer
	  (LPCSTR)"_",     // default for unmappable chars
	  (LPBOOL)true  // set when default char used
	);
	CString ret(convBuffer);
	delete convBuffer;
	return ret;
}


wchar_t codes_greek[]  = {
	  0x00,  0x01,  0x02,  0x03,  0x04,  0x05,  0x06,  0x07,  0x08,  0x09,  0x0A,  0x0B,  0x0C,  0x0D,  0x0E,  0x0F,
	  0x10,  0x11,  0x12,  0x13,  0x14,  0x15,  0x16,  0x17,  0x18,  0x19,  0x1A,  0x1B,  0x1C,  0x1D,  0x1E,  0x1F,
	  0x20,  0x21,0x2019,0x2019,0x2019,  0x25,  0x26,0x2019,  0x28,  0x29,  0x2A, 0x385,  0x2C,  0x2D,  0x2E, 0x301,
	  0x30,  0x31,  0x32,  0x33,  0x34,  0x35,  0x36,  0x37,  0x38,  0x39,0x201B,0x201B,  0x3C,     0,  0x3E,  0x3F,
	  0x40, 0x391, 0x392, 0x3A7, 0x394, 0x395, 0x3A6, 0x393, 0x397, 0x399,  0x4A, 0x39A, 0x39B, 0x39C, 0x39D, 0x39F,
	 0x3A0, 0x3A8, 0x3A1, 0x3A3, 0x3A4, 0x3A5, 0x2D9, 0x3A9, 0x39E, 0x398, 0x396,  0x5B,  0x5C,  0x5D,  0x5E, 0x2D8,
	  0x60, 0x3B1, 0x3B2, 0x3C7, 0x3B4, 0x3B5, 0x3C6, 0x3B3, 0x3B7, 0x3B9, 0x300, 0x3BA, 0x3BB, 0x3BC, 0x3BD, 0x3BF,
	 0x3C0, 0x3C8, 0x3C1, 0x3C3, 0x3C4, 0x3C5, 0x3C2, 0x3C9, 0x3BE, 0x3B8, 0x3B6,0x201B,  0x7C,0x2019, 0x2DC,     0,
	     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0
};

wchar_t codes_hebrew[] = {
	  0x00,  0x01,  0x02,  0x03,  0x04,  0x05,  0x06,  0x07,  0x08,  0x09,  0x0A,  0x0B,  0x0C,  0x0D,  0x0E,  0x0F,
	  0x10,  0x11,  0x12,  0x13,  0x14,  0x15,  0x16,  0x17,  0x18,  0x19,  0x1A,  0x1B,  0x1C,  0x1D,  0x1E,  0x1F,
	  0x20,  0x21, 0x5B7, 0x5B2, 0x5B3, 0x5B1,  0x26, 0x5b2,  0x28,  0x29,  0x2A, 0x5BC,  0x2C,  0x2D,  0x2E, 0x5b7,
	  0x30,  0x31,  0x32,  0x33,  0x34,  0x35,  0x36,  0x37,  0x38,  0x39, 0x5C3, 0x5B4,  0x3C, 0x5BC,  0x3E,  0x3F,
	  0x40,  0x41,  0x42,  0x43,     0,     0,     0,     0,     0,  0x49,     0, 0x5DA,     0, 0x5DD, 0x5DF,     0,
	 0x5E3,     0,     0,     0,     0, 0x5E5, 0x5C1,     0,     0,     0,     0, 0x5B5, 0x5B7, 0x5B5,  0x5E, 0x5B0,
	  0x60, 0x5d0, 0x5d1, 0x5BB, 0x5D3, 0x5E2, 0x5B9, 0x5D2, 0x5D4, 0x5B4, 0x5D8, 0x5DB, 0x5DC, 0x5DE, 0x5E0, 0x5E1,
	 0x5E4, 0x5E7, 0x5E8, 0x5E9, 0x5EA, 0x5E6, 0x5E9, 0x5D5, 0x5D7, 0x5D9, 0x5D6, 0x5B5,  0x7C, 0x5B5,  0x7E,  0x7F,
	  0x80,  0x81,  0x82,  0x83,  0x84,  0x85,  0x86,  0x87,  0x88,  0x89,  0x8A,  0x8B,  0x8C,  0x8D,  0x8E,  0x8F,
	  0x90,  0x91,  0x92,  0x93,  0x94,  0x95,  0x96,  0x97,  0x98,  0x99,  0x9A,  0x9B,  0x9C,  0x9D,  0x9E,  0x9F,
	  0xA0,  0xA1,  0xA2,  0xA3,  0xA4,  0xA5,  0xA6,  0xA7,  0xA8,  0xA9,  0xAA,  0xAB,  0xAC,  0xAD,  0xAE,  0xAF,
	  0xB0,  0xB1,  0xB2,  0xB3,  0xB4,  0xB5,  0xB6,  0xB7,  0xB8,  0xB9,  0xBA,  0xBB,  0xBC,  0xBD,  0xBE,  0xBF,
	  0xC0,  0xC1,  0xC2,  0xC3,  0xC4,  0xC5,  0xC6,  0xC7,  0xC8,  0xC9,  0xCA,  0xCB,  0xCC,  0xCD,  0xCE,  0xCF,
	  0xD0,  0xD1,  0xD2,  0xD3,  0xD4,  0xD5,  0xD6,  0xD7,  0xD8,  0xD9,  0xDA,  0xDB,  0xDC,  0xDD,  0xDE,  0xDF,
	0xFB4B,0xFB31,0xFB32,0xFB33,0xFB34,0xFB35,0xFB36,     0,0xFB38,0xFB39,0xFB3A,0xFB3B,0xFB3C,0xE802,0xFB3E,0xE803,
	0xFB40,0xFB41,     0,0xFB43,0xFB44,     0,0xFB46,0xFB47,0xFB48,0xFB49,     0,     0,     0,     0,     0,     0
};
/*
wchar_t codes_hebrew_cons[] = {// consonants only
	  0x00,  0x01,  0x02,  0x03,  0x04,  0x05,  0x06,  0x07,  0x08,  0x09,  0x0A,  0x0B,  0x0C,  0x0D,  0x0E,  0x0F,
	  0x10,  0x11,  0x12,  0x13,  0x14,  0x15,  0x16,  0x17,  0x18,  0x19,  0x1A,  0x1B,  0x1C,  0x1D,  0x1E,  0x1F,
	  0x20,  0x21,	   0,     0,     0,     0,  0x26, 0x5b2,  0x28,  0x29,  0x2A, 0x5BC,  0x2C,  0x2D,  0x2E, 0x5b7,
	  0x30,  0x31,  0x32,  0x33,  0x34,  0x35,  0x36,  0x37,  0x38,  0x39, 0x5C3, 0x5B4,  0x3C, 0x5BC,  0x3E,  0x3F,
	  0x40,  0x41,  0x42,  0x43,     0,     0,     0,     0,     0,  0x49,     0, 0x5DA,     0, 0x5DD, 0x5DF,     0,
	 0x5E3,     0,     0,     0,     0, 0x5E5, 0x5C1,     0,     0,     0,     0, 0x5B5, 0x5B7, 0x5B5,  0x5E, 0x5B0,
	  0x60, 0x5d0, 0x5d1, 0x5BB, 0x5D3, 0x5E2, 0x5B9, 0x5D2, 0x5D4, 0x5B4, 0x5D8, 0x5DB, 0x5DC, 0x5DE, 0x5E0, 0x5E1,
	 0x5E4, 0x5E7, 0x5E8, 0x5E9, 0x5EA, 0x5E6, 0x5E9, 0x5D5, 0x5D7, 0x5D9, 0x5D6, 0x5B5,  0x7C, 0x5B5,  0x7E,  0x7F,
	  0x80,  0x81,  0x82,  0x83,  0x84,  0x85,  0x86,  0x87,  0x88,  0x89,  0x8A,  0x8B,  0x8C,  0x8D,  0x8E,  0x8F,
	  0x90,  0x91,  0x92,  0x93,  0x94,  0x95,  0x96,  0x97,  0x98,  0x99,  0x9A,  0x9B,  0x9C,  0x9D,  0x9E,  0x9F,
	  0xA0,  0xA1,  0xA2,  0xA3,  0xA4,  0xA5,  0xA6,  0xA7,  0xA8,  0xA9,  0xAA,  0xAB,  0xAC,  0xAD,  0xAE,  0xAF,
	  0xB0,  0xB1,  0xB2,  0xB3,  0xB4,  0xB5,  0xB6,  0xB7,  0xB8,  0xB9,  0xBA,  0xBB,  0xBC,  0xBD,  0xBE,  0xBF,
	  0xC0,  0xC1,  0xC2,  0xC3,  0xC4,  0xC5,  0xC6,  0xC7,  0xC8,  0xC9,  0xCA,  0xCB,  0xCC,  0xCD,  0xCE,  0xCF,
	  0xD0,  0xD1,  0xD2,  0xD3,  0xD4,  0xD5,  0xD6,  0xD7,  0xD8,  0xD9,  0xDA,  0xDB,  0xDC,  0xDD,  0xDE,  0xDF,
	0xFB4B,0xFB31,0xFB32,0xFB33,0xFB34,0xFB35,0xFB36,     0,0xFB38,0xFB39,0xFB3A,0xFB3B,0xFB3C,0xE802,0xFB3E,0xE803,
	0xFB40,0xFB41,     0,0xFB43,0xFB44,     0,0xFB46,0xFB47,0xFB48,0xFB49,     0,     0,     0,     0,     0,     0
};
*/
CString convert2uni(char* src, int limit, UINT from, bool hideStrongs){
	int   i=0;
	char* ptr;
	char* ptr0;
	wchar_t* dest;
	CString ret;
	bool spaceOccured = false;
	int nRet = MultiByteToWideChar(from, MB_PRECOMPOSED, (LPCSTR)src, -1, NULL, 0);
	dest = new wchar_t[(nRet+nRet+1)*sizeof(wchar_t)];
	ptr = src;
	do{
		if(*ptr==0x20) spaceOccured = true;

		if(*ptr=='<') {
			if(strncmp(ptr, "<font face=Heb>", 15)==0){
				ptr+= 15;
				ptr0 = strstr(ptr, "</font>");
				dest[i++] = 0x0020;
				for(;ptr<ptr0;ptr++){
					if(codes_hebrew[*ptr])
						dest[i++] = codes_hebrew[*ptr];
				}
				dest[i++] = 0x0020;
				ptr+= 7;
			}
			else if(strncmp(ptr, "<font face=Grk>", 15)==0){
				ptr+= 15;
				ptr0 = strstr(ptr, "</font>");
				//dest[i++] = 0x003C;
				for(;ptr<ptr0;ptr++){
					if(codes_greek[*ptr])
						dest[i++] = codes_greek[*ptr];
				}
				//dest[i++] = 0x003E;
				ptr+= 7;
			}
			else{
				ptr = strchr(ptr, '>');
				if(ptr==0){
					*(dest+i) = 0;
					break;
				}
				ptr++;
			}
		}
		else if(*ptr=='&') {
			ptr = strchr(ptr, ';');
			if(ptr==0){
				*(dest + i++) = 0;
				break;
			}
			if(strncmp(ptr+1, "nbsp;", 5)==0){
				*(dest + i++ ) = ' ';
			}
			else if(strncmp(ptr+1, "qt;", 3)==0){
				*(dest + i++ ) = '"';
			}
			else if(strncmp(ptr+1, "lt;", 3)==0){
				*(dest + i++ ) = '<';
			}
			else if(strncmp(ptr+1, "gt;", 3)==0){
				*(dest + i++ ) = '>';
			}
			ptr++;
		}
		else if(hideStrongs && *ptr==0x20 && *(ptr+1)>=0x30 && *(ptr+1)<=0x39 ){
			// if strongs are invisible - don't show them
			ptr++;
		}
		else if(spaceOccured && hideStrongs && *ptr>=0x30 && *ptr<=0x39 ){
			// if strongs are invisible - don't show them
			ptr++;
			if(*ptr==0x20 && (*(ptr+1)=='.'||*(ptr+1)==','||*(ptr+1)==';'||*(ptr+1)==':'||*(ptr+1)=='!'||*(ptr+1)=='?'||*(ptr+1)=='-')){
				ptr++;
			}
		}
		else {
			//*(dest + i++ ) = *(ptr++);
			nRet = MultiByteToWideChar(from, MB_PRECOMPOSED, (LPCSTR)ptr, 1, NULL, 0);
			MultiByteToWideChar(from, MB_PRECOMPOSED, (LPCSTR)ptr, 1, (LPWSTR)(dest+i), nRet);
			i+= nRet;
			ptr++;
			/*
			switch(*(dest+i-nRet)){
				case 0x0410:
				case 0x0415:
				case 0x0418:
				case 0x041E:
				case 0x0423:
				case 0x042B:
				case 0x042D:
				case 0x042E:
				case 0x042F:
				case 0x0430:
				case 0x0435:
				case 0x0438:
				case 0x043E:
				case 0x0443:
				case 0x044B:
				case 0x044D:
				case 0x044E:
				case 0x044F:
					*(dest + i++ ) = 0x00AD;
					//*(dest + i++ ) = '\r';
					//*(dest + i++ ) = '\n';
			}
			*/
		}		
		if(i>=limit) {
			*(dest+limit-1)=0;
			break;
		}
	}while(*ptr != 0);
	*(dest+i) = 0;
	ret = CString(dest);
	delete dest;
	return ret;
}
