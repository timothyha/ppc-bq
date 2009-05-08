#include "BQini.h"
#include "BQdictionary.h"
#include <afx.h>
#include <atlstr.h>
class BQBooklist{
protected:
	BQini* Bibles[100];
	BQini* Commentaries[100];
	BQini* Others[100];
	BQdictionary strongs_heb;
	BQdictionary strongs_grk;
	int BiblesQty, CommentariesQty, OthersQty;
	bool StrongsDictionaryExists;
	bool StrongsDictionaryHebExists;
	bool StrongsDictionaryGrkExists;
public:
	BQBooklist(void);
	~BQBooklist(void);
	int load(CString Path);
	int load(CString Path, int isComment);
	int getItemQty();
	CString getItemName(int num);
	CString getItemPath(int num);
	int getItemEnabled(int num);
	BQini* getIni(int num);
	char* getItemCharShortName(int num);
	bool getStrongsDictionaryExists(){return StrongsDictionaryExists;}
	bool getStrongsDictionaryHebExists(){return StrongsDictionaryHebExists;}
	bool getStrongsDictionaryGrkExists(){return StrongsDictionaryGrkExists;}
	BQdictionary* getStrongsHeb(){if(StrongsDictionaryExists) return &strongs_heb; else return NULL;}
	BQdictionary* getStrongsGrk(){if(StrongsDictionaryExists) return &strongs_grk; else return NULL;}
};