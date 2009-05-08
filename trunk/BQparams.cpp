#include "stdafx.h"
#include "BQparams.h"
#include "BQclasses.h"
#include "c_ini.h"
#include <stdlib.h>
#include <AtlConv.h>

void BQparams::load(CString lpath){
	path = lpath;
	CIni ini;
	int section,param;
	if(!ini.LoadIniFile(lpath.GetBuffer())) return;
	if((section = ini.GetSectionIndexByName("GENERAL"))>=0){
		if((param=ini.GetParamIndexByName(section,"ShowScrollBar")>=0))
			ShowScrollBar  = atoi(ini.GetParamValue( section, "ShowScrollBar"));
	}
	if((section = ini.GetSectionIndexByName("FULLSCREEN"))>=0){
		if((param=ini.GetParamIndexByName(section,"FSShowScrollBar")>=0))
			FSShowScrollBar  = atoi(ini.GetParamValue( section, "FSShowScrollBar"));
		if((param=ini.GetParamIndexByName(section,"FSUseTaps")>=0))
			FSUseTaps  = atoi(ini.GetParamValue( section, "FSUseTaps"));
	}
	if((section = ini.GetSectionIndexByName("COLOR"))>=0){
		if((param=ini.GetParamIndexByName(section,"ColorFontRed")>=0))
			ColorFontRed  = atoi(ini.GetParamValue( section, "ColorFontRed"));
		if((param=ini.GetParamIndexByName(section,"ColorFontGreen")>=0))
			ColorFontGreen  = atoi(ini.GetParamValue( section, "ColorFontGreen"));
		if((param=ini.GetParamIndexByName(section,"ColorFontBlue")>=0))
			ColorFontBlue  = atoi(ini.GetParamValue( section, "ColorFontBlue"));
		if((param=ini.GetParamIndexByName(section,"ColorBgRed")>=0))
			ColorBgRed  = atoi(ini.GetParamValue( section, "ColorBgRed"));
		if((param=ini.GetParamIndexByName(section,"ColorBgGreen")>=0))
			ColorBgGreen  = atoi(ini.GetParamValue( section, "ColorBgGreen"));
		if((param=ini.GetParamIndexByName(section,"ColorBgBlue")>=0))
			ColorBgBlue  = atoi(ini.GetParamValue( section, "ColorBgBlue"));
	}
	if((section = ini.GetSectionIndexByName("HISTORY"))>=0){
		currentPosition.set(ini.GetParamValue( section, "Module"), atoi(ini.GetParamValue( section, "Book")), atoi(ini.GetParamValue( section, "Chapter")),0);
	}
	if((section = ini.GetSectionIndexByName("FONT"))>=0){
		if((param=ini.GetParamIndexByName(section,"TextFontHeight")>=0))
			TextFontHeight = atoi(ini.GetParamValue( section, "TextFontHeight"));
		if((param=ini.GetParamIndexByName(section,"TextFontWeight")>=0))
			TextFontWeight = atoi(ini.GetParamValue( section, "TextFontWeight"));
		if((param=ini.GetParamIndexByName(section,"TextFontItalic")>=0))
			TextFontItalic = atoi(ini.GetParamValue( section, "TextFontItalic"));
		if((param=ini.GetParamIndexByName(section,"TextFontFace")>=0))
			TextFontFace   = CString(ini.GetParamValue( section, "TextFontFace"));
	}
	changed = false;
}
void BQparams::save(void){
	if(!changed) return;
	char out[1000];
	char s[255];
	char* p;
	HANDLE file;
	DWORD written;
	p = out;
	// general part:
	strcpy(p, "[GENERAL]\r\n"); p+= strlen(p);
	strcpy(p, "ShowScrollBar="); p+= strlen(p);
	_itoa(ShowScrollBar, p, 10);p+= strlen(p);
	strcpy(p, "\r\n"); p+= strlen(p);
	// fullscreen part:
	strcpy(p, "[FULLSCREEN]\r\n"); p+= strlen(p);
	strcpy(p, "FSShowScrollBar="); p+= strlen(p);
	_itoa(FSShowScrollBar, p, 10);p+= strlen(p);
	strcpy(p, "\r\n"); p+= strlen(p);
	strcpy(p, "FSUseTaps="); p+= strlen(p);
	_itoa(FSUseTaps, p, 10);p+= strlen(p);
	strcpy(p, "\r\n"); p+= strlen(p);
	// color part:
	strcpy(p, "[COLOR]\r\n"); p+= strlen(p);
	strcpy(p, "ColorFontRed="); p+= strlen(p);
	_itoa(ColorFontRed, p, 10);p+= strlen(p);
	strcpy(p, "\r\n"); p+= strlen(p);
	strcpy(p, "ColorFontGreen="); p+= strlen(p);
	_itoa(ColorFontGreen, p, 10);p+= strlen(p);
	strcpy(p, "\r\n"); p+= strlen(p);
	strcpy(p, "ColorFontBlue="); p+= strlen(p);
	_itoa(ColorFontBlue, p, 10);p+= strlen(p);
	strcpy(p, "\r\n"); p+= strlen(p);
	strcpy(p, "ColorBgRed="); p+= strlen(p);
	_itoa(ColorBgRed, p, 10);p+= strlen(p);
	strcpy(p, "\r\n"); p+= strlen(p);
	strcpy(p, "ColorBgGreen="); p+= strlen(p);
	_itoa(ColorBgGreen, p, 10);p+= strlen(p);
	strcpy(p, "\r\n"); p+= strlen(p);
	strcpy(p, "ColorBgBlue="); p+= strlen(p);
	_itoa(ColorBgBlue, p, 10);p+= strlen(p);
	strcpy(p, "\r\n"); p+= strlen(p);
	strcpy(p, "[HISTORY]\r\n"); p+= strlen(p);
	strcpy(p, "Module="); p+= strlen(p);
	strcpy(p, currentPosition.moduleShortName);p+= strlen(p);
	strcpy(p, "\r\n"); p+= strlen(p);
	strcpy(p, "Book="); p+= strlen(p);
	_itoa(currentPosition.bookAbsIndex, p, 10);p+= strlen(p);
	strcpy(p, "\r\n"); p+= strlen(p);
	strcpy(p, "Chapter="); p+= strlen(p);
	_itoa(currentPosition.chapter, p, 10);p+= strlen(p);
	strcpy(p, "\r\n"); p+= strlen(p);
	strcpy(p, "[FONT]\r\n"); p+= strlen(p);
	strcpy(p, "TextFontHeight="); p+= strlen(p);
	_itoa(TextFontHeight, p, 10);p+= strlen(p);
	strcpy(p, "\r\n"); p+= strlen(p);
	strcpy(p, "TextFontWeight="); p+= strlen(p);
	_itoa(TextFontWeight, p, 10);p+= strlen(p);
	strcpy(p, "\r\n"); p+= strlen(p);
	strcpy(p, "TextFontItalic="); p+= strlen(p);
	_itoa(TextFontItalic, p, 10);p+= strlen(p);
	strcpy(p, "\r\n"); p+= strlen(p);
	strcpy(p, "TextFontFace="); p+= strlen(p);
	if(file = CreateFile(path.GetBuffer(),GENERIC_WRITE,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL)){
		SetEndOfFile(file);
		WriteFile(file, out, (p-out), (LPDWORD)&written,NULL);
		UnicodeToANSI((LPWSTR)TextFontFace.GetString(),(LPSTR)s);
		WriteFile(file, s, strlen(s), (LPDWORD)&written,NULL);
		CloseHandle(file);
	}
}

BOOL UnicodeToANSI(LPWSTR Wstr,LPSTR Astr)
{
	return WideCharToMultiByte(CP_ACP,0,Wstr,-1,Astr,128,NULL,NULL);
}

BOOL ANSI2Unicode(LPSTR Astr,LPWSTR Wstr)
{
	return MultiByteToWideChar(CP_ACP,0,Astr,-1,Wstr,128);
}