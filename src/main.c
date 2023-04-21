#include "framework.h"
#include "resource.h"


HINSTANCE g_hInst;
HWND hList;


BOOL CALLBACK EnumWindowsProc( HWND hWnd, LPARAM showHidden )
{
	char szBuffer[256];
	char *szName;
	GetWindowText(hWnd, szBuffer, sizeof(szBuffer));

	if ( showHidden || (IsWindowVisible(hWnd) && *szBuffer) )
	{
		if ( !*szBuffer )
			sprintf(szBuffer, "Wnd<%p>", hWnd);

		int idx = ListBox_AddString(hList, szBuffer);
		ListBox_SetItemData(hList, idx, hWnd);
	}

	return TRUE;
}

void EnumWnd( BOOL showHidden )
{
	ListBox_ResetContent(hList);
	EnumDesktopWindows(NULL, EnumWindowsProc, (LPARAM)showHidden);
}

BOOL CALLBACK MakeRTL_EnumChild( HWND hWnd, LPARAM lParam )
{
	long exStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
	exStyle &= ~(WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);
	exStyle |= WS_EX_RIGHT | WS_EX_LAYOUTRTL | WS_EX_LEFTSCROLLBAR;
	SetWindowLong(hWnd, GWL_EXSTYLE, exStyle);

	return TRUE;
}

void FuckItUp( HWND hWnd )
{
	MakeRTL_EnumChild(hWnd, 0);
	EnumChildWindows(hWnd, MakeRTL_EnumChild, 0);

	unsigned long swpFlags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER
		| SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_FRAMECHANGED;
	SetWindowPos(hWnd, NULL, 0, 0, 0, 0, swpFlags | SWP_HIDEWINDOW);
	SetWindowPos(hWnd, NULL, 0, 0, 0, 0, swpFlags | SWP_SHOWWINDOW);

	RedrawWindow(hWnd, NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW);
}

BOOL CALLBACK MainDialogProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static HWND hHidden;

	switch ( uMsg )
	{
		case WM_INITDIALOG:
		{
			HICON hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_APP));
			SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
			SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

			hList = GetDlgItem(hWnd, IDC_WNDLIST);
			hHidden = GetDlgItem(hWnd, IDC_HIDDEN);

			Button_SetCheck(hHidden, FALSE);

			return TRUE;
		}

		case WM_SHOWWINDOW:
			if ( wParam == TRUE && lParam == 0 )
			{
				EnumWnd(FALSE);
				SetFocus(hList);
				ListBox_SetCurSel(hList, 0);
			}
			return TRUE;


		case WM_COMMAND:
			switch ( LOWORD(wParam) )
			{
				case IDC_REFRESH:
				case IDC_HIDDEN:
					EnumWnd(Button_GetCheck(hHidden));
					ListBox_SetCurSel(hList, 0);
					return TRUE;

				case IDC_FUCKITUP:
				{
					int idx = ListBox_GetCurSel(hList);
					if ( idx != LB_ERR )
					{
						HWND hToRtl = (HWND)ListBox_GetItemData(hList, idx);
						FuckItUp(hToRtl);
					}
					return TRUE;
				}

				case IDOK:
				case IDCANCEL:
					SendMessage(hWnd, WM_CLOSE, 0, 0);
					return TRUE;
			}
			break;

		case WM_CLOSE:
			DestroyWindow(hWnd);
			return TRUE;

		case WM_DESTROY:
			PostQuitMessage(0);
			return TRUE;
	}

	return FALSE;
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow )
{
	HWND hDlg;
	MSG msg;

	g_hInst = hInstance;

	hDlg = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDialogProc);

	if ( hDlg == NULL )
		return 1;

	ShowWindow(hDlg, nCmdShow);

	while ( GetMessage(&msg, NULL, 0, 0) > 0 )
	{
		if ( !IsDialogMessage(hDlg, &msg) )
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return 0;
}
