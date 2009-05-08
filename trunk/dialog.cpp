// dialog.cpp : Defines the entry point for the options dialog.
//

#include "stdafx.h"
#include "ppcbq.h"
#include "dialog.h"
#include <windows.h>
#include <commctrl.h>

#define ID_DLG_CLOSE                 16011
#define ID_CHECK_BOLD                16008
#define ID_CHECK_ITALIC              16009
#define ID_CHECK_SHOW_SCROLLBAR      16010
#define ID_DLG_TAB                   20000
#define ID_COMBO_FONT                17003
#define ID_COMBO_FONTSIZE            17004
#define ID_COLOR_RED                 17005
#define ID_COLOR_GREEN               17006
#define ID_COLOR_BLUE                17007
#define ID_FONT_FG                   17008
#define ID_FONT_BG                   17009
#define ID_FS_CHECK_SHOW_SCROLLBAR   17010
#define ID_FS_CHECK_USE_TAPS         17011

#define TAB_GENERAL    0
#define TAB_FONT       1
#define TAB_COLOR      2
#define TAB_ABOUT      3

#define FONT_SIZE_COUNT 20

HWND hwndTab;
HWND hwndDlgClose;
HWND hwndDlgContent;
HWND hwndDlgSelectFont;
HWND hwndDlgSelectFontSize;
HWND hwndDlgCheckBold;
HWND hwndDlgCheckItalic;
HWND hwndCheckShowScrollBar;
HWND hwndFSText;
HWND hwndFSCheckShowScrollBar;
HWND hwndFSUseTaps;
HWND hwndDlgSampleContainer;
HWND hwndDlgSample;
HWND hwndFontColorRed;
HWND hwndFontColorGreen;
HWND hwndFontColorBlue;
HWND hwndFontForeground;
HWND hwndFontBackground;

int FontColorRed, FontColorGreen, FontColorBlue;
int BgColorRed, BgColorGreen, BgColorBlue;
int ColorCtrLeft, ColorCtrRight, ColorCtrTop, ColorCtrHeight, ColorCtrStep;
bool FG=true;
bool resizeDialogInProcess = false;

int fontSize[]={12,13,14,16,18,20,22,24,26,28,32,36,40,44,48,52,56,60,64,68};

LRESULT CALLBACK DlgPreviewProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DlgColorControlProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

