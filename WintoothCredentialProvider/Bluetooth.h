#pragma once

#include "CommandWindow.h"
#include <combaseapi.h>
#include "guid.h"
#include <winsock2.h>
#include <assert.h>
#include <Ws2bth.h>

#pragma comment(lib, "ws2_32.lib")

#ifndef	ASSERT
#define ASSERT assert
#endif

typedef ULONGLONG BT_ADDR;

HRESULT PerformSearch(DWORD dwControlFlags, LPVOID);
HRESULT ConnectForCreadential(SOCKET socClient, LPVOID lpParameter);
DWORD WINAPI thConnect(LPVOID pb);
