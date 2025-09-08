#pragma once
#include <windows.h>
#include <Uxtheme.h>
#include <dwmapi.h>

#pragma comment(lib, "UxTheme.lib")

static void CenterWindow(HWND hWnd)
{
	RECT rc;
	GetWindowRect(hWnd, &rc);
	int windowWidth = rc.right - rc.left;
	int windowHeight = rc.bottom - rc.top;
	int xPos = (GetSystemMetrics(SM_CXSCREEN) - windowWidth) / 2;
	int yPos = (GetSystemMetrics(SM_CYSCREEN) - windowHeight) / 3;
	SetWindowPos(hWnd, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

static bool EnableShutdownPrivilege()
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		return false;
	}
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
	if (GetLastError() != ERROR_SUCCESS)
	{
		return false;
	}
	return true;
}

static void MakeWindowClassic(HWND hWnd)
{
	SetWindowTheme(hWnd, L" ", L" ");
    DWMNCRENDERINGPOLICY ncrp = DWMNCRP_DISABLED;
    DwmSetWindowAttribute(hWnd, DWMWA_NCRENDERING_POLICY, &ncrp, sizeof(DWMNCRENDERINGPOLICY));
	// Iterate over all child windows
	HWND hChild = GetWindow(hWnd, GW_CHILD);
	while (hChild != NULL)
	{
		SetWindowTheme(hChild, L" ", L" ");
		hChild = GetWindow(hChild, GW_HWNDNEXT);
	}
}

DWORD GetLoggedOnUserInfo(LPWSTR lpUsername, UINT cchUsernameMax, LPWSTR lpDomain, UINT cchDomainMax);
int GetLastLogonUser(LPWSTR lpUsername, UINT cchUsernameMax);
bool GetUserLogonTime(LPSYSTEMTIME lpSystemTime);
int GetConfigInt(LPCWSTR lpValueName, int defaultValue);
bool SetConfigInt(LPCWSTR lpValueName, int value);
bool GetConfigString(LPCWSTR lpValueName, LPWSTR lpBuffer, DWORD dwBufferSize, LPCWSTR lpDefaultValue = NULL);
bool IsSystemUser(void);
bool IsFriendlyLogonUI(void);
bool GetUserSid(LPCWSTR lpUsername, LPWSTR lpSid, DWORD dwSidSize);
bool GetUserHomeDir(LPWSTR lpUsername, LPWSTR lpHomeDir, DWORD dwHomeDirSize);
LSTATUS GetUserRegHive(REGSAM samDesired = KEY_READ, PHKEY phkResult = NULL);
void ApplyUserColors();
void EmergencyRestart(void);
