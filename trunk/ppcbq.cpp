// ppcbq.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "ppcbq.h"
#include "dialog.h"
#include "c_ini.h"
#include "FullScreen.h"
#include <windows.h>
#include <commctrl.h>

#define MAX_LOADSTRING 100

#define ID_BUTTON_PLUS               16000
#define ID_BUTTON_FIND               16001
#define ID_BUTTON_FIND_PREV          16002
#define ID_BUTTON_FIND_NEXT          16003
#define ID_CHECK_FIND_WHOLE_WORDS    16004
#define ID_CHECK_FIND_CASE_SENSITIVE 16005
#define ID_CHECK_FIND_CURRENT_BOOK   16006
#define ID_CHECK_FIND_VERSE_GROUPS   16007
#define ID_COMBO_MODULE              17000
#define ID_COMBO_BOOK                17001
#define ID_COMBO_CHAPTER             17002
#define ID_EDIT                      18000
#define IDM_EDIT                     18001
#define IDK_DOWN                     19000
#define IDK_UP                       19001
#define IDK_LEFT                     19002
#define IDK_RIGHT                    19003
#define IDK_FULLSCREEN               19004

#define WM_EDITDBLCLICK				7770001
#define WM_EDITTAP				    7770002

// Global Variables:
HINSTANCE			g_hInst;			// current instance
HWND				g_hWndMenuBar;		// menu bar handle
HWND				hwndWindow;

HWND				hwndEdit;
HWND				hwndEditScroll;
HWND				hwndEditNoscroll;
HWND				hwndSelectModule;
HWND				hwndSelectBible;
HWND				hwndSelectBook;
HWND				hwndSelectChapter;
HWND				hwndButtonPlus;
HWND				hwndButtonFind;
HWND				hwndButtonFindPrev;
HWND				hwndButtonFindNext;
HWND				hwndEditFind;
HWND				hwndCheckFindWholeWords;
HWND				hwndCheckFindCurrentBook;
HWND				hwndCheckFindCaseSensitive;
HWND				hwndCheckFindVerseGroups;

HWND				lastMessageTarget;

HBRUSH hDlgBrush; // кисть
HBRUSH hEditBrush;

HFONT               font;

#define REGIME_VIEW 1
#define REGIME_FIND 2
#define REGIME_STRONG 3
#define REGIME_DICT 4

int                 editPosition;
int					ToolbarVisible;
int					StrongsDictionaryExists;
int					regime;
CString             RunPath;
BQBooklist          BQbibles;
BQBible             BQbook;
int                 foundPage;
int                 foundPageCount;
int                 absIndex;
int                 chIndex;
bool                fullScreen;
FullScreen          fullScreenObject;
BQparams            params;
bool                StrongsVisible;
bool                leftMousePressed=false;
bool                IsBeingResized=false;

MSG lastClickMsg;

// Forward declarations of functions included in this code module:
ATOM			MyRegisterClass(HINSTANCE, LPTSTR);
BOOL			InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
#ifndef WIN32_PLATFORM_WFSP
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
#endif // !WIN32_PLATFORM_WFSP

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
	MSG msg;

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow)) 
	{
		return FALSE;
	}

#ifndef WIN32_PLATFORM_WFSP
	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PPCBQ));
#endif // !WIN32_PLATFORM_WFSP
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (msg.hwnd == hwndEdit && msg.message == WM_KEYDOWN && VK_UP==(int)msg.wParam) {
			AtomScrollPageUp();
		}
		else if (msg.hwnd == hwndEdit && msg.message == WM_KEYDOWN && VK_DOWN==(int)msg.wParam) {
			AtomScrollPageDown();
		}
		else if (msg.hwnd == hwndEdit && msg.message == WM_KEYDOWN && VK_LEFT==(int)msg.wParam) {
			AtomScrollLineUp();
		}
		else if (msg.hwnd == hwndEdit && msg.message == WM_KEYDOWN && VK_RIGHT==(int)msg.wParam) {
			AtomScrollLineDown();
		}
		else if (msg.hwnd == hwndEdit && msg.message == WM_KEYDOWN && VK_RETURN==(int)msg.wParam) {
			AtomFulscreenToggle();
		}
		else{
			if (fullScreen && params.getFSUseTaps() && msg.hwnd == hwndEdit && msg.message == WM_LBUTTONDOWN) {
				leftMousePressed=true;
				continue;
			}
			if (/*!StrongsVisible && */fullScreen && params.getFSUseTaps() && msg.hwnd == hwndEdit && msg.message == WM_LBUTTONUP && leftMousePressed) {
				WndProc(hwndWindow, WM_EDITTAP, 0, (LPARAM)&msg);
				leftMousePressed=false;
			}
			else if (/*StrongsVisible && */msg.hwnd == hwndEdit && msg.message == WM_LBUTTONDBLCLK) {
				WndProc(hwndWindow, WM_EDITDBLCLICK, 0, (LPARAM)&msg);
				leftMousePressed=false;
			}
			else{
#ifndef WIN32_PLATFORM_WFSP
				if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
#endif // !WIN32_PLATFORM_WFSP
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
			if (msg.message == WM_LBUTTONUP) {
				leftMousePressed=false;
			}
		}
		HideCaret(hwndEdit);
	}

	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{
	WNDCLASS wc;

	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PPCBQ));
	wc.hCursor       = 0;
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClass;

	return RegisterClass(&wc);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;
    TCHAR szTitle[MAX_LOADSTRING];		// title bar text
    TCHAR szWindowClass[MAX_LOADSTRING];	// main window class name

    g_hInst = hInstance; // Store instance handle in our global variable

#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
    // SHInitExtraControls should be called once during your application's initialization to initialize any
    // of the device specific controls such as CAPEDIT and SIPPREF.
    SHInitExtraControls();
#endif // WIN32_PLATFORM_PSPC || WIN32_PLATFORM_WFSP

    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING); 
    LoadString(hInstance, IDC_PPCBQ, szWindowClass, MAX_LOADSTRING);

