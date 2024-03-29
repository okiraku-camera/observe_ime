// observe_ime.cpp Copyright (c) 2018 Takeshi Higasa.
// 
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#include "stdafx.h"
#include "observe_ime.h"
#include "observe_ime_wnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CobserveimeApp

BEGIN_MESSAGE_MAP(CobserveimeApp, CWinApp)
END_MESSAGE_MAP()

const TCHAR* appid = _T("observeime.AppID.NoVersion");

CobserveimeApp::CobserveimeApp() noexcept { 
	SetAppID(appid); 
	version = _T("unknown");
}

CobserveimeApp theApp;

#include "winver.h"

void CobserveimeApp::readVersionInfo() {
	TCHAR filename[MAX_PATH];

	if (GetModuleFileName(NULL, filename, sizeof(filename)) == 0)
		return;

	DWORD dw = GetFileVersionInfoSize(filename, NULL);
	char* pBlock = (char*)malloc(dw);
	if (!pBlock)
		return;
	GetFileVersionInfo(filename, NULL, dw, pBlock);
	VS_FIXEDFILEINFO* pInfo;
	UINT infoSize = 0;
	VerQueryValue((LPCVOID)pBlock, _T("\\"), (LPVOID*)&pInfo, &infoSize);

	version.Format(_T("%d.%d.%d.%d"), 
			HIWORD(pInfo->dwFileVersionMS), LOWORD(pInfo->dwFileVersionMS),
			HIWORD(pInfo->dwFileVersionLS), LOWORD(pInfo->dwFileVersionLS));
//	TRACE(_T("FileVersionMS=%08x, FileVersionLS=%08x, %s\n"), pInfo->dwFileVersionMS, pInfo->dwFileVersionLS, version);
	free(pBlock);
}


HANDLE mutex = 0;
BOOL CobserveimeApp::InitInstance() {
	CWinApp::InitInstance();
	if (!AfxOleInit())
		return FALSE;
	mutex = CreateMutex(0, 0, appid);
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		CloseHandle(mutex);
		mutex = 0;
		return 0;
	}
	SetRegistryKey(_T("okiraku-camera"));
	AfxEnableControlContainer();
	EnableTaskbarInteraction(FALSE);
	readVersionInfo();

	Cobserve_ime_wnd* p = new Cobserve_ime_wnd;
	if (!p->Create()) {
		delete p;
		return FALSE;
	}
	m_pMainWnd = p;
	m_pMainWnd->UpdateWindow();
	return TRUE;
}

int CobserveimeApp::ExitInstance() {
	if (mutex) {
		ReleaseMutex(mutex);
		CloseHandle(mutex);
	}
	AfxOleTerm(FALSE);
	return CWinApp::ExitInstance();
}
