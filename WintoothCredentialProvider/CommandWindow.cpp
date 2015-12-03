#include "stdafx.h"
#include "CommandWindow.h"
#include <strsafe.h>

// Custom messages for managing the behavior of the window thread.
#define WM_EXIT_THREAD              WM_USER + 1
#define WM_TOGGLE_CONNECTED_STATUS  WM_USER + 2

const TCHAR *g_wszClassName = "EventWindow";
const TCHAR *g_wszConnected = "Connected";
const TCHAR *g_wszDisconnected = "Disconnected";

CCommandWindow::CCommandWindow(void)
{
	_handleThread = NULL;
	_hInst = NULL;
	_fConnected = FALSE;
	_pProvider = NULL;
	_fComplete = FALSE;
	_pUsername = NULL;
	_pPassword = NULL;
}

CCommandWindow::~CCommandWindow(void)
{
	// If we have an active window, we want to post it an exit message.

	WSACleanup();
	if (_handleThread != NULL)
	{
		::PostThreadMessage(_lpThreadId, WM_EXIT_THREAD, 0, 0);
		_handleThread = NULL;
	}

	// We'll also make sure to release any reference we have to the provider.
	if (_pProvider != NULL)
	{
		_pProvider->Release();
		_pProvider = NULL;
	}
}

// Performs the work required to spin off our message so we can listen for events.
HRESULT CCommandWindow::Initialize(CProvider *pProvider)
{
	HRESULT hr = S_OK;

	// Be sure to add a release any existing provider we might have, then add a reference
	// to the provider we're working with now.
	if (_pProvider != NULL)
	{
		_pProvider->Release();
	}
	_pProvider = pProvider;
	_pProvider->AddRef();

	// Create and launch the window thread.
	_handleThread = ::CreateThread(NULL, 0, CCommandWindow::_ThreadProc, (LPVOID) this, 0, &_lpThreadId);
	if (_handleThread == NULL)
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
	}

	return hr;
}

// Wraps our internal connected status so callers can easily access it.
BOOL CCommandWindow::GetConnectedStatus()
{
	return _fConnected;
}


BOOL CCommandWindow::GetAccountStatus()
{
	return _fComplete;
}


VOID CCommandWindow::ChangeAccountStatus()
{
	_fComplete = !_fComplete;
}

//
//  FUNCTION: _MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//

/*
HRESULT CCommandWindow::_MyRegisterClass(void)
{
	HRESULT hr = S_OK;

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = CCommandWindow::_WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = _hInst;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = ::g_wszClassName;
	wcex.hIconSm = NULL;

	if (!RegisterClassEx(&wcex))
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
	}

	return hr;
}
*/


// Called from the separate thread to process the next message in the message queue. If
// there are no messages, it'll wait for one.
BOOL CCommandWindow::_ProcessNextMessage()
{
	// Grab, translate, and process the message.
	MSG msg;
	(void) ::GetMessage(&(msg), NULL, 0, 0);
	//(void) ::TranslateMessage(&(msg));
	//(void) ::DispatchMessage(&(msg));

	// This section performs some "post-processing" of the message. It's easier to do these
	// things here because we have the handles to the window, its button, and the provider
	// handy.
	switch (msg.message)
	{
		// Return to the thread loop and let it know to exit.
	case WM_EXIT_THREAD: return FALSE;

		// Toggle the connection status, which also involves updating the UI.
	case WM_TOGGLE_CONNECTED_STATUS:
		_fConnected = !_fConnected;
		/*
		if (_fConnected)
		{
		::SetWindowText(_hWnd, ::g_wszConnected);
		::SetWindowText(_hWndButton, "Press to disconnect");
		}
		else
		{
		::SetWindowText(_hWnd, ::g_wszDisconnected);
		::SetWindowText(_hWndButton, "Press to connect");
		}
		*/
		_pProvider->_pCredential->SetBluetoothAccount(_pUsername, _pPassword);

		_pProvider->OnConnectStatusChanged();
		break;
	}
	return TRUE;
}