#ifndef WIN32_PLATFORM_WFSP
// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
	int mesRes; //message result
	RECT rcClient;
	wchar_t*c;
	int sel;
	HDC hdc;
    PAINTSTRUCT ps;

    wmId    = LOWORD(wParam); 
    wmEvent = HIWORD(wParam); 

    switch (message)
    {
        case WM_INITDIALOG:
#ifdef SHELL_AYGSHELL
            {
                // Create a Done button and size it.  
                SHINITDLGINFO shidi;
                shidi.dwMask = SHIDIM_FLAGS;
                shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_EMPTYMENU;
                shidi.hDlg = hDlg;
                SHInitDialog(&shidi);
            }
#endif // SHELL_AYGSHELL
			createDialogControls(hDlg);
            return (INT_PTR)TRUE;
//
        case WM_COMMAND:
#ifdef SHELL_AYGSHELL
			if(wmId==ID_DLG_CLOSE){
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
			}else if (wmId==IDOK)
#endif
            {
				params.setShowScrollBar(SendMessage(hwndCheckShowScrollBar,BM_GETCHECK,0,0));
				sel = SendMessage(hwndDlgSelectFont,CB_GETCURSEL,0,0);
				mesRes = SendMessage(hwndDlgSelectFont,CB_GETLBTEXTLEN,sel,0);
				c = new wchar_t[(mesRes+1)*sizeof(wchar_t)];
				SendMessage(hwndDlgSelectFont,CB_GETLBTEXT,sel,(LPARAM)c);
				params.setTextFontFace(c);
				delete c;
				params.setTextFontHeight(fontSize[SendMessage(hwndDlgSelectFontSize,CB_GETCURSEL,0,0)]);
				params.setTextFontWeight(SendMessage(hwndDlgCheckBold,BM_GETCHECK,0,0)?900:400);
				params.setTextFontItalic(SendMessage(hwndDlgCheckItalic,BM_GETCHECK,0,0));
				params.setColorFontRed(FontColorRed);
				params.setColorFontGreen(FontColorGreen);
				params.setColorFontBlue(FontColorBlue);
				params.setColorBgRed(BgColorRed);
				params.setColorBgGreen(BgColorGreen);
				params.setColorBgBlue(BgColorBlue);
				params.setFSShowScrollBar(SendMessage(hwndFSCheckShowScrollBar,BM_GETCHECK,0,0));
				params.setFSUseTaps(SendMessage(hwndFSUseTaps,BM_GETCHECK,0,0));

                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
			}
            // Parse the menu selections:
            switch (wmId)
            {
				case ID_COMBO_FONT:
				case ID_COMBO_FONTSIZE:
				case ID_CHECK_BOLD:
				case ID_CHECK_ITALIC:
					sel = SendMessage(hwndDlgSelectFont,CB_GETCURSEL,0,0);
					mesRes = SendMessage(hwndDlgSelectFont,CB_GETLBTEXTLEN,sel,0);
					c = new wchar_t[(mesRes+1)*sizeof(wchar_t)];
					SendMessage(hwndDlgSelectFont,CB_GETLBTEXT,sel,(LPARAM)c);
					font = CreateFont(
						fontSize[SendMessage(hwndDlgSelectFontSize,CB_GETCURSEL,0,0)],// height of font
						params.getTextFontWidth(),            // average character width
						params.getTextFontEscapement(),       // angle of escapement
						params.getTextFontOrientation(),      // base-line orientation angle
						(SendMessage(hwndDlgCheckBold,BM_GETCHECK,0,0)?700:400),// font weight
						SendMessage(hwndDlgCheckItalic,BM_GETCHECK,0,0),// italic attribute option
						params.getTextFontUnderline(),        // underline attribute option
						params.getTextFontStrikeOut(),        // strikeout attribute option
						params.getTextFontCharSet(),          // character set identifier
						params.getTextFontOutputPrecision(),  // output precision
						params.getTextFontClipPrecision(),    // clipping precision
						params.getTextFontQuality(),          // output quality
						params.getTextFontPitchAndFamily(),   // pitch and family
						c              // typeface name
					);
					delete c;
					SendMessage( hwndDlgSample, WM_SETFONT, (WPARAM) font, true);  
					SendMessage( hwndDlgSample, WM_PAINT, 0, 0);  
					break;
				case ID_FONT_FG:
				case ID_FONT_BG:
					FG = (wmId==ID_FONT_FG);
					ShowWindow(hwndFontColorRed, SW_HIDE);
					ShowWindow(hwndFontColorRed, SW_SHOW);
					ShowWindow(hwndFontColorGreen, SW_HIDE);
					ShowWindow(hwndFontColorGreen, SW_SHOW);
					ShowWindow(hwndFontColorBlue, SW_HIDE);
					ShowWindow(hwndFontColorBlue, SW_SHOW);
					break;
				default:
					return DefWindowProc(hDlg, message, wParam, lParam);
			}
			break;

        case WM_CLOSE:
            EndDialog(hDlg, message);
            return (INT_PTR)TRUE;
			
#ifdef _DEVICE_RESOLUTION_AWARE
        case WM_SIZE:
			if(!resizeDialogInProcess)
            {
				resizeDialogInProcess = true;
				DRA::RelayoutDialog(
					g_hInst, 
					hDlg, 
					MAKEINTRESOURCE(IDD_ABOUTBOX));
				GetClientRect(hDlg,&rcClient);
				resizeDialog(rcClient);
				resizeDialogInProcess = false;
			}
            break;
#endif
        case WM_NOTIFY:
			switch (((LPNMHDR)lParam)->code)
            {
                case TCN_SELCHANGE:
					DialogShowControls();
					break;
			}
        case WM_PAINT:
            hdc = BeginPaint(hDlg, &ps);

			UpdateWindow(hwndFontColorRed);
            EndPaint(hDlg, &ps);
            break;

    }
    return (INT_PTR)FALSE;
}
#endif // !WIN32_PLATFORM_WFSP