#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
    //If it is already running, then focus on the window, and exit
    hWnd = FindWindow(szWindowClass, szTitle);	
    if (hWnd) 
    {
        // set focus to foremost child window
        // The "| 0x00000001" is used to bring any owned windows to the foreground and
        // activate them.
        SetForegroundWindow((HWND)((ULONG) hWnd | 0x00000001));
        return 0;
    } 
#endif // WIN32_PLATFORM_PSPC || WIN32_PLATFORM_WFSP

    if (!MyRegisterClass(hInstance, szWindowClass))
    {
    	return FALSE;
    }

		WNDCLASS wc;

	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = DlgPreviewProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = g_hInst;
	wc.hIcon         = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_PPCBQ));
	wc.hCursor       = 0;
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = TEXT("PREVIEW");

	if(!RegisterClass(&wc))
    {
    	return FALSE;
    }

	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = DlgColorControlProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = g_hInst;
	wc.hIcon         = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_PPCBQ));
	wc.hCursor       = 0;
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = TEXT("COLORSLIDER");

	if(!RegisterClass(&wc))
    {
    	return FALSE;
    }


    hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        return FALSE;
    }

#ifdef WIN32_PLATFORM_PSPC
    // When the main window is created using CW_USEDEFAULT the height of the menubar (if one
    // is created is not taken into account). So we resize the window after creating it
    // if a menubar is present
    if (g_hWndMenuBar)
    {
        RECT rc;
        RECT rcMenuBar;

        GetWindowRect(hWnd, &rc);
        GetWindowRect(g_hWndMenuBar, &rcMenuBar);
        rc.bottom -= (rcMenuBar.bottom - rcMenuBar.top);
		
        MoveWindow(hWnd, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, FALSE);
    }
#endif // WIN32_PLATFORM_PSPC

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);


    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	hwndWindow = hWnd;
    int wmId, wmEvent;
	int mesRes; //message result
    PAINTSTRUCT ps;
    HDC hdc;
	RECT rc;
	WCHAR szPath[MAX_PATH];
	int count;
	CString chInd;

#if defined(SHELL_AYGSHELL) && !defined(WIN32_PLATFORM_WFSP)
    static SHACTIVATEINFO s_sai;
