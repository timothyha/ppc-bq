#include "BQhistory.h"

int BQhistory::load(CString file){
	return 0;
}
int BQhistory::save(int depth){
	return 0;
}
int BQhistory::back(void){
	if(position>0)position--;
	return 0;
}
int BQhistory::forward(void){
	if(position<count)position++;
	return 0;
}
int BQhistory::insert(const char*mod, int bk, int ch, int scr){
	if(position+1>limit){
		// shift
	}
	if(position+1>allocated){
		// add 10
	}
	if(position+1<count){
		list[count] = new BQposition();
		list[count++]->set(mod, bk, ch, scr);
	}
	position++;
	return true;
}
