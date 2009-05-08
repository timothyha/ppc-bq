#pragma once
#ifdef POCKETPC2003_UI_MODEL
#include "resourceppc.h"
#endif 
#ifdef SMARTPHONE2003_UI_MODEL
#include "resourcesp.h"
#endif
#include "BQparams.h"
#include "BQclasses.h"
#include "BQBooklist.h"
#include "BQini.h"
extern CString RunPath;
extern BQBooklist BQbibles;
extern BQBible BQbook;
extern int foundPage;
extern int absIndex;
extern int chIndex;
void createControls(HWND hWnd);
void fillBooksCombo(void);
void reloadChapters(void);
void toggleToolbar(void);
void resizeWindow();
void showFound(int page);
void findVerses(void);
int nextChapter(void);
int prevChapter(void);
void setWindowFont(HWND hWnd);

extern int                 editPosition;
extern int                 ToolbarVisible;
extern int                 StrongsDictionaryExists;
extern int                 regime;
extern CString             RunPath;
extern BQBooklist          BQbibles;
extern BQBible             BQbook;
extern int                 foundPage;
extern int                 foundPageCount;
extern int                 absIndex;
extern int                 chIndex;
extern BQparams            params;
extern bool                leftMousePressed;
extern HFONT               font;
extern HINSTANCE           g_hInst;			// current instance

int ShowChapter(int chapter);
/*
typedef struct _TOOLBARRES
{
	unsigned short lpReserved;
	unsigned short width;
	unsigned short height;
	unsigned short num;
	unsigned short ids[0];
}TOOLBARRES,*LPTOOLBARRES;
HWND CreateResToolBar(HMODULE hExe, HWND hParent, UINT uID);
BOOL DestroyResToolBar(HWND hCommandBar);
*/
int CALLBACK EnumFontFamExProc(ENUMLOGFONT *lpelfe,
	NEWTEXTMETRIC *lpntme,DWORD FontType,LPARAM lParam);
HBRUSH OnCtlColor(HDC dc, HWND hWnd);
int ShowStrong(CString*num);
int DisplayStrongs(int num, int lexicon);



int WinProcessHotKeys(WPARAM wParam, LPARAM lParam);
int WinProcessModuleChange();
int WinProcessViewportCreate();
int WinProcessViewportToggle();
HWND WinProcessCreateButton(HWND hWnd, int id, LPCTSTR caption);
HWND WinProcessCreateCheckBox(HWND hWnd, int id, LPCTSTR caption, bool check);
HWND WinProcessCreateComboBox(HWND hWnd, int id);
HWND WinProcessCreateStatic(HWND hWnd, LPCTSTR caption);

int AtomScrollLineDown();
int AtomScrollLineUp();
int AtomScrollPageUp();
int AtomScrollPageDown();
int AtomScrollSavePosition();
int AtomScrollRestorePosition();

int AtomHotKeysRegister();
int AtomHotKeysUnregister();

int AtomBibleNextChapter();
int AtomBiblePrevChapter();
int AtomBibleCurrentChapter();

int AtomToggleStrongs();
int AtomRegimeChange();
int AtomFulscreenToggle();



int CALLBACK EditWordBreakProc(      
    LPTSTR lpch,
    int ichCurrent,
    int cch,
    int code
);