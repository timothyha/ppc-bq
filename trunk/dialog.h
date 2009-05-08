//Dialog:
extern HWND hwndTab;
extern HWND hwndDlgClose;
extern HWND hwndDlgContent;
extern HWND hwndDlgSelectFont;
extern HWND hwndDlgSelectFontSize;
extern HWND hwndDlgCheckBold;
extern HWND hwndDlgCheckItalic;
extern HWND hwndCheckShowScrollBar;

void DialogShowControls();
void createDialogControls(HWND hDlg);
void resizeDialog(RECT rcClient);
int PaintColorScroll(HDC hdc, HWND hWnd, int Pos, int R, int G, int B);
LRESULT CALLBACK DlgPreviewProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
int PaintColorScroll(HDC hdc, HWND hWnd, int Pos, int R, int G, int B);
LRESULT CALLBACK DlgColorControlProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);