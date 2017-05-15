//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//

#pragma once

#include <credentialprovider.h>
#include <strsafe.h>

#include "CommandWindow.h"
#include "CCredential.h"
#include "MessageCredential.h"
#include "helpers.h"

// Forward references for classes used here.
class CCommandWindow;
class CCredential;
class CMessageCredential;

class CProvider : public ICredentialProvider
{
public:
	// IUnknown
	STDMETHOD_(ULONG, AddRef)()
	{
		return _cRef++;
	}

	STDMETHOD_(ULONG, Release)()
	{
		LONG cRef = _cRef--;
		if (!cRef)
		{
			delete this;
		}
		return cRef;
	}

	STDMETHOD(QueryInterface)(REFIID riid, void** ppv)
	{
		HRESULT hr;
		if (IID_IUnknown == riid ||
			IID_ICredentialProvider == riid)
		{
			*ppv = this;
			reinterpret_cast<IUnknown*>(*ppv)->AddRef();
			hr = S_OK;
		}
		else
		{
			*ppv = NULL;
			hr = E_NOINTERFACE;
		}
		return hr;
	}

public:
	IFACEMETHODIMP SetUsageScenario(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags);
	IFACEMETHODIMP SetSerialization(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs);

	IFACEMETHODIMP Advise(__in ICredentialProviderEvents* pcpe, UINT_PTR upAdviseContext);
	IFACEMETHODIMP UnAdvise();

	IFACEMETHODIMP GetFieldDescriptorCount(__out DWORD* pdwCount);
	IFACEMETHODIMP GetFieldDescriptorAt(DWORD dwIndex, __deref_out CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd);

	IFACEMETHODIMP GetCredentialCount(__out DWORD* pdwCount,
		__out DWORD* pdwDefault,
		__out BOOL* pbAutoLogonWithDefault);
	IFACEMETHODIMP GetCredentialAt(DWORD dwIndex,
		__out ICredentialProviderCredential** ppcpc);

	friend HRESULT CProvider_CreateInstance(REFIID riid, __deref_out void** ppv);

public:
	void OnConnectStatusChanged();

protected:
	CProvider();
	__override ~CProvider();

public:
	//This is not safe弄好了赶快改
	CCredential           *_pCredential;          // Our "connected" credential.

private:
	CCommandWindow              *_pCommandWindow;       // Emulates external events.
	LONG                        _cRef;                  // Reference counter.
	CMessageCredential          *_pMessageCredential;   // Our "disconnected" credential.
	ICredentialProviderEvents   *_pcpe;                    // Used to tell our owner to re-enumerate credentials.
	UINT_PTR                    _upAdviseContext;       // Used to tell our owner who we are when asking to 
	// re-enumerate credentials.
	CREDENTIAL_PROVIDER_USAGE_SCENARIO      _cpus;
};
