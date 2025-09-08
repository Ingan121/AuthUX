#pragma once
#include "pch.h"
#include <windows.h>
#include <stdio.h>
#include "gina_securitycontrol.h"
#include "gina_shutdownview.h"
#include "util.h"
#include "../lib/logonframe.h"

void ShowGinaSecurityView()
{
	if (!ginaManager::Get()->hGinaDll) {
		ginaManager::Get()->LoadGina();
	}

	ginaSecurityControl::Get()->Create();
	ginaSecurityControl::Get()->Show();
	ginaSecurityControl::Get()->BeginMessageLoop();
}

ginaSecurityControl* ginaSecurityControl::Get()
{
	static ginaSecurityControl dlg;
	return &dlg;
}

void ginaSecurityControl::Create()
{
	HINSTANCE hInstance = ginaManager::Get()->hInstance;
	HINSTANCE hGinaDll = ginaManager::Get()->hGinaDll;
	ginaSecurityControl::Get()->hDlg = CreateDialogParamW(hGinaDll, MAKEINTRESOURCEW(GetRes(GINA_DLG_SECURITY_CONTROL)), 0, (DLGPROC)DlgProc, 0);
	if (!ginaSecurityControl::Get()->hDlg)
	{
		MessageBoxW(0, L"Failed to create security control dialog! Please make sure your copy of msgina.dll in system32 is valid!", L"Error", MB_OK | MB_ICONERROR);
		return;
	}
	if (ginaManager::Get()->config.classicTheme)
	{
		MakeWindowClassic(ginaSecurityControl::Get()->hDlg);
	}
}

void ginaSecurityControl::Destroy()
{
	ginaSecurityControl* dlg = ginaSecurityControl::Get();
	EndDialog(dlg->hDlg, 0);
	PostMessage(dlg->hDlg, WM_DESTROY, 0, 0);
}

void ginaSecurityControl::Show()
{
	ginaSecurityControl* dlg = ginaSecurityControl::Get();
	CenterWindow(dlg->hDlg);
	ShowWindow(dlg->hDlg, SW_SHOW);
	UpdateWindow(dlg->hDlg);
}

void ginaSecurityControl::Hide()
{
	ginaSecurityControl* dlg = ginaSecurityControl::Get();
	ShowWindow(dlg->hDlg, SW_HIDE);
}

void ginaSecurityControl::BeginMessageLoop()
{
	ginaSecurityControl* dlg = ginaSecurityControl::Get();
	MSG msg;
	while (GetMessageW(&msg, NULL, 0, 0))
	{
		if (!IsDialogMessageW(dlg->hDlg, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}
}

int CALLBACK ginaSecurityControl::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
	{
		ginaManager::Get()->CloseAllDialogs();

		WCHAR _wszUserName[MAX_PATH], _wszDomainName[MAX_PATH];
		WCHAR szFormat[256], szText[1024];
		GetLoggedOnUserInfo(_wszUserName, MAX_PATH, _wszDomainName, MAX_PATH);
		LoadStringW(ginaManager::Get()->hGinaDll, GINA_STR_LOGON_NAME, szFormat, 256);
		swprintf_s(szText, szFormat, _wszUserName, _wszDomainName, _wszUserName);
		SetDlgItemTextW(hWnd, GetRes(IDC_SECURITY_LOGONNAME), szText);

		SYSTEMTIME _logonTime;
		WCHAR szDate[128], szTime[128], szDateText[256];
		GetUserLogonTime(&_logonTime);
		GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, NULL, &_logonTime, NULL, szDate, 128, NULL);
		GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, NULL, &_logonTime, NULL, szTime, 128);
		swprintf_s(szDateText, L"%s %s", szDate, szTime);
		SetDlgItemTextW(hWnd, GetRes(IDC_SECURITY_DATE), szDateText);

		ginaManager::Get()->MoveChildrenForBranding(hWnd, FALSE);

		break;
	}
	case WM_COMMAND:
	{
		if (LOWORD(wParam) == GetRes(IDC_SECURITY_LOCK))
		{
			//system("rundll32.exe user32.dll,LockWorkStation");
			//buttonsList[1].Press();
			LC::LogonUISecurityOptions options = LC::LogonUISecurityOptions_Lock;
			CLogonFrame::GetSingleton()->OnSecurityOptionSelected(options);
			EndDialog(hWnd, 0);
			PostQuitMessage(0); // Trigger exit loop
		}
		else if (LOWORD(wParam) == GetRes(IDC_SECURITY_LOGOFF))
		{
			//MessageBoxW(hWnd, L"Not implemented yet!", L"Logoff Dialog", MB_OK | MB_ICONINFORMATION);
			ShowLogoffDialog(hWnd);
		}
		else if (LOWORD(wParam) == GetRes(IDC_SECURITY_SHUTDOWN))
		{
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
			{
				wchar_t title[256], desc[256];
				LoadStringW(ginaManager::Get()->hGinaDll, GINA_STR_EMERGENCY_RESTART_TITLE, title, 256);
				LoadStringW(ginaManager::Get()->hGinaDll, GINA_STR_EMERGENCY_RESTART_DESC, desc, 256);
				if (MessageBoxW(hWnd, desc, title, MB_YESNO | MB_ICONERROR) == IDYES)
				{
					EmergencyRestart();
				}
			}
			else
			{
				ShowShutdownDialog(hWnd);
				//MessageBoxW(hWnd, L"Not implemented yet!", L"Shutdown Dialog", MB_OK | MB_ICONINFORMATION);
			}
		}
		else if (LOWORD(wParam) == GetRes(IDC_SECURITY_CHANGEPWD))
		{
			MessageBoxW(hWnd, L"Not implemented yet!", L"Change Password", MB_OK | MB_ICONINFORMATION);
		}
		else if (LOWORD(wParam) == GetRes(IDC_SECURITY_TASKMGR))
		{
			//MessageBoxW(hWnd, L"Not implemented yet!", L"Task Manager", MB_OK | MB_ICONINFORMATION);
			LC::LogonUISecurityOptions options = LC::LogonUISecurityOptions_TaskManager;
			CLogonFrame::GetSingleton()->OnSecurityOptionSelected(options);
			EndDialog(hWnd, 0);
			PostQuitMessage(0); // Trigger exit loop
		}
		else if (LOWORD(wParam) == IDC_CANCEL)
		{
			//MessageBoxW(hWnd, L"Not implemented yet! Just open sethc cmd and kill logonui", L"Cancel", MB_OK | MB_ICONINFORMATION);
			EndDialog(hWnd, 0);
			PostQuitMessage(0); // Trigger exit thread
		}
		else
		{
			MessageBoxW(hWnd, L"wtf is this command?", L"Unknown Command", MB_OK | MB_ICONERROR);
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
		PostQuitMessage(0); // Trigger exit thread
		break;
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0); // Trigger exit thread
		break;
	}
	}
	return 0;
}
