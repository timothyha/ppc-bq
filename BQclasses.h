#pragma once
#include "BQini.h"
#include <afx.h>
#include <atlstr.h>
typedef char* PCHAR;
typedef PCHAR* PPCHAR;
//class BQini;
struct BQskatch{
	int Book;
	int Chapter;
	int Verse;
	int toVerse;
};
struct BQfindOptions{
	int AllWords;      //
	int CaseSensitive; //
	int WholeWords;    //
	int ExactPhrase;   //
	int VerseGroups;   
	int BookStart;     //
	int BookEnd;       //
	BQfindOptions(){
		AllWords=1; 
		CaseSensitive=0; 
		WholeWords=0; 
		ExactPhrase=0; 
		VerseGroups=0; 
		BookStart=0; 
		BookEnd=0;
	};
};
class BQBible{
protected:
	char*buf;
	int bufAllocated;
	int currentBook;
	int chapterCount;
	int* verseCount;
	PCHAR* chapters;
	PCHAR** verses;
	BQini* ini;
	BQskatch* foundList;
	int foundCount;
	int markBook(void);
	int markChapter(int num);
	int absIndex;
public:
//	CEdit* EditScripture;
	BQBible(void);
	~BQBible(void);
	int loadBook(int num);
	int getChapterCount();
	CString getChapter(int chNumber, bool showStrongs);
	CString getChapter(int chNumber) {return getChapter(chNumber, false);};
	CString getVerse(int chNumber, int vNumber, bool showStrongs);
	CString getVerse(int chNumber, int vNumber){return getVerse(chNumber, vNumber, false);};
	int convert(char* src, char* dest, int limit);
	CString getName();
	CString getShortName();
	int find(CString what, const BQfindOptions* options);
	int getFoundQty(){return foundCount;};
	CString getFoundTitle(int num);
	CString getVerseTitle(int book, int chapter, int verse, int toVerse);
	CString getFoundContent(int num);

	// BQini:
	int loadFromPath(CString path);
	CString getPath();
	CString getFullName();
	CString getShortName(int);
	int getBookQty();
	CString getBookPathName(int);
	CString getBookFullName(int);
	CString getBookShortName(int);
	int getBookChapterQty(int);
	void setIni(BQini* ini1){if(ini!=ini1) currentBook=-1;ini = ini1;};
	BQini* getIni(){return ini;};
	int loadAbsBook(int idx);
	int getAbsIndex(){return absIndex;};
};

CString txt2uni(char* buf, UINT from);
CString uni2txt(CString s);