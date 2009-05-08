#include "FullScreen.h"

extern HWND g_hWndMenuBar;

int FullScreen::InitFullScreen (HWND window)
{
    hWndInputPanel = FindWindow(TEXT("SipWndClass"), NULL);
    hWndSipButton = FindWindow(TEXT("MS_SIPBUTTON"), NULL);
    hWndTaskBar = FindWindow(TEXT("HHTaskBar"), NULL);
	hWnd = window;

	return 0;
}

int FullScreen::ToggleFullScreen()
{
    if (mode)
		FullScreenOff();
    else
		FullScreenOn();
	return mode;
}
int FullScreen::FullScreenOff()
{
    RECT rtMenu, rtDesktop;
    if(hWndTaskBar != NULL)        
	ShowWindow(g_hWndMenuBar, SW_SHOW);
	GetWindowRect(g_hWndMenuBar, &rtMenu);
    ShowWindow(hWndTaskBar, SW_SHOW);
    //if (hWndInputPanel != NULL) ShowWindow(hWndInputPanel, SW_HIDE); //Never forcibly show the input panel
    //if(hWndSipButton != NULL) ShowWindow(hWndSipButton, SW_SHOW);
    if(SystemParametersInfo(SPI_GETWORKAREA, 0, &rtDesktop, NULL) == 1)
		SetWindowPos(hWnd,HWND_NOTOPMOST,rtDesktop.left,rtDesktop.top,rtDesktop.right - 
		rtDesktop.left,rtDesktop.bottom - rtDesktop.top + rtMenu.top - rtMenu.bottom, SWP_SHOWWINDOW);
	//MoveWindow
    if(hWndSipButton != NULL) ShowWindow(hWndSipButton, SW_SHOW);
    return mode = false;
}
int FullScreen::FullScreenOn()
{
    if (hWndTaskBar != NULL)    ShowWindow(hWndTaskBar, SW_HIDE);
    if (hWndInputPanel != NULL)    ShowWindow(hWndInputPanel, SW_HIDE);
    if (hWndSipButton != NULL)    ShowWindow(hWndSipButton, SW_HIDE);
    SetWindowPos(hWnd,HWND_NOTOPMOST,0,0,GetSystemMetrics(SM_CXSCREEN),
            GetSystemMetrics(SM_CYSCREEN), SWP_SHOWWINDOW);
	ShowWindow(g_hWndMenuBar, SW_HIDE);
    return mode = true;
}
int FullScreen::FullScreenBlur()
{
    if (hWndTaskBar != NULL) ShowWindow(hWndTaskBar, SW_SHOW);
	return 0;
}
int FullScreen::FullScreenFocus()
{
    if (hWndTaskBar != NULL) ShowWindow(hWndTaskBar, SW_HIDE);
	return 0;
}
int FullScreen::FullScreenRedo()
{
    if (hWndSipButton != NULL) ShowWindow(hWndSipButton, SW_HIDE);
	if (g_hWndMenuBar != NULL) ShowWindow(g_hWndMenuBar, SW_HIDE);
	return 0;
}
