#include <afx.h>
#include <atlstr.h>

class BQposition{
public:
	char* moduleShortName;
	int   bookAbsIndex;
	int   chapter;
	int   scroll;
	CString path;
	int   isSet;
	BQposition(){
		isSet = false;
	};
	void set(const char*mod, int bk, int ch, int scr){
		if(isSet){
			delete moduleShortName;
		}
		moduleShortName = new char[(strlen(mod)+1)*sizeof(char)];
		strcpy(moduleShortName, mod);
		bookAbsIndex = bk;
		chapter = ch;
		scroll = scr;
		isSet = true;
	};
	~BQposition(){
		if(isSet)
			delete moduleShortName;
	};
};
class BQhistory{
public:
	int count;
	int allocated;
	int position;
	int limit;
	BQposition**list;
public:
	BQhistory(){
		count = 0;
		allocated = 0;
		position = -1;
		limit=100;
	};
	~BQhistory(){
		for(int i=0;i<count;i++)
			delete list[i];
		delete list;
	};
	int load(CString file);
	int save(int depth);
	int back(void);
	int forward(void);
	int insert(const char*mod, int bk, int ch, int scr);
};
