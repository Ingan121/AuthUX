#pragma once
#include "pch.h"
#include "gina_shutdownview.h"
#include "util.h"
#include "../lib/logonframe.h"
#include <powrprof.h>

#pragma comment(lib, "PowrProf.lib")

int CALLBACK ShutdownDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
	{
		if (ginaManager::Get()->config.classicTheme)
		{
			MakeWindowClassic(hWnd);
		}

		HINSTANCE hGinaDll = ginaManager::Get()->hGinaDll;

		// Load the icon (shutdown icon in msgina.dll for 2000+, generic MB_ICONINFORMATION icon for NT4)
		HWND hShutdownIcon = GetDlgItem(hWnd, GetRes(IDC_SHUTDOWN_ICON));
		HICON hIcon = NULL;
		if (ginaManager::Get()->ginaVersion == GINA_VER_NT4)
		{
			HINSTANCE hShell32 = LoadLibraryW(L"shell32.dll");
			if (hShell32)
			{
				hIcon = (HICON)LoadImageW(hShell32, MAKEINTRESOURCEW(SHELL32_INFO), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_VGACOLOR);
				FreeLibrary(hShell32);
			}
		}
		else
		{
			hIcon = LoadIconW(hGinaDll, MAKEINTRESOURCEW(IDI_SHUTDOWN));
		}
		if (hIcon)
		{
			SendMessageW(hShutdownIcon, STM_SETICON, (WPARAM)hIcon, 0);
		}

		// Populate shutdown combo (2000+)
		HWND hShutdownCombo = GetDlgItem(hWnd, GetRes(IDC_SHUTDOWN_COMBO));
		if (hShutdownCombo)
		{
			wchar_t shutdownStr[256];
			if (!IsSystemUser())
			{
				wchar_t logoffStr[256], username[MAX_PATH], domain[MAX_PATH];
				GetLoggedOnUserInfo(username, MAX_PATH, domain, MAX_PATH);
				LoadStringW(hGinaDll, GINA_STR_LOGOFF, logoffStr, 256);
				swprintf_s(shutdownStr, logoffStr, username);
				SendMessageW(hShutdownCombo, CB_ADDSTRING, 0, (LPARAM)shutdownStr);
			}
			LoadStringW(hGinaDll, GINA_STR_SHUTDOWN, shutdownStr, 256);
			SendMessageW(hShutdownCombo, CB_ADDSTRING, 0, (LPARAM)shutdownStr);
			LoadStringW(hGinaDll, GINA_STR_RESTART, shutdownStr, 256);
			SendMessageW(hShutdownCombo, CB_ADDSTRING, 0, (LPARAM)shutdownStr);
			LoadStringW(hGinaDll, GINA_STR_SLEEP, shutdownStr, 256);
			SendMessageW(hShutdownCombo, CB_ADDSTRING, 0, (LPARAM)shutdownStr);
			LoadStringW(hGinaDll, GINA_STR_HIBERNATE, shutdownStr, 256);
			SendMessageW(hShutdownCombo, CB_ADDSTRING, 0, (LPARAM)shutdownStr);
			// Set the default selection to Restart
			SendMessageW(hShutdownCombo, CB_SETCURSEL, IsSystemUser() ? 1 : 2, 0);

			wchar_t shutdownDesc[256];
			LoadStringW(hGinaDll, GINA_STR_RESTART_DESC, shutdownDesc, 256);
			SetDlgItemTextW(hWnd, GetRes(IDC_SHUTDOWN_DESC), shutdownDesc);
		}

		// Hide help button and move the OK and Cancel buttons (2000+)
		HWND hHelpBtn = GetDlgItem(hWnd, GetRes(IDC_SHUTDOWN_HELP));
		if (hHelpBtn)
		{
			ShowWindow(hHelpBtn, SW_HIDE);
			HWND hOkBtn = GetDlgItem(hWnd, IDC_OK);
			HWND hCancelBtn = GetDlgItem(hWnd, IDC_CANCEL);
			RECT helpRect, okRect, cancelRect;
			GetWindowRect(hHelpBtn, &helpRect);
			GetWindowRect(hOkBtn, &okRect);
			GetWindowRect(hCancelBtn, &cancelRect);
			MapWindowPoints(HWND_DESKTOP, hWnd, (LPPOINT)&helpRect, 2);
			MapWindowPoints(HWND_DESKTOP, hWnd, (LPPOINT)&okRect, 2);
			MapWindowPoints(HWND_DESKTOP, hWnd, (LPPOINT)&cancelRect, 2);
			SetWindowPos(hOkBtn, NULL, okRect.left + helpRect.right - helpRect.left + 8, okRect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(hCancelBtn, NULL, cancelRect.left + helpRect.right - helpRect.left + 8, cancelRect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		}

		// Hide the shutdown tracker (XP only)
		HWND hShutdownTracker = GetDlgItem(hWnd, GetRes(IDC_SHUTDOWN_TRACKER_GROUP));
		if (hShutdownTracker)
		{
			ShowWindow(hShutdownTracker, SW_HIDE);
			HWND hShutdownTrackerDesc = GetDlgItem(hWnd, GetRes(IDC_SHUTDOWN_TRACKER_DESC));
			HWND hShutdownTrackerBooted = GetDlgItem(hWnd, GetRes(IDC_SHUTDOWN_TRACKER_BOOTED));
			HWND hShutdownTrackerOptionsLabel = GetDlgItem(hWnd, GetRes(IDC_SHUTDOWN_TRACKER_OPTIONS_LABEL));
			HWND hShutdownTrackerOptionsCombo = GetDlgItem(hWnd, GetRes(IDC_SHUTDOWN_TRACKER_OPTIONS_COMBO));
			HWND hShutdownTrackerOptionsDesc = GetDlgItem(hWnd, GetRes(IDC_SHUTDOWN_TRACKER_OPTIONS_DESC));
			HWND hShutdownTrackerDescLabel = GetDlgItem(hWnd, GetRes(IDC_SHUTDOWN_TRACKER_DESC_LABEL));
			HWND hShutdownTrackerDescEdit = GetDlgItem(hWnd, GetRes(IDC_SHUTDOWN_TRACKER_DESC_EDIT));
			ShowWindow(hShutdownTrackerDesc, SW_HIDE);
			ShowWindow(hShutdownTrackerBooted, SW_HIDE);
			ShowWindow(hShutdownTrackerOptionsLabel, SW_HIDE);
			ShowWindow(hShutdownTrackerOptionsCombo, SW_HIDE);
			ShowWindow(hShutdownTrackerOptionsDesc, SW_HIDE);
			ShowWindow(hShutdownTrackerDescLabel, SW_HIDE);
			ShowWindow(hShutdownTrackerDescEdit, SW_HIDE);

			RECT trackerRect;
			GetWindowRect(hShutdownTracker, &trackerRect);
			MapWindowPoints(HWND_DESKTOP, hWnd, (LPPOINT)&trackerRect, 2);
			int trackerHeight = trackerRect.bottom - trackerRect.top;
			RECT dlgRect;
			GetWindowRect(hWnd, &dlgRect);

			RECT okRect, cancelRect;
			HWND hOkBtn = GetDlgItem(hWnd, IDC_OK);
			HWND hCancelBtn = GetDlgItem(hWnd, IDC_CANCEL);
			GetWindowRect(hOkBtn, &okRect);
			GetWindowRect(hCancelBtn, &cancelRect);
			MapWindowPoints(HWND_DESKTOP, hWnd, (LPPOINT)&okRect, 2);
			MapWindowPoints(HWND_DESKTOP, hWnd, (LPPOINT)&cancelRect, 2);
			SetWindowPos(hOkBtn, NULL, okRect.left, okRect.top - trackerHeight, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(hCancelBtn, NULL, cancelRect.left, cancelRect.top - trackerHeight, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

			RECT dlgRectNew;
			GetWindowRect(hWnd, &dlgRectNew);
			int dlgHeight = dlgRectNew.bottom - dlgRectNew.top;
			SetWindowPos(hWnd, NULL, dlgRectNew.left, dlgRectNew.top, dlgRectNew.right - dlgRectNew.left, dlgHeight - trackerHeight, SWP_NOZORDER);
		}

		// Hide power off option (no one uses non-ACPI systems anymore) (NT4 only)
		// And hide duplicate OK and Cancel buttons
		HWND hPowerOff = GetDlgItem(hWnd, IDC_SHUTDOWN_POWEROFF);
		if (hPowerOff)
		{
			HWND hOK = GetDlgItem(hWnd, IDC_SHUTDOWN_OK);
			HWND hCancel = GetDlgItem(hWnd, IDC_SHUTDOWN_CANCEL);
			ShowWindow(hPowerOff, SW_HIDE);
			ShowWindow(hOK, SW_HIDE);
			ShowWindow(hCancel, SW_HIDE);
		}

		// Check the shutdown checkbox
		HWND hShutdown = GetDlgItem(hWnd, IDC_SHUTDOWN_SHUTDOWN);
		if (hShutdown)
		{
			SendMessageW(hShutdown, BM_SETCHECK, BST_CHECKED, 0);
		}

		ginaManager::Get()->MoveChildrenForBranding(hWnd, FALSE);

		return TRUE;
	}
	case WM_COMMAND:
	{
		if (HIWORD(wParam) == CBN_SELCHANGE)
		{
			HWND hShutdownCombo = GetDlgItem(hWnd, GetRes(IDC_SHUTDOWN_COMBO));
			int index = SendMessageW(hShutdownCombo, CB_GETCURSEL, 0, 0);
			if (IsSystemUser())
			{
				index += 1;
			}
			int descId = 0;
			switch (index)
			{
			case 0:
				descId = GINA_STR_LOGOFF_DESC;
				break;
			case 1:
				descId = GINA_STR_SHUTDOWN_DESC;
				break;
			case 2:
				descId = GINA_STR_RESTART_DESC;
				break;
			case 3:
				descId = GINA_STR_SLEEP_DESC;
				break;
			case 4:
				descId = GINA_STR_HIBERNATE_DESC;
				break;
			}
			wchar_t shutdownDesc[256];
			LoadStringW(ginaManager::Get()->hGinaDll, descId, shutdownDesc, 256);
			SetDlgItemTextW(hWnd, GetRes(IDC_SHUTDOWN_DESC), shutdownDesc);
		}
		else if (LOWORD(wParam) == IDC_OK)
		{
			HWND hShutdownCombo = GetDlgItem(hWnd, GetRes(IDC_SHUTDOWN_COMBO));
			if (hShutdownCombo) // 2000+ (combobox)
			{
				int index = SendMessageW(hShutdownCombo, CB_GETCURSEL, 0, 0);
				if (IsSystemUser())
				{
					index += 1;
				}

				switch (index)
				{
				case 0:
					ExitWindowsEx(EWX_LOGOFF, 0);
					break;
				case 1:
					EnableShutdownPrivilege();
					ExitWindowsEx(EWX_SHUTDOWN, 0);
					break;
				case 2:
					EnableShutdownPrivilege();
					ExitWindowsEx(EWX_REBOOT, 0);
					break;
				case 3:
					SetSuspendState(FALSE, FALSE, FALSE);
					break;
				case 4:
					SetSuspendState(TRUE, FALSE, FALSE);
					break;
				}
			}
			else // NT4 (checkboxes)
			{
				int action = 0;
				if (IsDlgButtonChecked(hWnd, IDC_SHUTDOWN_SHUTDOWN) == BST_CHECKED)
				{
					action = EWX_SHUTDOWN;
				}
				else if (IsDlgButtonChecked(hWnd, IDC_SHUTDOWN_RESTART) == BST_CHECKED)
				{
					action = EWX_REBOOT;
				}
				EnableShutdownPrivilege();
				ExitWindowsEx(action, 0);
			}

			EndDialog(hWnd, 0);
		}
		else if (LOWORD(wParam) == IDC_CANCEL)
		{
			EndDialog(hWnd, 0);
		}
		break;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		RECT rc;
		GetClientRect(hWnd, &rc);
		ginaManager::Get()->PaintBranding(hdc, &rc, FALSE);
		EndPaint(hWnd, &ps);
		return 0;
	}
	case WM_CLOSE:
	{
		EndDialog(hWnd, 0);
		break;
	}
	}
	return 0;
}

int CALLBACK LogoffDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
	{
		if (ginaManager::Get()->config.classicTheme)
		{
			MakeWindowClassic(hWnd);
		}
		HINSTANCE hGinaDll = ginaManager::Get()->hGinaDll;

		// Load the icon (logoff icon in msgina.dll for 2000+, generic MB_ICONINFORMATION icon for NT4)
		HWND hLogoffIcon = GetDlgItem(hWnd, GetRes(IDC_LOGOFF_ICON));
		HICON hIcon = NULL;
		if (ginaManager::Get()->ginaVersion == GINA_VER_NT4)
		{
			HINSTANCE hShell32 = LoadLibraryW(L"shell32.dll");
			if (hShell32)
			{
				hIcon = (HICON)LoadImageW(hShell32, MAKEINTRESOURCEW(SHELL32_INFO), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_VGACOLOR);
				FreeLibrary(hShell32);
			}
		}
		else
		{
			hIcon = LoadIconW(hGinaDll, MAKEINTRESOURCEW(IDI_LOGOFF));
		}
		if (hIcon)
		{
			SendMessageW(hLogoffIcon, STM_SETICON, (WPARAM)hIcon, 0);
		}
	}
	case WM_COMMAND:
	{
		if (LOWORD(wParam) == IDC_OK)
		{
			// todo implement
			LC::LogonUISecurityOptions options = LC::LogonUISecurityOptions_LogOff;
			CLogonFrame::GetSingleton()->OnSecurityOptionSelected(options);
			EndDialog(hWnd, 0);
		}
		else if (LOWORD(wParam) == IDC_CANCEL)
		{
			EndDialog(hWnd, 0);
		}
		break;
	}
	}
	return 0;
}

void ShowShutdownDialog(HWND parent)
{
	HINSTANCE hGinaDll = ginaManager::Get()->hGinaDll;
	DialogBoxW(hGinaDll, MAKEINTRESOURCEW(GetRes(GINA_DLG_SHUTDOWN)), parent, (DLGPROC)ShutdownDlgProc);
}

void ShowLogoffDialog(HWND parent)
{
	HINSTANCE hGinaDll = ginaManager::Get()->hGinaDll;
	DialogBoxW(hGinaDll, MAKEINTRESOURCEW(GetRes(GINA_DLG_LOGOFF)), parent, (DLGPROC)LogoffDlgProc);
}
