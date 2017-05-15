
#pragma once

#include "CProvider.h"

#define WM_EXIT_THREAD              WM_USER + 1
#define WM_TOGGLE_CONNECTED_STATUS  WM_USER + 2
class CCommandWindow
{
public:
	CCommandWindow(void);
	~CCommandWindow(void);
	HRESULT Initialize(CProvider *pProvider);
	BOOL GetConnectedStatus();
	BOOL GetAccountStatus();
	VOID ChangeAccountStatus();
	HRESULT SetRecieveAccount(PTSTR, PTSTR);

private:
	HRESULT _MyRegisterClass(void);
	HRESULT _InitInstance();
	BOOL _ProcessNextMessage();
	PWSTR MuiltiToWide(PTSTR pChar);

	static DWORD WINAPI _ThreadProc(LPVOID lpParameter);
	static DWORD WINAPI _ThreadSearchUsingCache(LPVOID lpParameter);
	static DWORD WINAPI _ThreadSearchWithoutCache(LPVOID lpParameter);
	static LRESULT CALLBACK    _WndProc(HWND, UINT, WPARAM, LPARAM);

	CProvider              *_pProvider;        // Pointer to our owner.
	// HWND                        _hWnd;                // Handle to our window.
	// HWND                        _hWndButton;        // Handle to our window's button.
	HINSTANCE                    _hInst;                // Current instance
	BOOL                        _fConnected;        // Whether or not we're connected.

	//TUDO:
	BOOL                        _fComplete;
	PWSTR                      _pUsername;
	PWSTR                      _pPassword;
	HANDLE                      _handleThread;                // Handle to our thread.
	HANDLE						_handleThread1;
	HANDLE                      _handleThread2;
public:
	DWORD						_lpThreadId;
};