void createDialogControls(HWND hDlg){
	TCITEM tie;
	int mesRes; //message result
	CString str;
	HDC hdc;
	LOGFONT  lf;


	hwndDlgClose = CreateWindowW(
		TEXT ("button"), NULL,
		WS_CHILD|WS_VISIBLE,
		0,0,20,20,
		hDlg, (HMENU)ID_DLG_CLOSE,
		g_hInst, NULL
		);
	SetWindowText(hwndDlgClose, (LPCTSTR)TEXT("x") );

	/*GENERAL:*/
	
	hwndCheckShowScrollBar = WinProcessCreateCheckBox(hDlg, ID_CHECK_SHOW_SCROLLBAR, (LPCTSTR)TEXT("Show scroll bar"), params.getShowScrollBar());
	
	/*FULLSCREEN:*/
	hwndFSText = CreateWindow(TEXT("STATIC"), TEXT(""), 
		WS_CHILD | WS_VISIBLE, 
		10,10,100,100,
		hDlg, NULL, g_hInst, NULL); 
	SetWindowText(hwndFSText,TEXT("Fullscreen:"));

	hwndFSCheckShowScrollBar = WinProcessCreateCheckBox(hDlg, ID_FS_CHECK_SHOW_SCROLLBAR, (LPCTSTR)TEXT("Show scroll bar"), params.getFSShowScrollBar());
	hwndFSUseTaps = WinProcessCreateCheckBox(hDlg, ID_FS_CHECK_USE_TAPS, (LPCTSTR)TEXT("Use taps"), params.getFSUseTaps());

	/*FONT:*/
	
	hwndDlgSelectFont = WinProcessCreateComboBox(hDlg, ID_COMBO_FONT);
	hdc = GetDC(hDlg);		// связанный DC
	lf.lfFaceName[0] = '\0';
	lf.lfCharSet = DEFAULT_CHARSET;	// все шрифты
	EnumFontFamilies(hdc,NULL,(FONTENUMPROC)EnumFontFamExProc,
		(LPARAM) 0);
	ReleaseDC(hDlg,hdc);

	hwndDlgSelectFontSize = WinProcessCreateComboBox(hDlg, ID_COMBO_FONTSIZE);
	for(int i=0; i<FONT_SIZE_COUNT; i++){
		str.Format(_T("%d"), fontSize[i]);
		mesRes = SendMessage(hwndDlgSelectFontSize, CB_ADDSTRING, 0, (LPARAM)str.GetBuffer());  
		if(i==0||fontSize[i]==params.getTextFontHeight()){
			SendMessage(hwndDlgSelectFontSize, CB_SETCURSEL, i, 0);
		}
	}
	hwndDlgCheckBold = WinProcessCreateCheckBox(hDlg, ID_CHECK_BOLD, (LPCTSTR)TEXT("Bold"),params.getTextFontWeight()>400?true:false);
	hwndDlgCheckItalic = WinProcessCreateCheckBox(hDlg, ID_CHECK_ITALIC, (LPCTSTR)TEXT("Italic"),params.getTextFontItalic());

	hwndDlgSampleContainer = CreateWindow(TEXT("PREVIEW"), TEXT(""), 
		WS_CHILD | WS_VISIBLE | WS_BORDER, 
		10,10,100,100,
		hDlg, NULL, g_hInst, NULL); 

	hwndDlgSample = WinProcessCreateStatic(hwndDlgSampleContainer, TEXT("Pocket Bible Quote"));
	setWindowFont(hwndDlgSample);

	/*COLORS:*/
	FontColorRed   = params.getColorFontRed();
	FontColorGreen = params.getColorFontGreen();
	FontColorBlue  = params.getColorFontBlue();
	BgColorRed     = params.getColorBgRed();
	BgColorGreen   = params.getColorBgGreen();
	BgColorBlue    = params.getColorBgBlue();

	hwndFontForeground = CreateWindow(
		TEXT ("button"), NULL,
		WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON,
		10,10,100,100,
		hDlg, (HMENU)ID_FONT_FG, g_hInst, NULL); 
	SetWindowText(hwndFontForeground,TEXT("Foreground"));

	FG = true;
	CheckRadioButton(hDlg, ID_FONT_FG, ID_FONT_BG, ID_FONT_FG);

	hwndFontBackground = CreateWindow(
		TEXT ("button"), NULL,
		WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON,
		10,10,100,100,
		hDlg, (HMENU)ID_FONT_BG, g_hInst, NULL); 
	SetWindowText(hwndFontBackground,TEXT("Background"));

	hwndFontColorRed = CreateWindow(TEXT("COLORSLIDER"), TEXT(""), 
		WS_CHILD | WS_VISIBLE | WS_BORDER, 
		10,10,200,200,
		hDlg, NULL, g_hInst, NULL); 

	hwndFontColorGreen = CreateWindow(TEXT("COLORSLIDER"), TEXT(""), 
		WS_CHILD | WS_VISIBLE | WS_BORDER, 
		10,10,200,200,
		hDlg, NULL, g_hInst, NULL); 

	hwndFontColorBlue = CreateWindow(TEXT("COLORSLIDER"), TEXT(""), 
		WS_CHILD | WS_VISIBLE | WS_BORDER, 
		10,10,200,200,
		hDlg, NULL, g_hInst, NULL); 

	/*ABOUT:*/
	
	hwndDlgContent = WinProcessCreateStatic(hDlg, TEXT("Pocket Bible Quote\nVersion 0.5.5\nfor WM2003 and higher, QVGA/VGA\n\nFreeware\nAuthor: Oleksandr Zayikin aka Nuh\n\nhttp://ppc-bq.narod.ru/"));

	/*TAB CONTROL:*/
	
	hwndTab = CreateWindow(WC_TABCONTROL, TEXT(""), 
		WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, 
		10, 10, 100, 100, 
		hDlg, (HMENU)ID_DLG_TAB, g_hInst, NULL);
	tie.mask = TCIF_TEXT | TCIF_IMAGE; 
	tie.iImage = -1; 
	tie.pszText = TEXT("General"); 
	TabCtrl_InsertItem(hwndTab, TAB_GENERAL, &tie); 
	tie.pszText = TEXT("Font"); 
	TabCtrl_InsertItem(hwndTab, TAB_FONT, &tie); 
	tie.pszText = TEXT("Color"); 
	TabCtrl_InsertItem(hwndTab, TAB_COLOR, &tie); 
	tie.pszText = TEXT("About"); 
	TabCtrl_InsertItem(hwndTab, TAB_ABOUT, &tie); 
	DialogShowControls();
}

