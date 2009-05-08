#include <afx.h>
#include <atlstr.h>
#include "BQhistory.h"

class BQparams{
	CString path;
	// toolbar part:
	int ToolbarMenu;
	int ToolbarFind;
	int ToolbarCopy;
	int ToolbarControls;
	int ToolbarChapterNext;
	int ToolbarChapterPrev;
	int ToolbarEasyNav;
	int ToolbarHistory;
	int ToolbarForward;
	int ToolbarBack;
	int ToolbarOptions;
	int ToolbarFullScreen;
	int ToolbarHelp;
	// general part:
	int ShowScrollBar;
	// fullscreen part:
	int FSShowScrollBar;
	int FSUseTaps;
	// color part:
	int ColorFontRed;
	int ColorFontGreen;
	int ColorFontBlue;
	int ColorBgRed;
	int ColorBgGreen;
	int ColorBgBlue;
	// font part:
	int TextFontHeight;
	int TextFontWidth;              // average character width
	int TextFontEscapement;         // angle of escapement
	int TextFontOrientation;        // base-line orientation angle
	int TextFontWeight;             // font weight
	DWORD TextFontItalic;           // italic attribute option
	DWORD TextFontUnderline;        // underline attribute option
	DWORD TextFontStrikeOut;        // strikeout attribute option
	DWORD TextFontCharSet;          // character set identifier
	DWORD TextFontOutputPrecision;  // output precision
	DWORD TextFontClipPrecision;    // clipping precision
	DWORD TextFontQuality;          // output quality
	DWORD TextFontPitchAndFamily;   // pitch and family
	CString TextFontFace;           // typeface name
	bool changed;
public:
	//navigation part:
	BQposition			currentPosition;
public:
	BQparams(){
		ToolbarMenu              = 1;
		ToolbarFind              = 1;
		ToolbarCopy              = 1;
		ToolbarControls          = 1;
		ToolbarChapterNext       = 0;
		ToolbarChapterPrev       = 0;
		ToolbarEasyNav           = 0;
		ToolbarHistory           = 0;
		ToolbarForward           = 0;
		ToolbarBack              = 0;
		ToolbarOptions           = 0;
		ToolbarFullScreen        = 0;
		ToolbarHelp              = 0;
		// general part:
		ShowScrollBar   = 1;
		// fullscreen part:
		FSShowScrollBar = 0;
		FSUseTaps       = 1;
		// color part:
		ColorFontRed   = 255;
		ColorFontGreen = 255;
		ColorFontBlue  = 255;
		ColorBgRed     = 0;
		ColorBgGreen   = 0;
		ColorBgBlue    = 0;
		// font part:
		TextFontHeight           = 14;
		TextFontWidth            = NULL;        // average character width
		TextFontEscapement       = 0;        // angle of escapement
		TextFontOrientation      = 0;        // base-line orientation angle
		TextFontWeight           = 0;        // font weight
		TextFontItalic           = 0;        // italic attribute option
		TextFontUnderline        = 0;        // underline attribute option
		TextFontStrikeOut        = 0;        // strikeout attribute option
		TextFontCharSet          = NULL;          // character set identifier
		TextFontOutputPrecision  = NULL;  // output precision
		TextFontClipPrecision    = NULL;    // clipping precision
		TextFontQuality          = NULL;          // output quality
		TextFontPitchAndFamily   = NULL;   // pitch and family
		TextFontFace             = CString(TEXT("Tahoma"));  // typeface name
		changed                  = true;
	};

	int getShowScrollBar(){return ShowScrollBar;};

	int getFSShowScrollBar(){return FSShowScrollBar;};
	int getFSUseTaps(){return FSUseTaps;};

	int getColorFontRed(){return ColorFontRed;};
	int getColorFontGreen(){return ColorFontGreen;};
	int getColorFontBlue(){return ColorFontBlue;};
	int getColorBgRed(){return ColorBgRed;};
	int getColorBgGreen(){return ColorBgGreen;};
	int getColorBgBlue(){return ColorBgBlue;};

