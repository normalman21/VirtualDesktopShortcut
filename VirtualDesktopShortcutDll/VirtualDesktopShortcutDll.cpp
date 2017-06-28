// VirtualDesktopShortcutDll.cpp : DLL ���� ���α׷��� ���� ������ �Լ��� �����մϴ�.
//

#include "stdafx.h"


HINSTANCE gModule = nullptr;
HHOOK hKeyHook = nullptr;
HHOOK hMouseHook = nullptr;

bool g_bLeftButtonClick = false;
bool g_bRightButtonClick = false;

bool g_bLeftWinKeyDown = false;
bool g_bLeftCtrlKeyDown = false;

/// ��Ű�� �ߵ��� downŰ ���� upŰ�� ���� �̺�Ʈ�� �ٸ� �ڵ鿡�� ������ �ʵ��� �ϴ� ����.
bool lockInput = false;


void SwitchVirtualDesktop(bool bLeft)
{
	HWND hWorker = 0;
	WPARAM wParam = (WPARAM)0x28;
	LPARAM lParam = MAKELPARAM(MOD_WIN | MOD_CONTROL, VK_RIGHT);
	if (bLeft)
	{
		wParam = (WPARAM)0x27;
		lParam = MAKELPARAM(MOD_WIN | MOD_CONTROL, VK_LEFT);
	}

	do
	{
		hWorker = FindWindowEx(NULL, hWorker, L"WorkerW", NULL);
		if (hWorker)
		{
			PostMessage(hWorker, WM_HOTKEY, wParam, lParam);
		}

	} while (hWorker);
}

bool isAllKeyDown()
{
	if (g_bLeftWinKeyDown && g_bLeftCtrlKeyDown)
	{
		if (g_bLeftButtonClick)
		{
			g_bLeftButtonClick = false;
			SwitchVirtualDesktop(true);
		}
		else if (g_bRightButtonClick)
		{
			g_bRightButtonClick = false;
			SwitchVirtualDesktop(false);
		}

		return true;
	}

	return false;
}

LRESULT CALLBACK KeyHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		KBDLLHOOKSTRUCT* kb = (KBDLLHOOKSTRUCT*)lParam;

		if (kb->vkCode == VK_LWIN)
		{
			if (wParam == WM_KEYDOWN)
			{
				g_bLeftWinKeyDown = true;
			}
			else if (wParam == WM_KEYUP)
			{
				g_bLeftWinKeyDown = false;
			}
		}
		else if (kb->vkCode == VK_LCONTROL)
		{
			if (wParam == WM_KEYDOWN)
			{
				g_bLeftCtrlKeyDown = true;
			}
			else if (wParam == WM_KEYUP)
			{
				g_bLeftCtrlKeyDown = false;
			}
		}
	}

	return CallNextHookEx(hKeyHook, nCode, wParam, lParam);
}

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		if (wParam == WM_LBUTTONDOWN)
		{
			g_bLeftButtonClick = true;
			if (isAllKeyDown())
			{
				lockInput = true;
				return 1;
			}

			lockInput = false;
		}
		else if (wParam == WM_LBUTTONUP)
		{
			g_bLeftButtonClick = false;
			if (lockInput)
			{
				return 1;
			}
		}
		else if (wParam == WM_RBUTTONDOWN)
		{
			g_bRightButtonClick = true;
			if (isAllKeyDown())
			{
				lockInput = true;
				return 1;
			}

			lockInput = false;
		}
		else if (wParam == WM_RBUTTONUP)
		{
			g_bRightButtonClick = false;
			if (lockInput)
			{
				return 1;
			}
		}
	}

	return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
}

extern "C" __declspec(dllexport) void InstallHook(HWND hWnd)
{
	hKeyHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyHookProc, gModule, NULL);
	hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, gModule, NULL);
}

extern "C" __declspec(dllexport) void UninstallHook()
{
	UnhookWindowsHookEx(hKeyHook);
	UnhookWindowsHookEx(hMouseHook);
}