// Manages window messages on the window thread.
LRESULT CALLBACK CCommandWindow::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		// Originally we were going to work with USB keys being inserted and removed, but it
		// seems as though these events don't get to us on the secure desktop. However, you
		// might see some messageboxi in CredUI.
		// TODO: Remove if we can't use from LogonUI.
		//case WM_DEVICECHANGE:
		//::MessageBox(NULL, TEXT("Device change"), TEXT("Device change"), 0);
		//break;

		// We assume this was the button being clicked.
	case WM_COMMAND:
		::PostMessage(hWnd, WM_TOGGLE_CONNECTED_STATUS, 0, 0);
		break;

		// To play it safe, we hide the window when "closed" and post a message telling the 
		// thread to exit.
	case WM_CLOSE:
		::ShowWindow(hWnd, SW_HIDE);
		::PostMessage(hWnd, WM_EXIT_THREAD, 0, 0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Our thread procedure. We actually do a lot of work here that could be put back on the 
// main thread, such as setting up the window, etc.
DWORD WINAPI CCommandWindow::_ThreadProc(LPVOID lpParameter)
{
	CCommandWindow *pCommandWindow = static_cast<CCommandWindow *>(lpParameter);
	if (pCommandWindow == NULL)
	{
		// TODO: What's the best way to raise this error?
		return 0;
	}

	HRESULT hr = S_OK;

	//Create Bluetooth thread

	if (SUCCEEDED(hr))
	{
		HANDLE hThread = ::CreateThread(NULL, 0, CCommandWindow::_ThreadSearchUsingCache, lpParameter, 0, NULL);
		if (hThread == NULL)
		{
			hr = HRESULT_FROM_WIN32(::GetLastError());
		}
		else
		{
			hThread = ::CreateThread(NULL, 0, CCommandWindow::_ThreadSearchWithoutCache, lpParameter, 0, NULL);
			if (hThread == NULL)
			{
				hr = HRESULT_FROM_WIN32(::GetLastError());
			}
		}
	}
	// ProcessNextMessage will pump our message pump and return false if it comes across
	// a message telling us to exit the thread.
	if (SUCCEEDED(hr))
	{
		while (pCommandWindow->_ProcessNextMessage())
		{
		}
	}

	return 0;
}


DWORD WINAPI CCommandWindow::_ThreadSearchUsingCache(LPVOID lpParameter)
{
	CCommandWindow* pCommandWindow = static_cast<CCommandWindow *>(lpParameter);
	HRESULT hr;
	while (true)
	{
		hr = PerformSearch(LUP_CONTAINERS, pCommandWindow);
		/*
		if (SUCCEEDED(hr))
		{
		::PostThreadMessage(pCommandWindow->_lpThreadId, WM_TOGGLE_CONNECTED_STATUS, 0, 0);
		return 0;
		}
		if (pCommandWindow->GetConnectedStatus())
		return 0;
		//Sleep(200);
		*/
	}
}

DWORD WINAPI CCommandWindow::_ThreadSearchWithoutCache(LPVOID lpParameter)
{
	CCommandWindow* pCommandWindow = static_cast<CCommandWindow *>(lpParameter);
	HRESULT hr;
	while (true)
	{
		hr = PerformSearch(LUP_CONTAINERS | LUP_FLUSHCACHE, pCommandWindow);
		/*
		if (SUCCEEDED(hr))
		{
		::PostThreadMessage(pCommandWindow->_lpThreadId, WM_TOGGLE_CONNECTED_STATUS, 0, 0);
		return 0;
		}
		*/
		if (pCommandWindow->GetConnectedStatus())
			return 0;
	}
}


HRESULT CCommandWindow::SetRecieveAccount(PTSTR usr, PTSTR pwd)
{
	_pUsername = MuiltiToWide(usr);
	_pPassword = MuiltiToWide(pwd);
	return S_OK;

}

PWSTR CCommandWindow::MuiltiToWide(PTSTR pChar)
{
	wchar_t* pWCHAR = NULL;

	//计算pChar所指向的多字节字符串相当于多少个宽字节
	DWORD num = MultiByteToWideChar(CP_ACP, 0, pChar, -1, NULL, 0);

	pWCHAR = (wchar_t*)malloc(num*sizeof(wchar_t));

	if (pWCHAR == NULL)
	{
		free(pWCHAR);
	}

	memset(pWCHAR, 0, num*sizeof(wchar_t));

	//多字节转换为宽字节
	MultiByteToWideChar(CP_ACP, 0, pChar, -1, pWCHAR, num);
	return pWCHAR;
}