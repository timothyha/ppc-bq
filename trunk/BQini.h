#pragma once
#include <afx.h>
#include <atlstr.h>

class BQini{
protected:
	char* buf;
	int bufAllocated;

	CString Path;
	CString FullName;   char* charFullName;
	CString ShortName;  char* charShortName;
	CString Copyright;  char* charCopyright;
	CString Alphabet;   char* charAlphabet;
	char* HTMLFilter;
	char* ChapterSign;
	char* VerseSign;

	int izZipped;
	int Encoding;
	int ChapterQty;
	int isBible;
	int hasOldTestament;
	int hasNewTestament;
	int hasApocrypha;
	int hasChapterZero;
	int isGreek;
	int StrongNumbers;
	int ParagraphSkips;
	int DesiredFontCharset;
	int BookQty;
	char* charBookPathName [1000];
	char* charBookFullName [1000];
	char* charBookShortName[1000];
	int   BookChapterQty[1000];
	char* BibleShortNames[77]; // "true" book name
	
	int parseBuffer(int read);
public:
	BQini();
	~BQini();
	int loadFromPath(CString path);
	int loadFromZip(CString path);
	CString getPath(){return Path;};
	CString getFullName(){return FullName;};
	char* getCharShortName(){return charShortName;};
	CString getShortName(){return ShortName;};
	int getBookQty(){return BookQty;};
	char* getChapterSign(){return ChapterSign;};
	char* getVerseSign(){return VerseSign;};
	int getEncoding(){return Encoding;};
	int getIsBible(){return isBible;};
	int getHasOldTestament(){return hasOldTestament;};
	int getHasNewTestament(){return hasNewTestament;};
	int getHasApocrypha(){return hasApocrypha;};
	int getHasChapterZero(){return hasChapterZero;};
	int getIsGreek(){return isGreek;};
	int getIsZipped(){return izZipped;};
	int getHasStrongNumbers(){return StrongNumbers;}

	CString getBookPathName(int);
	CString getBookFullName(int);
	CString getBookShortName(int);
	CString getBookShortNames(int);
	int getBookChapterQty(int);
	int getBookAbsoluteIndex(int num);
	int getBookIndexByAbsolute(int idx);
};