	int getTextFontHeight(){return TextFontHeight;};
	int getTextFontWidth(){return TextFontWidth;};
	int getTextFontEscapement(){return TextFontEscapement;};
	int getTextFontOrientation(){return TextFontOrientation;};
	int getTextFontWeight(){return TextFontWeight;};
	DWORD getTextFontItalic(){return TextFontItalic;};
	DWORD getTextFontUnderline(){return TextFontUnderline;};
	DWORD getTextFontStrikeOut(){return TextFontStrikeOut;};
	DWORD getTextFontCharSet(){return TextFontCharSet;};
	DWORD getTextFontOutputPrecision(){return TextFontOutputPrecision;};
	DWORD getTextFontClipPrecision(){return TextFontClipPrecision;};
	DWORD getTextFontQuality(){return TextFontQuality;};
	DWORD getTextFontPitchAndFamily(){return TextFontPitchAndFamily;};
	CString getTextFontFace(){return TextFontFace;};

	void setShowScrollBar(int par){if(ShowScrollBar!=par)changed = true; ShowScrollBar = (par==0?0:1);};

	void setFSShowScrollBar(int par){if(FSShowScrollBar!=par)changed = true; FSShowScrollBar= (par==0?0:1);};
	void setFSUseTaps(int par){if(FSUseTaps!=par)changed = true; FSUseTaps = (par==0?0:1);};

	void setColorFontRed(int par){if(ColorFontRed!=par)changed = true; ColorFontRed = par;};
	void setColorFontGreen(int par){if(ColorFontGreen!=par)changed = true; ColorFontGreen = par;};
	void setColorFontBlue(int par){if(ColorFontBlue!=par)changed = true; ColorFontBlue = par;};
	void setColorBgRed(int par){if(ColorBgRed!=par)changed = true; ColorBgRed = par;};
	void setColorBgGreen(int par){if(ColorBgGreen!=par)changed = true; ColorBgGreen = par;};
	void setColorBgBlue(int par){if(ColorBgBlue!=par)changed = true; ColorBgBlue = par;};

	void setTextFontHeight(int par){if(TextFontHeight!=par)changed = true;TextFontHeight = par;};
	void setTextFontWidth(int par){if(TextFontWidth!=par)changed = true;TextFontWidth = par;};
	void setTextFontEscapement(int par){if(TextFontEscapement!=par)changed = true;TextFontEscapement = par;};
	void setTextFontOrientation(int par){if(TextFontOrientation!=par)changed = true;TextFontOrientation = par;};
	void setTextFontWeight(int par){if(TextFontWeight!=par)changed = true;TextFontWeight = par;};
	void setTextFontItalic(DWORD par){if(TextFontItalic!=par)changed = true;TextFontItalic = par;};
	void setTextFontUnderline(DWORD par){if(TextFontUnderline!=par)changed = true;TextFontUnderline = par;};
	void setTextFontStrikeOut(DWORD par){if(TextFontStrikeOut!=par)changed = true;TextFontStrikeOut = par;};
	void setTextFontCharSet(DWORD par){if(TextFontCharSet!=par)changed = true;TextFontCharSet = par;};
	void setTextFontOutputPrecision(DWORD par){if(TextFontOutputPrecision!=par)changed = true;TextFontOutputPrecision = par;};
	void setTextFontClipPrecision(DWORD par){if(TextFontClipPrecision!=par)changed = true;TextFontClipPrecision = par;};
	void setTextFontQuality(DWORD par){if(TextFontQuality!=par)changed = true;TextFontQuality = par;};
	void setTextFontPitchAndFamily(DWORD par){if(TextFontPitchAndFamily!=par)changed = true;TextFontPitchAndFamily = par;};
	void setTextFontFace(CString par){if(TextFontFace.CompareNoCase(par)!=0)changed = true;TextFontFace = par;};
	void setChanged(bool ch){changed = ch;}

	void load(CString path);
	void save(void);
};

BOOL UnicodeToANSI(LPWSTR Wstr,LPSTR Astr);
BOOL ANSI2Unicode(LPSTR Astr,LPWSTR Wstr);
