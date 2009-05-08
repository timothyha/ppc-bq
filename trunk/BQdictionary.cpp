#include "BQdictionary.h"

CString convert2uni(char* src, int limit, UINT from, bool hideStrongs);

BQdictionary::BQdictionary()
{
	count = -1;
}
BQdictionary::~BQdictionary()
{
	if(count>0){
		for(int i=0; i<count; i++)
			if(Words[i]) delete Words[i];
		delete Words;
		delete positions;
	}
}
int BQdictionary::load(CString path, UINT _encoding)
{
	encoding = _encoding;
	CString PATH = path + CString(".idx");
	CFile iniFile;
	CFileException fileException;
	int i, i1, i0, k;
	char* buf;
	if(iniFile.Open(PATH, CFile::modeRead, &fileException)) {
		Path = path;
		ULONGLONG size = iniFile.GetLength();
		buf = new char[(size+1)*sizeof(char)];
		int read = iniFile.Read(buf, size); buf[read++] = '\r';buf[read++] = '\n';buf[read] = 0;
		iniFile.Close();
		i=0;
		count = 0;
		while(i<size){
			i0 = i;
			while(i<size&&buf[i++]!='\n');
			while(i<size&&buf[i++]!='\n');
			if(i - i0 > 4)
				count++;
		}
		Words = new pchar[(count+1)*sizeof(pchar)];
		positions = new int[(count+1)*sizeof(int)];
		i=0; k=0;
		
		while(i<size){
			i0 = i;
			while(buf[i++]!='\n');
			if(i - i0 > 2){
				i1 = i;
				while(buf[i1++]!='\n');
				int nRet = MultiByteToWideChar(1251, MB_PRECOMPOSED, (LPCSTR)buf+i, i1-i-2, NULL, 0);
				Words[k] = new wchar_t[(nRet+1)*sizeof(wchar_t)];
				MultiByteToWideChar(1251, MB_PRECOMPOSED, (LPCSTR)buf+i, i1-i-2, (LPWSTR)(Words[k]), nRet);
				Words[k][nRet] = 0;
				i = i1;
				sscanf(buf+i,"%d", positions+(k++));
			}
		}
		
		if(iniFile.Open(path + CString(".htm"), CFile::modeRead, &fileException)) {
			positions[count] = iniFile.GetLength();
			iniFile.Close();
		}
		delete buf;
		return 1;
	}
	return 0;
}
CString BQdictionary::get(int Number)
{
	CFile iniFile;
	CFileException fileException;
	char* buf;
	if(Number<0 || Number>count)
		return CString(TEXT("Hello! Strongs do not work yet!"));
	if(iniFile.Open(Path + CString(".htm"), CFile::modeRead, &fileException)) {
		buf = new char[positions[Number+1] - positions[Number]+1];
		iniFile.Seek(positions[Number], CFile::begin);
		iniFile.Read(buf, positions[Number+1] - positions[Number]);
		buf[positions[Number+1] - positions[Number]] = 0;
		CString ret = convert2uni(buf, positions[Number+1] - positions[Number], encoding, false);
		iniFile.Close();
		delete buf;
		return ret;
	}
	return CString(TEXT(""));
}
int BQdictionary::getPosition(CString*Text)
{
	for(int i=0; i<count; i++){
		if(Text[0].Compare(CString(Words[i]))==0)
			return i;
	}
	return -1;
}