void DialogShowControls(){
	int tab = TabCtrl_GetCurSel(hwndTab);
	ShowWindow(hwndCheckShowScrollBar, tab==TAB_GENERAL);
	ShowWindow(hwndFSText, tab==TAB_GENERAL);
	ShowWindow(hwndFSCheckShowScrollBar,tab==TAB_GENERAL);
	ShowWindow(hwndFSUseTaps,          tab==TAB_GENERAL);
	ShowWindow(hwndDlgSelectFont,      tab==TAB_FONT);
	ShowWindow(hwndDlgSelectFontSize,  tab==TAB_FONT);
	ShowWindow(hwndDlgCheckBold,       tab==TAB_FONT);
	ShowWindow(hwndDlgCheckItalic,     tab==TAB_FONT);
	ShowWindow(hwndDlgSampleContainer, tab==TAB_FONT || tab==TAB_COLOR);
	ShowWindow(hwndFontForeground,     tab==TAB_COLOR);
	ShowWindow(hwndFontBackground,     tab==TAB_COLOR);
	ShowWindow(hwndFontColorRed,       tab==TAB_COLOR);
	ShowWindow(hwndFontColorGreen,     tab==TAB_COLOR);
	ShowWindow(hwndFontColorBlue,      tab==TAB_COLOR);
	ShowWindow(hwndDlgContent,         tab==TAB_ABOUT);
}

