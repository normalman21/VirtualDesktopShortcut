// VirtualDesktopShortcut.cpp : ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include "VirtualDesktopShortcut.h"

#define MAX_LOADSTRING 100
#define ICON_ID 1
#define WM_TRAY_NOTIFICATION WM_APP + 1

// ���� ����:
HINSTANCE hInst;                                // ���� �ν��Ͻ��Դϴ�.
WCHAR szTitle[MAX_LOADSTRING];                  // ���� ǥ���� �ؽ�Ʈ�Դϴ�.
WCHAR szWindowClass[MAX_LOADSTRING];            // �⺻ â Ŭ���� �̸��Դϴ�.

// �� �ڵ� ��⿡ ��� �ִ� �Լ��� ������ �����Դϴ�.
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

extern "C" __declspec(dllimport) void InstallHook(HWND hWnd);
extern "C" __declspec(dllimport) void UninstallHook();
typedef void(*INSTALL_HOOK)(HWND);
typedef void(*UNINSTALL_HOOK)(void);

TCHAR msg1[128] = { 0, };
TCHAR msg2[128] = { 0, };
HINSTANCE hinstDLL = nullptr;
INSTALL_HOOK installHook = nullptr;
UNINSTALL_HOOK uninstallHook = nullptr;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: ���⿡ �ڵ带 �Է��մϴ�.

    // ���� ���ڿ��� �ʱ�ȭ�մϴ�.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_VIRTUALDESKTOPSHORTCUT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // ���� ���α׷� �ʱ�ȭ�� �����մϴ�.
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_VIRTUALDESKTOPSHORTCUT));

    MSG msg;

    // �⺻ �޽��� �����Դϴ�.
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  �Լ�: MyRegisterClass()
//
//  ����: â Ŭ������ ����մϴ�.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VIRTUALDESKTOPSHORTCUT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_VIRTUALDESKTOPSHORTCUT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   �Լ�: InitInstance(HINSTANCE, int)
//
//   ����: �ν��Ͻ� �ڵ��� �����ϰ� �� â�� ����ϴ�.
//
//   ����:
//
//        �� �Լ��� ���� �ν��Ͻ� �ڵ��� ���� ������ �����ϰ�
//        �� ���α׷� â�� ���� ���� ǥ���մϴ�.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // �ν��Ͻ� �ڵ��� ���� ������ �����մϴ�.

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 450, 200, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ////////////////////////////////////////////////////////////////////////////
   /// Add tray icon.
   NOTIFYICONDATA nid;
   ZeroMemory(&nid, sizeof(nid));
   nid.cbSize = sizeof(nid);
   nid.uID = ICON_ID;
   nid.hWnd = hWnd;

   nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
   nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));
   lstrcpy(nid.szTip, szTitle);
   nid.uCallbackMessage = WM_TRAY_NOTIFICATION;

   BOOL bRet = ::Shell_NotifyIcon(NIM_ADD, &nid);
   ////////////////////////////////////////////////////////////////////////////

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  �Լ�: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ����:  �� â�� �޽����� ó���մϴ�.
//
//  WM_COMMAND  - ���� ���α׷� �޴��� ó���մϴ�.
//  WM_PAINT    - �� â�� �׸��ϴ�.
//  WM_DESTROY  - ���� �޽����� �Խ��ϰ� ��ȯ�մϴ�.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_CREATE:
		hinstDLL = LoadLibrary(_T("VirtualDesktopShortcutDll.dll"));
		installHook = (INSTALL_HOOK)GetProcAddress(hinstDLL, "InstallHook");
		uninstallHook = (UNINSTALL_HOOK)GetProcAddress(hinstDLL, "UninstallHook");

		if (hinstDLL && installHook && uninstallHook)
		{
			installHook(hWnd);

			_stprintf_s(msg1, 128, _T("Add a shortcut to switch desktop : Ctrl + Win + Mouse L/R button"));
			_stprintf_s(msg2, 128, _T("Please press the Minimize button."));
		}
		else
		{
			_stprintf_s(msg1, 128, _T("Error!"));
			_stprintf_s(msg2, 128, _T(""));
		}
		
		return 0;

	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED)
		{
			ShowWindow(hWnd, SW_HIDE); //������ ���߱�
		}
		break;

	case WM_TRAY_NOTIFICATION:
		if (wParam == ICON_ID)
		{
			switch (lParam)
			{
			case WM_LBUTTONUP:
				ShowWindow(hWnd, SW_SHOW);
				ShowWindow(hWnd, SW_SHOWNORMAL);
				break;
			}
		}
		break;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // �޴� ������ ���� �м��մϴ�.
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

			TextOut(hdc, 10, 10, msg1, lstrlen(msg1));
			TextOut(hdc, 10, 30, msg2, lstrlen(msg2));

            EndPaint(hWnd, &ps);
        }
        break;

    case WM_DESTROY:

		/// Delete tray icon.
		NOTIFYICONDATA    nid;
		ZeroMemory(&nid, sizeof nid);
		nid.cbSize = sizeof nid;
		nid.hWnd = hWnd;
		nid.uID = ICON_ID;
		Shell_NotifyIcon(NIM_DELETE, &nid);

		if (uninstallHook)
		{
			uninstallHook();
		}
		
		if (hinstDLL)
		{
			FreeLibrary(hinstDLL);
		}
		
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// ���� ��ȭ ������ �޽��� ó�����Դϴ�.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
