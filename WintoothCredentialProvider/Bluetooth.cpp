#include "stdafx.h"
#include "Bluetooth.h"
#include <vector>

using std::vector;
vector<BT_ADDR> vecBTHAddr;
CCommandWindow* pCW = NULL;

BOOL Ifexist(BT_ADDR b)
{
	for (vector<BT_ADDR>::iterator it = vecBTHAddr.begin(); it != vecBTHAddr.end();)
	{
		if (*it == b)
		{
			return TRUE;
		}
		else
		{
			it++;
		}
	}
	return FALSE;
}
void Delete(BT_ADDR b)
{
	for (std::vector<BT_ADDR>::iterator it = vecBTHAddr.begin(); it != vecBTHAddr.end();)
	{
		if (*it == b)
		{
			vecBTHAddr.erase(it);
			break;
		}
		else
		{
			it++;
		}
	}
}

HRESULT PerformSearch(DWORD dwControlFlags, LPVOID pCommandWindow)
{
	pCW = static_cast<CCommandWindow *>(pCommandWindow);
	WSADATA      wsaData;
	WSAStartup(0x0202, &wsaData);

	//search devices to get sa.btAddr
	WSAQUERYSET wsaq;
	HANDLE hLookup;
	union {
		CHAR buf[5000];
		double __unused; // ensure proper alignment
	};

	LPWSAQUERYSET pwsaResults = (LPWSAQUERYSET)buf;
	DWORD dwSize = sizeof(buf);
	BOOL bHaveName;
	ZeroMemory(&wsaq, sizeof(wsaq));

	wsaq.dwSize = sizeof(wsaq);
	wsaq.dwNameSpace = NS_BTH;
	wsaq.lpcsaBuffer = NULL;
	SetLastError(0);
	if (ERROR_SUCCESS != WSALookupServiceBegin(&wsaq, dwControlFlags, &hLookup))
	{
		return E_FAIL;
	}

	ZeroMemory(pwsaResults, sizeof(WSAQUERYSET));
	pwsaResults->dwSize = sizeof(WSAQUERYSET);
	pwsaResults->dwNameSpace = NS_BTH;
	pwsaResults->lpBlob = NULL;

	HRESULT hr = E_FAIL;
	SetLastError(0);
	while (ERROR_SUCCESS == WSALookupServiceNext(hLookup, LUP_RETURN_NAME | LUP_RETURN_ADDR, &dwSize, pwsaResults))
	{
		ASSERT(pwsaResults->dwNumberOfCsAddrs == 1);
		BT_ADDR b = ((SOCKADDR_BTH *)pwsaResults->lpcsaBuffer->RemoteAddr.lpSockaddr)->btAddr;
		//bHaveName = pwsaResults->lpszServiceInstanceName && *(pwsaResults->lpszServiceInstanceName);

		if (Ifexist(b))
		{
			continue;
		}
		else
		{
			BT_ADDR *pb = static_cast<BT_ADDR *> (malloc(sizeof(BT_ADDR)));
			*pb = b;
			vecBTHAddr.push_back(b);
			HANDLE handleThread = ::CreateThread(NULL, 0, thConnect, (LPVOID)pb, 0, NULL);
		}
	}
	WSALookupServiceEnd(hLookup);
	//WSACleanup();
	return hr;
}

DWORD WINAPI thConnect(LPVOID pb)
{
	CCommandWindow* pCommandWindow;
	if (!pCW)
		return 0;
	pCommandWindow = pCW;

	//local socket
	SetLastError(0);
	SOCKET client_socket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
	if (client_socket == INVALID_SOCKET)
	{
		return 0;
	}

	//structure of target address (type: bluetooth)
	//sa.btAddr is needed
	SOCKADDR_BTH sa;
	memset(&sa, 0, sizeof(sa));
	sa.addressFamily = AF_BTH;
	CLSIDFromString(BTHGUID, &sa.serviceClassId);
	sa.btAddr = *static_cast<BT_ADDR*>(pb);
	SetLastError(0);

	if (!connect(client_socket, (SOCKADDR *)(&sa), sizeof(sa)))
	{
		if (SUCCEEDED(ConnectForCreadential(client_socket, pCommandWindow)))
		{
			::PostThreadMessage(pCommandWindow->_lpThreadId, WM_TOGGLE_CONNECTED_STATUS, 0, 0);
		}
	}

	//WSACleanup();
	Delete(*static_cast<BT_ADDR*>(pb));
	free(pb);
	return 0;
}

HRESULT ConnectForCreadential(SOCKET socClient, LPVOID lpParameter)
{
	CCommandWindow* pCommandWindow = static_cast<CCommandWindow *>(lpParameter);
	if (!pCommandWindow->GetAccountStatus())
	{
		pCommandWindow->ChangeAccountStatus();

		int dwLen = 0;
		int res = 0;
		WCHAR wsz[MAX_COMPUTERNAME_LENGTH + 1];
		DWORD cch = ARRAYSIZE(wsz);
		CHAR *cwsz = (CHAR*)wsz;
		if (GetComputerNameW(wsz, &cch))
		{
			cwsz = (char *)malloc(2 * wcslen(wsz) + 1);
			wcstombs(cwsz, wsz, 2 * wcslen(wsz) + 1);
			dwLen = strlen(cwsz);
			res += send(socClient, "1\n", 2, MSG_OOB);
			res += send(socClient, cwsz, strlen(cwsz), MSG_OOB);
			res += send(socClient, "\n", 1, MSG_OOB);
			free(cwsz);
			dwLen += 3;
			if (res == dwLen)
			{
				dwLen = 0;
				int lenthA = 0, lenthB = 0;
				dwLen += recv(socClient, (char*)(&lenthA), 4, 0);
				dwLen += recv(socClient, (char*)(&lenthB), 4, 0);
				if (dwLen == 2)
				{
					dwLen = 0;
					CHAR Account[80] = { 0 }, Password[80] = { 0 };
					dwLen += recv(socClient, (char*)Account, lenthA * 2, 0);
					dwLen += recv(socClient, (char*)Password, lenthB * 2, 0);

					if (dwLen == lenthA + lenthB)
					{
						pCommandWindow->SetRecieveAccount(Account, Password);
						return S_OK;
					}
				}
			}
		}
		pCommandWindow->ChangeAccountStatus();
	}
	return E_FAIL;
}