int CALLBACK EnumFontFamExProc(ENUMLOGFONT *lpelfe,
	NEWTEXTMETRIC *lpntme,DWORD FontType,LPARAM lParam)
{
	int i = SendMessage(hwndDlgSelectFont, CB_GETCOUNT, 0, 0);
	SendMessage(hwndDlgSelectFont, CB_ADDSTRING, 0, (LPARAM)lpelfe->elfLogFont.lfFaceName);
	if(i==0||params.getTextFontFace().Compare(CString(lpelfe->elfLogFont.lfFaceName))==0){
		SendMessage(hwndDlgSelectFont, CB_SETCURSEL, i, 0);
	}
	return true;
}
void resizeDialog(RECT rcClient){
	RECT prc;
	RECT rc1;
	int margin;
	int w1;
	int t1;
	SetWindowPos(hwndTab, NULL, rcClient.left, rcClient.top, 
				rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, 
				SWP_NOZORDER);
	GetClientRect(hwndTab, &prc);
	TabCtrl_AdjustRect(hwndTab,false,&prc);
	margin = (prc.right-prc.left+prc.bottom-prc.top)/70;
	w1 = (int)((prc.right-prc.left)*0.7);
	t1 = prc.top+margin;
	GetClientRect(hwndDlgSelectFont,&rc1);
	int m1 = (int)(prc.top*0.1);
	int h1 = rc1.bottom-rc1.top;
	MoveWindow(hwndDlgClose,prc.right-prc.top+m1+m1+m1,m1,prc.top-m1-m1,prc.top-m1-m1-m1,true);
	MoveWindow(hwndCheckShowScrollBar,prc.left+margin+margin,t1,w1,rc1.bottom - rc1.top,true);
	MoveWindow(hwndFSText,prc.left+margin,t1+margin+h1+m1,w1,rc1.bottom - rc1.top,true);
	MoveWindow(hwndFSCheckShowScrollBar,prc.left+margin+margin,t1+2*(margin+h1)+m1,w1,rc1.bottom - rc1.top,true);
	MoveWindow(hwndFSUseTaps,prc.left+margin+margin,t1+3*(margin+h1)+m1,w1,rc1.bottom - rc1.top,true);
	MoveWindow(hwndDlgSelectFont,prc.left+margin,t1,w1,(int)(0.7*(prc.bottom-prc.top)),true);
	MoveWindow(hwndDlgSelectFontSize,prc.left+margin+margin+w1,t1,prc.right-prc.left-w1-margin-margin-margin,(int)(0.7*(prc.bottom-prc.top)),true);
	t1+= rc1.bottom - rc1.top;
	t1+= margin;
	MoveWindow(hwndDlgCheckBold,prc.left+margin+w1/4,t1+margin,w1/3,rc1.bottom - rc1.top,true);
	MoveWindow(hwndDlgCheckItalic,prc.left+margin+w1/4*3,t1+margin,w1/3,rc1.bottom - rc1.top,true);
	MoveWindow(hwndDlgContent,prc.left+margin, prc.top+margin, prc.right-prc.left-margin-margin, prc.bottom-prc.top-margin-margin,true);
	MoveWindow(hwndDlgSampleContainer,prc.left+margin, prc.top+margin+(prc.bottom-prc.top)/2, prc.right-prc.left-margin-margin, (prc.bottom-prc.top)/2-margin-margin,true);
	MoveWindow(hwndDlgSample, 0, 0, prc.right-margin, (prc.bottom+prc.top)/2-margin,true);
	
	t1 = prc.top+margin;
	MoveWindow(hwndFontForeground, prc.left+margin, t1, (prc.right-prc.left)/2-margin-margin, margin*3,true);
	MoveWindow(hwndFontBackground, (prc.left+prc.right)/2+margin, t1, (prc.right-prc.left)/2-margin-margin, margin*3,true);
	t1+= rc1.bottom - rc1.top + margin;
	MoveWindow(hwndFontColorRed,prc.left+margin, t1, prc.right-prc.left-margin-margin, margin*2,true);
	t1+= rc1.bottom - rc1.top;
	MoveWindow(hwndFontColorGreen,prc.left+margin, t1, prc.right-prc.left-margin-margin, margin*2,true);
	t1+= rc1.bottom - rc1.top;
	MoveWindow(hwndFontColorBlue,prc.left+margin, t1, prc.right-prc.left-margin-margin, margin*2,true);
}

