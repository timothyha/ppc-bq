#include <windows.h>

class FullScreen{
	HWND	hWnd;
	HWND	hWndInputPanel;
	HWND	hWndTaskBar;
	HWND	hWndSipButton;
	bool	mode;
public:
	FullScreen(){hWndInputPanel = NULL; hWndTaskBar = NULL; hWndSipButton = NULL; mode = false;};
	int InitFullScreen	(HWND window);
	int ToggleFullScreen(void);
	int FullScreenOn();
	int FullScreenOff();
	int FullScreenBlur();
	int FullScreenFocus();
	int FullScreenRedo();
};