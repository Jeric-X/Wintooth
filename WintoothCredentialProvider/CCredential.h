#pragma once

#include "common.h"
#include <windows.h>
#include <unknwn.h>
#include <strsafe.h>
#include <shlguid.h>
#include "helpers.h"
#include "dll.h"
#include "resource.h"
#include "CommandWindow.h"


class CCredential : public ICredentialProviderCredential
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
		if (ppv != NULL)
		{
			if (IID_IUnknown == riid ||
				IID_ICredentialProviderCredential == riid)
			{
				*ppv = static_cast<IUnknown*>(this);
				reinterpret_cast<IUnknown*>(*ppv)->AddRef();
				hr = S_OK;
			}
			else
			{
				*ppv = NULL;
				hr = E_NOINTERFACE;
			}
		}
		else
		{
			hr = E_INVALIDARG;
		}
		return hr;
	}
public:
	// ICredentialProviderCredential
	IFACEMETHODIMP Advise(ICredentialProviderCredentialEvents* pcpce);
	IFACEMETHODIMP UnAdvise();

	IFACEMETHODIMP SetSelected(BOOL* pbAutoLogon);
	IFACEMETHODIMP SetDeselected();

	IFACEMETHODIMP GetFieldState(DWORD dwFieldID,
		CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs,
		CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis);

	IFACEMETHODIMP GetStringValue(DWORD dwFieldID, PWSTR* ppwsz);
	IFACEMETHODIMP GetBitmapValue(DWORD dwFieldID, HBITMAP* phbmp);
	IFACEMETHODIMP GetCheckboxValue(DWORD dwFieldID, BOOL* pbChecked, PWSTR* ppwszLabel);
	IFACEMETHODIMP GetComboBoxValueCount(DWORD dwFieldID, DWORD* pcItems, DWORD* pdwSelectedItem);
	IFACEMETHODIMP GetComboBoxValueAt(DWORD dwFieldID, DWORD dwItem, PWSTR* ppwszItem);
	IFACEMETHODIMP GetSubmitButtonValue(DWORD dwFieldID, DWORD* pdwAdjacentTo);

	IFACEMETHODIMP SetStringValue(DWORD dwFieldID, PCWSTR pwz);
	IFACEMETHODIMP SetCheckboxValue(DWORD dwFieldID, BOOL bChecked);
	IFACEMETHODIMP SetComboBoxSelectedValue(DWORD dwFieldID, DWORD dwSelectedItem);
	IFACEMETHODIMP CommandLinkClicked(DWORD dwFieldID);

	IFACEMETHODIMP GetSerialization(CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr,
		CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs,
		PWSTR* ppwszOptionalStatusText,
		CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon);
	IFACEMETHODIMP ReportResult(NTSTATUS ntsStatus,
		NTSTATUS ntsSubstatus,
		PWSTR* ppwszOptionalStatusText,
		CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon);

public:
	HRESULT Initialize(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
		const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* rgcpfd,
		const FIELD_STATE_PAIR* rgfsp);
	CCredential();

	HRESULT SetBluetoothAccount(PWSTR pUsername, PWSTR pPassword);

	virtual ~CCredential();


private:
	LONG                                  _cRef;

	CREDENTIAL_PROVIDER_USAGE_SCENARIO    _cpus; // The usage scenario for which we were enumerated.

	CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR  _rgCredProvFieldDescriptors[SFI_NUM_FIELDS];  // An array holding the type 
	// and name of each field in 
	// the tile.

	FIELD_STATE_PAIR                      _rgFieldStatePairs[SFI_NUM_FIELDS];           // An array holding the state 
	// of each field in the tile.

	ICredentialProviderCredentialEvents*  _pCredProvCredentialEvents;
	PWSTR                                 _rgFieldStrings[SFI_NUM_FIELDS];              // An array holding the string 
	// value of each field. This is 
	// different from the name of 
	// the field held in 
	// _rgCredProvFieldDescriptors.

	bool								  _fComplete = FALSE;
};