LRESULT CALLBACK DlgPreviewProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HBRUSH   hbr  = NULL;
	COLORREF cr;
	COLORREF crbg;
	BOOL     mode = TRANSPARENT;
	RECT rc;
    switch (message) 
	{
		case WM_CTLCOLORSTATIC:
			SetBkMode((HDC)wParam, mode);
			cr = RGB(FontColorRed, FontColorGreen, FontColorBlue); // текст
			SetTextColor((HDC)wParam, cr);
			crbg = RGB(BgColorRed, BgColorGreen, BgColorBlue); // текст
			SetBkColor((HDC)wParam, crbg);
			hbr = CreateSolidBrush(crbg);
			return (LRESULT)hbr;
		case WM_ERASEBKGND:
			GetClientRect((HWND)lParam, &rc );
			FillRect((HDC)wParam, &rc, CreateSolidBrush(RGB(BgColorRed,BgColorGreen,BgColorBlue)));
			return 1;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK DlgColorControlProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HBRUSH   hbr  = NULL;
	BOOL     mode = TRANSPARENT;
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rcClient, rc;
	int msr, Color;
	switch (message) 
	{
		case WM_LBUTTONDOWN:
		case WM_MOUSEMOVE:
			GetClientRect(hWnd,&rcClient);
			msr = (rcClient.right - rcClient.left > 320) ? 6 : 3;
			if(LOWORD(lParam)<rcClient.left+2*msr+msr) Color = 0;
			else if(LOWORD(lParam)>=rcClient.right-msr-msr-msr) Color = 255;
			else Color = (255 * (LOWORD(lParam)-3*msr - rcClient.left+msr))/(rcClient.right - rcClient.left - 5*msr);
			if(hWnd==hwndFontColorRed && FG)
				FontColorRed = Color;
			else if(hWnd==hwndFontColorGreen && FG)
				FontColorGreen = Color;
			else if(hWnd==hwndFontColorBlue && FG)
				FontColorBlue = Color;
			else if(hWnd==hwndFontColorRed && !FG)
				BgColorRed = Color;
			else if(hWnd==hwndFontColorGreen && !FG)
				BgColorGreen = Color;
			else if(hWnd==hwndFontColorBlue && !FG)
				BgColorBlue = Color;
			InvalidateRect(hWnd, (LPCRECT)&rcClient, true);
			ShowWindow(hwndDlgSample, SW_HIDE);
			ShowWindow(hwndDlgSample, SW_SHOW);
			break;
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			if(hdc){
				GetClientRect(hWnd, &rc);
				if(hWnd==hwndFontColorRed)
					PaintColorScroll(hdc, hWnd, FG?FontColorRed:BgColorRed, 255, 0, 0);
				else if(hWnd==hwndFontColorGreen)
					PaintColorScroll(hdc, hWnd, FG?FontColorGreen:BgColorGreen, 0, 255, 0);
				else if(hWnd==hwndFontColorBlue)
					PaintColorScroll(hdc, hWnd, FG?FontColorBlue:BgColorBlue, 0, 0, 255);
				EndPaint(hWnd, &ps);
			}
			return 1;
			break;
       default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int PaintColorScroll(HDC hdc, HWND hWnd, int Pos, int R, int G, int B)
{
	int msr;
	int x1,x2,x3,x4,x5;
	RECT rc, rc2;
	GetClientRect(hWnd,&rc);
	msr = (rc.right - rc.left > 320) ? 6 : 3;
	int posX = (Pos*(rc.right - rc.left - msr - msr-msr - msr-msr)) / 255 + msr+msr;
	
	rc2.left   = rc.left+msr+msr;
	rc2.right  = rc.right-msr-msr;
	rc2.top    = rc.top;
	rc2.bottom = rc.top + (rc.bottom - rc.top)/3;
	FillRect(hdc, &rc2, CreateSolidBrush(RGB(255,255,255)));
	rc2.top    = rc.bottom - (rc.bottom - rc.top)/3;
	rc2.bottom = rc.bottom;
	FillRect(hdc, &rc2, CreateSolidBrush(RGB(255,255,255)));

	x1 = rc2.left;
	x2 = x1+(rc2.right-rc2.left)/4;
	x3 = x2+(rc2.right-rc2.left)/4;
	x4 = x3+(rc2.right-rc2.left)/4;
	x5 = rc2.right;
	rc2.top    = rc.top + (rc.bottom - rc.top)/3;
	rc2.bottom = rc.bottom - (rc.bottom - rc.top)/3;
	rc2.left  = x1;
	rc2.right = x2;
	FillRect(hdc, &rc2, CreateSolidBrush(RGB(7*R/10,7*G/10,7*B/10)));
	rc2.left  = x2;
	rc2.right = x3;
	FillRect(hdc, &rc2, CreateSolidBrush(RGB(8*R/10,8*G/10,8*B/10)));
	rc2.left  = x3;
	rc2.right = x4;
	FillRect(hdc, &rc2, CreateSolidBrush(RGB(9*R/10,9*G/10,9*B/10)));
	rc2.left  = x4;
	rc2.right = x5;
	FillRect(hdc, &rc2, CreateSolidBrush(RGB(R,G,B)));
	rc2.top    = rc.top;
	rc2.bottom = rc.bottom;
	rc2.left   = posX;
	rc2.right  = posX + msr;
	FillRect(hdc, &rc2, CreateSolidBrush(RGB(80,80,80)));
	return 0;
}