#endif // SHELL_AYGSHELL && !WIN32_PLATFORM_WFSP
	
    wmId    = LOWORD(wParam); 
    wmEvent = HIWORD(wParam); 
	int tmpMod, tmpBk, tmpCh, pos1, pos2;
	LPCWSTR* tmpStr0;
	CString AllText;
	CString PartText;
	int tmpWPar, tmpLPar, length;
	MSG*msg;
    switch (message) 
    {
		case WM_EDITTAP:
			msg = (MSG*)(lParam);
			GetClientRect(hwndEdit, &rc);
			if(msg->pt.y<(rc.bottom+rc.top)/2) AtomScrollPageUp();
			else AtomScrollPageDown();
			break;
		case WM_EDITDBLCLICK:
			SendMessage(hwndEdit, EM_GETSEL, (WPARAM)&tmpWPar, (LPARAM)&tmpLPar);
			tmpStr0 = new LPCWSTR[((length = GetWindowTextLength(hwndEdit))+1)*sizeof(LPCWSTR)];
			GetWindowText(hwndEdit, (LPWSTR)tmpStr0, GetWindowTextLength(hwndEdit));
			AllText = CString((LPWSTR)tmpStr0);
			delete tmpStr0;
			AllText.Replace(TEXT(","),TEXT(" "));
			AllText.Replace(TEXT("."),TEXT(" "));
			AllText.Replace(TEXT("?"),TEXT(" "));
			AllText.Replace(TEXT("!"),TEXT(" "));
			AllText.Replace(TEXT(":"),TEXT(" "));
			AllText.Replace(TEXT(";"),TEXT(" "));
			AllText.Replace(TEXT("-"),TEXT(" "));
			AllText.Replace(TEXT("("),TEXT(" "));
			AllText.Replace(TEXT(")"),TEXT(" "));
			AllText.Replace(TEXT("["),TEXT(" "));
			AllText.Replace(TEXT("]"),TEXT(" "));
			AllText.Replace(TEXT("\r"),TEXT(" "));
			//AllText.Replace(TEXT("\n"),TEXT(" "));
			AllText.Replace(TEXT("\""),TEXT(" "));
			AllText.Replace(TEXT("'"),TEXT(" "));
			pos1 = AllText.Find(TEXT(" "), tmpWPar>20?tmpWPar-20:0);
			while(pos1<length && (pos2 = AllText.Find(TEXT(" "), pos1))<tmpWPar && pos2>=0) pos1 = pos2<length-1 ? pos2+1 : pos2;
			if(pos1<length){
				pos2 = AllText.Find(TEXT(" "), tmpWPar);
			}
			else pos2 = pos1;
			PartText = AllText.Mid(pos1, pos2-pos1);
			if(PartText.GetLength()>0){
				pos1 = _wtoi(PartText+TEXT("1"));
				if (pos1>0 && PartText.GetAt(0)!=0x000A){
					ShowStrong(&PartText);
				}
			}
			break;
		case WM_COMMAND:
            // Parse the menu selections:
            switch (wmId)
            {
                case IDM_OK:
					SendMessage(hWnd, WM_DESTROY, 0, 0);
					break;
                case IDM_FIND:
					AtomRegimeChange();
                    break;
                case ID_BUTTON_FIND:
					findVerses();
                    break;
                case ID_BUTTON_FIND_PREV:
					if(foundPage>0) showFound(--foundPage);
                    break;
                case ID_BUTTON_FIND_NEXT:
					if(foundPage<foundPageCount-1) showFound(++foundPage);
                    break;
#ifndef WIN32_PLATFORM_WFSP
                case IDM_HELP_ABOUT:
                    DialogBox(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, About);
					WinProcessViewportToggle();
					setWindowFont(hwndEdit);
                    break;
#endif // !WIN32_PLATFORM_WFSP
				case ID_BUTTON_PLUS:
					if(wmEvent==BN_CLICKED)
						toggleToolbar();
					break;
				case ID_COMBO_MODULE:
					if(wmEvent==CBN_SELCHANGE)
						WinProcessModuleChange();
					break;
				case ID_COMBO_BOOK:
					if(wmEvent==CBN_SELCHANGE)
					{
						reloadChapters();
						mesRes = SendMessage(hwndSelectChapter, CB_SETCURSEL, 0, 0);  
						ShowChapter(1);
						chIndex = -BQbook.getIni()->getHasChapterZero();
						SetFocus(hwndEdit);
					}
					break;
				case ID_COMBO_CHAPTER:
					if(wmEvent==CBN_SELCHANGE)
					{
						mesRes  = SendMessage(hwndSelectChapter, CB_GETCURSEL, 0, 0);  
						chIndex = mesRes-BQbook.getIni()->getHasChapterZero();
						ShowChapter(mesRes+1);
						SetFocus(hwndEdit);
					}
					break;
				case IDM_COPY:
					SendMessage(hwndEdit, WM_COPY, 0, 0);
					break;
				case ID_ACTIONS_STRONGS:
					AtomToggleStrongs();
					break;
				case IDM_FULLSCREEN:
					AtomFulscreenToggle();
					break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        case WM_CREATE:
#ifdef SHELL_AYGSHELL
            SHMENUBARINFO mbi;
			ToolbarVisible = false;
			regime = REGIME_VIEW;

			// GET START DIR:
			GetModuleFileName(0,szPath,MAX_PATH);
			(*((wcsrchr(szPath,'\\'))+1))=0;
			RunPath = CString(szPath);

			params.load(RunPath+TEXT("bq.ini"));

			createControls(hWnd);

			memset(&mbi, 0, sizeof(SHMENUBARINFO));
            mbi.cbSize     = sizeof(SHMENUBARINFO);
            mbi.hwndParent = hWnd;
            mbi.nToolBarId = IDR_MENU;
            mbi.hInstRes   = g_hInst;

            if (!SHCreateMenuBar(&mbi)) 
            {
                g_hWndMenuBar = NULL;
            }
            else
            {
                g_hWndMenuBar = mbi.hwndMB;
            }
	
#ifndef WIN32_PLATFORM_WFSP
            // Initialize the shell activate info structure
            memset(&s_sai, 0, sizeof (s_sai));
            s_sai.cbSize = sizeof (s_sai);
#endif // !WIN32_PLATFORM_WFSP
#endif // SHELL_AYGSHELL

			fullScreen = false;
			fullScreenObject.InitFullScreen(hWnd);

			// INIT BIBLE:
			BQbibles.load(RunPath);
			count = BQbibles.getItemQty();
			tmpMod=1, tmpBk=0, tmpCh=0;
			for(int i=0; i<count; i++){
				SendMessage(hwndSelectModule, CB_ADDSTRING, 0, (LPARAM)BQbibles.getItemName(i).GetBuffer());
				if(params.currentPosition.moduleShortName){
					//tmpStr1 = BQbibles.getItemCharShortName(i);
					//tmpStr2 = params.currentPosition.moduleShortName;
					if(strcmp(BQbibles.getItemCharShortName(i), params.currentPosition.moduleShortName)==0){
						tmpMod = i;
						if(BQbibles.getIni(i)->getIsBible())
							tmpBk  = BQbibles.getIni(i)->getBookIndexByAbsolute(params.currentPosition.bookAbsIndex);
						else 
							tmpBk  = params.currentPosition.bookAbsIndex - 1;
						tmpCh  = params.currentPosition.chapter + BQbibles.getIni(i)->getHasChapterZero() - 1;
					}
				}
			}
		// Books combo box 
			mesRes = SendMessage(hwndSelectModule, CB_SETCURSEL, tmpMod, 0);  
			fillBooksCombo();
			absIndex = BQbook.getAbsIndex();
			mesRes = SendMessage(hwndSelectBook, CB_SETCURSEL, tmpBk, 0);  
			reloadChapters();
			mesRes = SendMessage(hwndSelectChapter, CB_SETCURSEL, tmpCh, 0);  
			ShowChapter(tmpCh+1);
			SetFocus(hwndEdit);
			break;
        case WM_HOTKEY:
			WinProcessHotKeys(wParam, lParam);
			break;
        case WM_SIZE:
			if(!IsBeingResized)
			{
				IsBeingResized = true;
				GetWindowRect(hwndWindow, &rc);
				if(fullScreen && (rc.bottom - rc.top < GetSystemMetrics(SM_CYSCREEN)))
				{
					SetWindowPos(hWnd,HWND_NOTOPMOST,0,0,GetSystemMetrics(SM_CXSCREEN),
					    GetSystemMetrics(SM_CYSCREEN), SWP_SHOWWINDOW);
				}
				resizeWindow();
				IsBeingResized = false;
			}
            break;
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            
            // TODO: Add any drawing code here...
            
            EndPaint(hWnd, &ps);
            break;
        case WM_DESTROY:
			params.save();
			//fullScreenObject.DoFullScreen(false);
#ifdef SHELL_AYGSHELL
            CommandBar_Destroy(g_hWndMenuBar);
#endif // SHELL_AYGSHELL
            PostQuitMessage(0);
            break;

#if defined(SHELL_AYGSHELL) && !defined(WIN32_PLATFORM_WFSP)
        case WM_ACTIVATE:    // Notify shell of our activate message
            SHHandleWMActivate(hWnd, wParam, lParam, &s_sai, FALSE);
			if(HIWORD(wParam)!=0||LOWORD(wParam)==WA_INACTIVE)
			{
				AtomHotKeysUnregister();
				if(fullScreen)
				{
					fullScreenObject.FullScreenBlur();
				}
			}
			else
			{
				AtomHotKeysRegister();
				if(fullScreen) 
				{
					fullScreenObject.FullScreenOn();
					if(regime!=REGIME_VIEW) ShowWindow(g_hWndMenuBar, SW_SHOW);
					resizeWindow();
				}
				SetFocus(hwndEdit);
			}
            //return DefWindowProc(hWnd, message, wParam, lParam);
            break;
        case WM_SETTINGCHANGE:
            SHHandleWMSettingChange(hWnd, wParam, lParam, &s_sai);
            break;
#endif // SHELL_AYGSHELL && !WIN32_PLATFORM_WFSP
		case WM_CTLCOLORBTN:
			return (LRESULT)OnCtlColor((HDC)wParam, (HWND)lParam);
		case WM_ERASEBKGND:
			GetClientRect(hwndEdit, &rc );
			FillRect((HDC)wParam, &rc, CreateSolidBrush(RGB(params.getColorBgRed(),params.getColorBgGreen(),params.getColorBgBlue())));
			return 1;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
HBRUSH OnCtlColor(HDC dc, HWND hWnd)
{
	HBRUSH   hbr  = NULL;
	COLORREF cr;
	COLORREF crbg;
	BOOL     mode = TRANSPARENT;
	UINT     ctrl = GetDlgCtrlID(hWnd);
	switch(ctrl)
	{
		case ID_EDIT: 
				SetBkMode(dc, mode);
				cr = RGB(params.getColorFontRed(), params.getColorFontGreen(), params.getColorFontBlue()); // текст
				SetTextColor(dc, cr);
				crbg = RGB(params.getColorBgRed(), params.getColorBgGreen(), params.getColorBgBlue()); // текст
				SetBkColor(dc, crbg);
				hbr = CreateSolidBrush(crbg);
				break;
		default: return NULL;
	}
	return hbr;
}

void createControls(HWND hWnd)
{
	hDlgBrush = CreateSolidBrush(RGB(0xc5, 0xc5, 0xc5));
	hEditBrush = CreateSolidBrush(RGB(56, 118, 34));
	hwndButtonPlus    = WinProcessCreateButton(hWnd, ID_BUTTON_PLUS, (LPCTSTR)TEXT("+"));
	hwndSelectModule  = WinProcessCreateComboBox(hWnd, ID_COMBO_MODULE);
	hwndSelectBook    = WinProcessCreateComboBox(hWnd, ID_COMBO_BOOK);
	hwndSelectChapter = WinProcessCreateComboBox(hWnd, ID_COMBO_CHAPTER);
	WinProcessViewportCreate();
	hwndEditFind = CreateWindow(
		TEXT ("edit"), NULL,
		WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL,
		0,0,0,0,
		hWnd, (HMENU)IDM_EDIT,//GetMenu(hWnd),
		g_hInst, NULL
		);
	hwndButtonFind = WinProcessCreateButton(hWnd, ID_BUTTON_FIND, (LPCTSTR)TEXT("GO"));
	hwndButtonFindPrev = WinProcessCreateButton(hWnd, ID_BUTTON_FIND_PREV, (LPCTSTR)TEXT("<"));
	hwndButtonFindNext = WinProcessCreateButton(hWnd, ID_BUTTON_FIND_NEXT, (LPCTSTR)TEXT(">"));
	hwndCheckFindWholeWords = WinProcessCreateCheckBox(hWnd, ID_CHECK_FIND_WHOLE_WORDS, (LPCTSTR)TEXT("Whole words"), false);
	hwndCheckFindCaseSensitive = WinProcessCreateCheckBox(hWnd, ID_CHECK_FIND_CASE_SENSITIVE, (LPCTSTR)TEXT("Case sensitive"), false);
	hwndCheckFindCurrentBook = WinProcessCreateCheckBox(hWnd, ID_CHECK_FIND_CURRENT_BOOK, (LPCTSTR)TEXT("Current book"), false);
	hwndCheckFindVerseGroups = WinProcessCreateCheckBox(hWnd, ID_CHECK_FIND_VERSE_GROUPS, (LPCTSTR)TEXT("Verse groups"), false);

	SHInitExtraControls();
	CreateWindowEx(0, L"SIPPREF", NULL, WS_CHILD, 10, 10, 5, 5, hWnd, NULL, g_hInst, NULL);
	setWindowFont(hwndEdit);
}
void setWindowFont(HWND hWnd){
	font = CreateFont(
		params.getTextFontHeight(),           // height of font
		params.getTextFontWidth(),            // average character width
		params.getTextFontEscapement(),       // angle of escapement
		params.getTextFontOrientation(),      // base-line orientation angle
		params.getTextFontWeight(),           // font weight
		params.getTextFontItalic(),           // italic attribute option
		params.getTextFontUnderline(),        // underline attribute option
		params.getTextFontStrikeOut(),        // strikeout attribute option
		params.getTextFontCharSet(),          // character set identifier
		params.getTextFontOutputPrecision(),  // output precision
		params.getTextFontClipPrecision(),    // clipping precision
		params.getTextFontQuality(),          // output quality
		params.getTextFontPitchAndFamily(),   // pitch and family
		params.getTextFontFace()              // typeface name
	);
	SendMessage( hWnd, WM_SETFONT, (WPARAM) font, true);  
}

void fillBooksCombo()
{
	int mesRes; //message result
	mesRes = SendMessage(hwndSelectBook, CB_RESETCONTENT, 0, 0);  
	mesRes = SendMessage( hwndSelectModule, CB_GETCURSEL, 0, 0);  
	BQbook.setIni(BQbibles.getIni(mesRes));
	int count = BQbook.getBookQty();
	for(int i=1;i<=count;i++){
		mesRes = SendMessage(hwndSelectBook, CB_ADDSTRING, 0, (LPARAM)BQbook.getBookFullName(i).GetBuffer());  
	}
}
void reloadChapters()
{
	int mesRes; //message result
	mesRes = SendMessage(hwndSelectChapter, CB_RESETCONTENT, 0, 0);  
	mesRes = SendMessage( hwndSelectBook, CB_GETCURSEL, 0, 0);  
	CString(chInd);
	BQbook.loadBook(mesRes+1);
	absIndex = BQbook.getAbsIndex();
	int count = BQbook.getBookChapterQty(mesRes+1);
	for(int i=1;i<=count;i++){
		chInd.Format(_T("%d"),i-BQbook.getIni()->getHasChapterZero());
		mesRes = SendMessage(hwndSelectChapter, CB_ADDSTRING, 0, (LPARAM)chInd.GetBuffer());  
	}
}
void toggleToolbar()
{
	ToolbarVisible = !ToolbarVisible;
	if(ToolbarVisible)
	{
		SetWindowText( hwndButtonPlus, (LPCTSTR)"-" );
	}else{
		SetWindowText( hwndButtonPlus, (LPCTSTR)"+" );
	}
	SendMessage(hwndWindow, WM_SIZE, 0, 0);  
}
void resizeWindow()
{
	RECT rc, rc1;

	ShowWindow(hwndButtonPlus,             regime==REGIME_VIEW || regime==REGIME_FIND);
	ShowWindow(hwndSelectModule,           regime==REGIME_VIEW && ToolbarVisible);  
	ShowWindow(hwndSelectBook,             regime==REGIME_VIEW);  
	ShowWindow(hwndSelectChapter,          regime==REGIME_VIEW);  
	ShowWindow(hwndEditFind,               regime==REGIME_FIND);  
	ShowWindow(hwndButtonFind,             regime==REGIME_FIND);  
	ShowWindow(hwndButtonFindPrev,         regime==REGIME_FIND);  
	ShowWindow(hwndButtonFindNext,         regime==REGIME_FIND);  
	ShowWindow(hwndCheckFindWholeWords,    regime==REGIME_FIND && ToolbarVisible);  
	ShowWindow(hwndCheckFindCaseSensitive, regime==REGIME_FIND && ToolbarVisible);  
	ShowWindow(hwndCheckFindCurrentBook,   regime==REGIME_FIND && ToolbarVisible);  
	ShowWindow(hwndCheckFindVerseGroups,   regime==REGIME_FIND && ToolbarVisible);  

	GetClientRect(hwndWindow,&rc);
	int ToolbarT, ToolbarH, ChH, ChW;
	int L = rc.left;
	int T = rc.top;
	int W = rc.right - rc.left;
	int H = rc.bottom - rc.top;
	int D = W>320?2:1;
	GetClientRect(hwndSelectBook,&rc1);
	ChH = rc1.bottom - rc1.top + D + D;
	ToolbarT = ChH + D;
	ToolbarH = 0;
	if(regime==REGIME_VIEW){
		ChW = (int)(W*0.18);
		MoveWindow(hwndButtonPlus, L, T, ChH, ChH, true);
		MoveWindow(hwndSelectBook, L+ChH+D, T, W-ChH-ChW-D-D, (int)(H*0.7), true);
		MoveWindow(hwndSelectChapter, L+W-ChW, T, ChW, (int)(H*0.7), true);
		if(ToolbarVisible)
		{
			GetClientRect(hwndSelectModule,&rc1);
			ToolbarH+= rc1.bottom - rc1.top + D + D + D;
			MoveWindow(hwndSelectModule, L, ToolbarT, W, (int)(H*0.7), true);
		}
	}else if(regime==REGIME_FIND){
		MoveWindow(hwndEditFind,       L+ChH+D, T, (int)(W-ChH-D-ChH-D-ChH-D-ChH*1.5-D), ChH, true);
		MoveWindow(hwndButtonFind,     L+W-ChH-ChH-D-(int)(ChH*1.5)-D, T, (int)(ChH*1.5), ChH, true);
		MoveWindow(hwndButtonFindPrev, L+W-ChH-ChH-D, T, ChH, ChH, true);
		MoveWindow(hwndButtonFindNext, L+W-ChH, T, ChH, ChH, true);
		if(ToolbarVisible)
		{
			GetClientRect(hwndSelectModule,&rc1);
			ToolbarH+= ChH + ChH + D + D + D;
			int M = (int)(W*0.1);
			int W2 = (int)(W*0.4);
			MoveWindow(hwndCheckFindWholeWords, L+M, ToolbarT, W2, ChH, true);
			MoveWindow(hwndCheckFindCaseSensitive, L+M, ToolbarT+ChH+D, W2, ChH, true);
			MoveWindow(hwndCheckFindCurrentBook, L+W2+M+M, ToolbarT, W2, ChH, true);
			MoveWindow(hwndCheckFindVerseGroups, L+W2+M+M, ToolbarT+ChH+D, W2, ChH, true);
		}
	}else if(regime==REGIME_STRONG){
		ToolbarH = 0;
		ToolbarT = 0;
	}
	MoveWindow(hwndEdit, L, T+ToolbarT+ToolbarH, W, H-ToolbarT-ToolbarH, true);
}
/*
HWND CreateResToolBar(HMODULE hExe, HWND hParent, UINT uID)
{
	HRSRC hRes;
	HRSRC hResLoad;

	hRes = FindResource(hExe, MAKEINTRESOURCE(uID), MAKEINTRESOURCE(241));
	hResLoad = (HRSRC) LoadResource(hExe, hRes);
	LPTOOLBARRES tbs = (LPTOOLBARRES)LockResource(hResLoad);

	TBBUTTON* tbb;
	tbb=(TBBUTTON*)malloc(sizeof(TBBUTTON)*tbs->num);

	int n=0;
	for(int i=0;i<tbs->num;i++)
	{
  tbb[i].fsState=TBSTATE_ENABLED;
  tbb[i].dwData=0;
  if(tbs->ids[i]==0){
  	tbb[i].iBitmap=-1;
  	tbb[i].idCommand=0;
  	tbb[i].fsStyle=TBSTYLE_SEP;
  	tbb[i].iString=0;
  }else{
  	tbb[i].iBitmap=n;
  	tbb[i].idCommand=tbs->ids[i];
  	tbb[i].fsStyle=TBSTYLE_BUTTON;
  	tbb[i].iString=tbs->ids[i];
  	n++;
  }
	}

	free(tbs);

	HWND htmp=CreateToolbarEx(
  hParent,
  WS_CHILD|WS_VISIBLE|WS_BORDER,
  uID,
  tbs->num,
  g_hInst,
  uID,
  tbb,
  tbs->num,
  tbs->width,
  tbs->height,
  tbs->width,
  tbs->height,
  sizeof(TBBUTTON));
	SetWindowLong(htmp,GWL_USERDATA,(LONG)tbb);
	return htmp;
}

BOOL DestroyResToolBar(HWND hCommandBar)
{
	TBBUTTON* tbb=(TBBUTTON*)GetWindowLong(hCommandBar,GWL_USERDATA);
	free(tbb);
	return DestroyWindow(hCommandBar);
}
*/
void showFound(int page){
	foundPage = page;
	int count = BQbook.getFoundQty();
	if(count==NULL)count = 0;
	int startNum = 100*page;
	int endNum = 100+startNum; if(endNum>count) endNum=count;
	CString text;
	if(count>100)text.Format(_T("Found %d verses (%d-%d listed):\r\n\r\n"), count, startNum+1, endNum);
	else text.Format(_T("Found %d verses:\r\n\r\n"), count);
	int i;
	for(i=startNum; i<count&&i<endNum; i++){
		CString Num;
		Num.Format(_T("%3d: "),(i+1));
		text+= Num+CString("[")+BQbook.getFoundTitle(i)+CString("]\r\n");
		text+= BQbook.getFoundContent(i)+CString("\r\n\r\n");
	}
	if(endNum<count) text+=CString("...");
	SetWindowTextW(hwndEdit, text.GetBuffer());
	EnableWindow(hwndButtonFindPrev, (page==0)?0:1);
	EnableWindow(hwndButtonFindNext, (endNum<count)?1:0);
}
void findVerses()
{   // FIND button pressed
	BQfindOptions findOptions;
	findOptions.WholeWords = SendMessage(hwndCheckFindWholeWords, BM_GETCHECK, 0, 0);
	findOptions.CaseSensitive = SendMessage(hwndCheckFindCaseSensitive, BM_GETCHECK, 0, 0);
	findOptions.VerseGroups = SendMessage(hwndCheckFindVerseGroups, BM_GETCHECK, 0, 0);
	if(SendMessage(hwndCheckFindCurrentBook, BM_GETCHECK, 0, 0)){
		findOptions.BookEnd = findOptions.BookStart = SendMessage(hwndSelectBook, CB_GETCURSEL, 0, 0)+1;
	}
    int cTxtLen = GetWindowTextLength(hwndEditFind); 
    LPWSTR pszMem = (LPWSTR) VirtualAlloc((LPVOID) NULL, (DWORD) (cTxtLen + 1), MEM_COMMIT, PAGE_READWRITE); 
    GetWindowText(hwndEditFind, pszMem, cTxtLen + 1); 
	CString what(pszMem);
	VirtualFree(pszMem, 0, MEM_RELEASE); 
	what.Trim();
	if(what.GetLength()>1){
		SetWindowTextW(hwndEdit, TEXT(""));
		SetCursor(LoadCursor(NULL, IDC_WAIT));
		BQbook.find(what, &findOptions);
		SetCursor(NULL);
		foundPage = 0;
		foundPageCount = (int)(ceil(BQbook.getFoundQty()/100.));
		showFound(0);
	}
}

int ShowChapter(int chapter)
{
	int i = SetWindowTextW(hwndEdit, BQbook.getChapter(chapter, StrongsVisible).Trim()+TEXT("\r\n___"));
	int ind;
	if(BQbook.getIni()->getIsBible())
		ind = BQbook.getAbsIndex();
	else
		ind = SendMessage(hwndSelectBook, CB_GETCURSEL, 0, 0) + 1;
	params.currentPosition.set(BQbook.getIni()->getCharShortName(), ind, chapter - BQbook.getIni()->getHasChapterZero(), 0);
	params.setChanged(true);
	return i;
}

int ShowStrong(CString*num)
{
	int Num;
	CString word;
	BQdictionary*dict;
	word = *num;

	if(regime==REGIME_VIEW){
		AtomScrollSavePosition();
		if(fullScreen)
			ShowWindow(g_hWndMenuBar, SW_SHOW);
			resizeWindow();
	}
	while(word.GetLength()<5) word = CString(TEXT("0")) + word;
	if(BQbook.getIni()->getHasStrongNumbers() && BQbibles.getStrongsDictionaryExists()){
		if(BQbook.getAbsIndex()<39){
			if(BQbibles.getStrongsDictionaryHebExists()){
				dict = BQbibles.getStrongsHeb();
				Num = dict->getPosition(&word);
				SetWindowTextW(hwndEdit, CString("HEBREW STRONG LEXICON:\r\n") + dict->get(Num));
				regime = REGIME_STRONG;
				SendMessage(hwndWindow, WM_SIZE, 0, 0);
			}
		}
		else{
			if(BQbibles.getStrongsDictionaryGrkExists()){
				dict = BQbibles.getStrongsGrk();
				Num = dict->getPosition(&word);
				SetWindowTextW(hwndEdit, CString("GREEK STRONG LEXICON:\r\n") + dict->get(Num));
				regime = REGIME_STRONG;
				SendMessage(hwndWindow, WM_SIZE, 0, 0);
			}
		}
	}
	return 0;
}










//
//
//
// THE WINDOW PROCEDURE HELPER FUNCTIONS:
//
//
int WinProcessHotKeys(WPARAM wParam, LPARAM lParam)
{
	switch(wParam){
		case IDK_DOWN:
			if(GetFocus()!=hwndEdit)
				SendMessage(GetFocus(), WM_KEYDOWN, VK_DOWN, lParam);
			else
				AtomScrollPageDown();
			break;
		case IDK_UP:
			if(GetFocus()!=hwndEdit)
				SendMessage(GetFocus(), WM_KEYDOWN, VK_DOWN, lParam);
			else
				AtomScrollPageUp();
			break;
		case IDK_LEFT:
			if(GetFocus()!=hwndEdit)
				SendMessage(GetFocus(), WM_KEYDOWN, VK_DOWN, lParam);
			else
				AtomScrollLineUp();
			break;
		case IDK_RIGHT:
			if(GetFocus()!=hwndEdit)
				SendMessage(GetFocus(), WM_KEYDOWN, VK_DOWN, lParam);
			else
				AtomScrollLineDown();
			break;
		case IDK_FULLSCREEN:
			AtomFulscreenToggle();
			break;
	}
	return 0;
}




int WinProcessModuleChange()
{
	int mesRes;
	int tmpAbsIndex = BQbook.getAbsIndex();
	mesRes  = SendMessage(hwndSelectChapter, CB_GETCURSEL, 0, 0);  
	chIndex = mesRes-BQbook.getIni()->getHasChapterZero();
	AtomScrollSavePosition();
	mesRes = SendMessage(hwndSelectModule, CB_GETCURSEL, 0, 0);  
	if(!BQbibles.getItemEnabled(mesRes)){
		mesRes = SendMessage(hwndSelectModule, CB_GETCURSEL, 0, 0);  
		mesRes = SendMessage(hwndSelectModule, CB_SETCURSEL, mesRes+1, 0);  
	}
	fillBooksCombo();
	int num = BQbook.getIni()->getBookIndexByAbsolute(absIndex);
	if(num==-1) {
		num = 0;
		chIndex = 0;
		tmpAbsIndex = -1;
	}
	mesRes = SendMessage(hwndSelectBook, CB_SETCURSEL, num, 0);  
	reloadChapters();
	if(chIndex+BQbook.getIni()->getHasChapterZero()>BQbook.getBookChapterQty(num+1))
		chIndex = 0;
	if(chIndex+BQbook.getIni()->getHasChapterZero()<0) chIndex=0;
	mesRes = SendMessage(hwndSelectChapter, CB_SETCURSEL, chIndex+BQbook.getIni()->getHasChapterZero(), 0);  
	ShowChapter(mesRes+1);
	absIndex = BQbook.getAbsIndex();
	toggleToolbar();
	if(tmpAbsIndex>=0 && absIndex>=0)
		AtomScrollRestorePosition();
	SetFocus(hwndEdit);
	return 0;
}
int WinProcessViewportCreate()
{
	hwndEditScroll = CreateWindow(
		TEXT ("edit"), NULL,
		WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_READONLY|WS_VSCROLL,
		0,0,0,0,
		hwndWindow, (HMENU)ID_EDIT,
		g_hInst, NULL
		);
	hwndEditNoscroll = CreateWindow(
		TEXT ("edit"), NULL,
		WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_READONLY,
		0,0,0,0,
		hwndWindow, (HMENU)ID_EDIT,
		g_hInst, NULL
		);
	hwndEdit = (params.getShowScrollBar()?hwndEditScroll:hwndEditNoscroll);
	HideCaret(hwndEdit);
	return 0;
}
int WinProcessViewportToggle()
{
	int len;
	wchar_t *str;
	bool scrollbar = fullScreen?params.getFSShowScrollBar():params.getShowScrollBar();
	HWND tmp = scrollbar?hwndEditScroll:hwndEditNoscroll;
	if(tmp!=hwndEdit){
		AtomScrollSavePosition();
		len = GetWindowTextLength(hwndEdit);
		str = new wchar_t[(len+1)*sizeof(wchar_t)];
		GetWindowText(hwndEdit, str, len);
		SetWindowText(hwndEdit, TEXT(""));
		ShowWindow(hwndEdit, SW_HIDE);
		ShowWindow(tmp, SW_SHOW);
		hwndEdit = tmp;
		setWindowFont(hwndEdit);
		SetWindowText(hwndEdit, str);
		delete str;
		AtomScrollRestorePosition();
	}
	SetFocus(hwndEdit);
	HideCaret(hwndEdit);
	return (int)hwndEdit;
}
HWND WinProcessCreateButton(HWND hWnd, int id, LPCTSTR caption)
{
	HWND hwnd = CreateWindowW(
		TEXT ("button"), NULL,
		WS_CHILD|WS_VISIBLE,
		0,0,20,20,
		hWnd, (HMENU)id,
		g_hInst, NULL
		);
	SetWindowText(hwnd, (LPCTSTR)caption);
	return hwnd;
}
HWND WinProcessCreateCheckBox(HWND hWnd, int id, LPCTSTR caption, bool check)
{
	HWND hwnd = CreateWindowW(
		TEXT ("button"), NULL,
		WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,
		0,0,20,20,
		hWnd, (HMENU)id,
		g_hInst, NULL
		);
	SetWindowText(hwnd, (LPCTSTR)caption);
	if(check)
		SendMessage(hwnd,BM_SETCHECK,check,0);
	return hwnd;
}
HWND WinProcessCreateComboBox(HWND hWnd, int id)
{
	return CreateWindow(
		TEXT ("combobox"), NULL,
		WS_CHILD|WS_VISIBLE|CBS_DISABLENOSCROLL|WS_VSCROLL|CBS_DROPDOWNLIST,
		0,0,100,100,
		hWnd, (HMENU)id,
		g_hInst, NULL
		);
}
HWND WinProcessCreateStatic(HWND hWnd, LPCTSTR caption)
{
	HWND hwnd = CreateWindow(TEXT("STATIC"), TEXT(""), 
		WS_CHILD | WS_VISIBLE, 
		10,10,100,100,
		hWnd, NULL, g_hInst, NULL);
	SetWindowText(hwnd, (LPCTSTR)caption);
	return hwnd;
}
//
//
//
// THE ATOM INTERFACE FUNCTIONS:
//
//
int AtomScrollPageDown()
{
	if(SendMessage(hwndEdit, EM_SCROLL, SB_PAGEDOWN, 0)==0){
		if(regime==REGIME_FIND){
			if(foundPage<foundPageCount-1) showFound(++foundPage);
		}
		else if(regime==REGIME_VIEW){
			AtomBibleNextChapter();
		}
	}
	else{
		return SendMessage(hwndEdit, EM_SCROLL, SB_LINEDOWN, 0);
	}
	return 0;
}
int AtomScrollPageUp()
{
	if(SendMessage(hwndEdit, EM_SCROLL, SB_PAGEUP, 0)==0){
		if(regime==REGIME_FIND){
			if(foundPage>0){
				showFound(--foundPage);
				return SendMessage(hwndEdit, EM_LINESCROLL, 0, 60000);
			}
		}
		else if(regime==REGIME_VIEW){
			if(AtomBiblePrevChapter())
				return SendMessage(hwndEdit, EM_LINESCROLL, 0, 60000);
		}
	}
	else{
		return SendMessage(hwndEdit, EM_SCROLL, SB_LINEUP, 0);
	}
	return 0;
}
int AtomScrollLineUp()
{
	return SendMessage(hwndEdit, EM_SCROLL, SB_LINEUP, 0);
}
int AtomScrollLineDown()
{
	return SendMessage(hwndEdit, EM_SCROLL, SB_LINEDOWN, 0);
}
int AtomScrollSavePosition()
{
	return editPosition = (SendMessage(hwndEdit,EM_GETFIRSTVISIBLELINE,0,0)*1000)/SendMessage(hwndEdit,EM_GETLINECOUNT,0,0);
}
int AtomScrollRestorePosition()
{
	return SendMessage(hwndEdit, EM_LINESCROLL, 0, (LPARAM)((int)((editPosition*SendMessage(hwndEdit,EM_GETLINECOUNT,0,0))/1000) - 1));
}
int AtomHotKeysRegister()
{
	//RegisterHotKey(hwndWindow, IDK_DOWN, 0, VK_DOWN);
	//RegisterHotKey(hwndWindow, IDK_UP, 0, VK_UP);
	//RegisterHotKey(hwndWindow, IDK_LEFT, 0, VK_LEFT);
	//RegisterHotKey(hwndWindow, IDK_RIGHT, 0, VK_RIGHT);
	//RegisterHotKey(hwndWindow, IDK_FULLSCREEN, 0, VK_RETURN);
	return 0;
}
int AtomHotKeysUnregister()
{
	//UnregisterHotKey(hwndWindow, IDK_DOWN);
	//UnregisterHotKey(hwndWindow, IDK_UP);
	//UnregisterHotKey(hwndWindow, IDK_LEFT);
	//UnregisterHotKey(hwndWindow, IDK_RIGHT);
	//UnregisterHotKey(hwndWindow, IDK_FULLSCREEN);
	return 0;
}

int AtomBibleNextChapter()
{
	int book    = SendMessage( hwndSelectBook, CB_GETCURSEL, 0, 0);
	int chapter = SendMessage( hwndSelectChapter, CB_GETCURSEL, 0, 0);
	int bookCount    = SendMessage( hwndSelectBook, CB_GETCOUNT, 0, 0);
	int chapterCount = SendMessage( hwndSelectChapter, CB_GETCOUNT, 0, 0);
	if(chapter<chapterCount-1){
		SendMessage( hwndSelectChapter, CB_SETCURSEL, ++chapter, 0);
		ShowChapter(chapter+1);
		return true;
	}
	else if(book<bookCount-1){
		SendMessage( hwndSelectBook, CB_SETCURSEL, ++book, 0);
		reloadChapters();
		SendMessage( hwndSelectChapter, CB_SETCURSEL, chapter=0, 0);
		ShowChapter(chapter+1);
		return true;
	}
	return false;
}
int AtomBiblePrevChapter()
{
	int book    = SendMessage( hwndSelectBook, CB_GETCURSEL, 0, 0);
	int chapter = SendMessage( hwndSelectChapter, CB_GETCURSEL, 0, 0);
	int chapterCount;
	if(chapter>0){
		SendMessage( hwndSelectChapter, CB_SETCURSEL, --chapter, 0);
		ShowChapter(chapter+1);
		return true;
	}
	else if(book>0){
		SendMessage( hwndSelectBook, CB_SETCURSEL, --book, 0);
		reloadChapters();
		chapterCount = SendMessage( hwndSelectChapter, CB_GETCOUNT, 0, 0);
		SendMessage( hwndSelectChapter, CB_SETCURSEL, chapter=chapterCount-1, 0);
		ShowChapter(chapter+1);
		return true;
	}
	return false;
}

int AtomBibleCurrentChapter()
{
	return ShowChapter(SendMessage( hwndSelectChapter, CB_GETCURSEL, 0, 0)+1);
}
int AtomToggleStrongs()
{
	int mesRes;
	if(regime!=REGIME_VIEW) return 0;
	if(!BQbook.getIni()->getHasStrongNumbers()) return 0;
	AtomScrollSavePosition();
	StrongsVisible = !StrongsVisible;
	mesRes = SendMessage(hwndSelectChapter, CB_GETCURSEL, 0, 0);  
	ShowChapter(mesRes+1);
	AtomScrollRestorePosition();
	SetFocus(hwndEdit);
	return StrongsVisible;
}
int AtomRegimeChange()
{
	int mesRes;
	if(regime==REGIME_VIEW){
		AtomScrollSavePosition();
	}
	regime = regime==REGIME_VIEW? REGIME_FIND : REGIME_VIEW;
	if(regime==REGIME_VIEW && fullScreen){
		fullScreenObject.FullScreenRedo();
	}
	ToolbarVisible = 0;
	resizeWindow();
	if(regime==REGIME_FIND){
		showFound(0);
		SetFocus(hwndEditFind);
	}
	else if(regime==REGIME_VIEW){
		mesRes = SendMessage(hwndSelectBook, CB_GETCURSEL, 0, 0);  
		BQbook.loadBook(mesRes+1);
		mesRes = SendMessage(hwndSelectChapter, CB_GETCURSEL, 0, 0);  
		ShowChapter(mesRes+1);
		AtomScrollRestorePosition();
		SetFocus(hwndEdit);
	}
	resizeWindow();
	return regime;
}

int AtomFulscreenToggle(){
	if(regime!=REGIME_FIND || fullScreen){
		if(fullScreen)
		{
			fullScreen = false;
			fullScreenObject.FullScreenOff();
		}
		else
		{
			fullScreen = true;
			fullScreenObject.FullScreenOn();
		}
		if(regime!=REGIME_VIEW) ShowWindow(g_hWndMenuBar, SW_SHOW);
		WinProcessViewportToggle();
		resizeWindow();
	}
	return 0;
}