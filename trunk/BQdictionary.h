#include <afx.h>
#include <atlstr.h>
typedef wchar_t* pchar;
class BQdictionary{
	int count;
	CString Path;
	pchar* Words;
	int* positions;
	UINT encoding;
public:
	BQdictionary();
	~BQdictionary();
	int load(CString path, UINT _encoding);
	CString get(int Number);
	int getPosition(CString*Text);
};