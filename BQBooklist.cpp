#include "stdafx.h"
#include "BQBooklist.h"

BQBooklist::BQBooklist(void){
	BiblesQty = CommentariesQty = OthersQty = 0;
}
BQBooklist::~BQBooklist(void){
	if(BiblesQty>0){
		for(int i=0; i<BiblesQty; i++) delete Bibles[i];
	}
	if(CommentariesQty>0){
		for(int i=0; i<CommentariesQty; i++) delete Commentaries[i];
	}
	if(OthersQty>0){
		for(int i=0; i<OthersQty; i++) delete Others[i];
	}
}
// Loads the list of books from the directory specified
int BQBooklist::load(CString Path){
	load(Path,0);
	load(Path,1);
	return 1;
}

int BQBooklist::load(CString Path, int isComment){
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	CFileException fileException;
	if(isComment) Path+=CString("Commentaries\\");
	hFind = FindFirstFile(Path+CString("*.*"), &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			CString ext = CString(FindFileData.cFileName).Right(4);
			if(!StrongsDictionaryExists && CString(FindFileData.cFileName)==CString("Strongs")){
				StrongsDictionaryExists = true;
				if(strongs_heb.load(Path+CString("Strongs\\hebrew"), 1251))
					StrongsDictionaryHebExists = true;
				if(strongs_grk.load(Path+CString("Strongs\\greek"), 1251))
					StrongsDictionaryGrkExists = true;
			}
			else if(ext==CString(".zip")||ext==CString(".ZIP")){
				CString PATH = CString(FindFileData.cFileName);
				BQini* tempIni = new BQini();
				if(tempIni->loadFromZip(Path+CString(FindFileData.cFileName))){
					if(isComment) Commentaries[CommentariesQty++] = tempIni;
					else if(tempIni->getIsBible()) Bibles[BiblesQty++] = tempIni;
					else Others[OthersQty++] = tempIni;
				}
				else delete tempIni;
			}
			else if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
				CString PATH = CString(FindFileData.cFileName)+CString("\\bibleqt.ini");
				WIN32_FIND_DATA FindFileData1;
				HANDLE hFind1;
				hFind1 = FindFirstFile(Path+PATH, &FindFileData1);
				if (hFind1 != INVALID_HANDLE_VALUE) {
					BQini* tempIni = new BQini();
					if(tempIni->loadFromPath(Path+CString(FindFileData.cFileName))){
						if(isComment) Commentaries[CommentariesQty++] = tempIni;
						else if(tempIni->getIsBible()) Bibles[BiblesQty++] = tempIni;
						else Others[OthersQty++] = tempIni;
					}
					else delete tempIni;
				}
				FindClose(hFind1);
			}
		}while (FindNextFile(hFind, &FindFileData));
	}
	FindClose(hFind);
	return 1;
}
// Gets the quantity of items (including the divisions)
int BQBooklist::getItemQty(){
	return BiblesQty + CommentariesQty + OthersQty +
		(BiblesQty?1:0) + (CommentariesQty?1:0) + (OthersQty?1:0);
}
CString BQBooklist::getItemName(int num){
	if(BiblesQty && num==0) return CString("======BIBLES======");
	if(num<BiblesQty+(BiblesQty?1:0)) return Bibles[num-1]->getFullName();
	num-= BiblesQty+(BiblesQty?1:0);
	if(CommentariesQty && num==0) return CString("===COMMENTARIES===");
	if(num<CommentariesQty+(CommentariesQty?1:0)) return Commentaries[num-1]->getFullName();
	num-= CommentariesQty+(CommentariesQty?1:0);
	if(OthersQty && num==0) return CString("====OTHER BOOKS====");
	if(num<=OthersQty) return Others[num-1]->getFullName();
	return CString("");
}
char* BQBooklist::getItemCharShortName(int num){
	if(BiblesQty && num==0) return "======BIBLES======";
	if(num<BiblesQty+(BiblesQty?1:0)) return Bibles[num-1]->getCharShortName();
	num-= BiblesQty+(BiblesQty?1:0);
	if(CommentariesQty && num==0) return "===COMMENTARIES===";
	if(num<CommentariesQty+(CommentariesQty?1:0)) return Commentaries[num-1]->getCharShortName();
	num-= CommentariesQty+(CommentariesQty?1:0);
	if(OthersQty && num==0) return "====OTHER BOOKS====";
	if(num<=OthersQty) return Others[num-1]->getCharShortName();
	return "";
}
CString BQBooklist::getItemPath(int num){
	if(BiblesQty && num==0) return CString("");
	if(num<BiblesQty+(BiblesQty?1:0)) return Bibles[num-1]->getPath();
	num-= BiblesQty+(BiblesQty?1:0);
	if(CommentariesQty && num==0) return CString("");
	if(num<CommentariesQty+(CommentariesQty?1:0)) return Commentaries[num-1]->getPath();
	num-= CommentariesQty+(CommentariesQty?1:0);
	if(OthersQty && num==0) return CString("");
	if(num<=OthersQty) return Others[num-1]->getPath();
	return CString("");
}
int BQBooklist::getItemEnabled(int num){
	if(num==0) return 0;
	if(num==BiblesQty+(BiblesQty?1:0)) return 0;
	if(num==BiblesQty+(BiblesQty?1:0)+CommentariesQty+(CommentariesQty?1:0)) return 0;
	if(num>=getItemQty())return 0;
	return 1;
}
BQini* BQBooklist::getIni(int num){
	if(BiblesQty && num==0) return 0;
	if(num<BiblesQty+(BiblesQty?1:0)) return Bibles[num-1];
	num-= BiblesQty+(BiblesQty?1:0);
	if(CommentariesQty && num==0) return 0;
	if(num<CommentariesQty+(CommentariesQty?1:0)) return Commentaries[num-1];
	num-= CommentariesQty+(CommentariesQty?1:0);
	if(OthersQty && num==0) return 0;
	if(num<=OthersQty) return Others[num-1];
	return 0;
}
