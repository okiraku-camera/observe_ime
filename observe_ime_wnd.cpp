// observe_ime_wnd.cpp Copyright (c) 2018 Takeshi Higasa.
//	1.0.2 Nov.25 2019.
//  1.1.0 Feb.22 2022 support hoboNicola msc notification.
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#include "stdafx.h"
#include "observe_ime.h"
#include "observe_ime_wnd.h"
#include "hoboMSC.h"
#include <imm.h>


static const int observe_interval = 200;
static const int msc_search_interval = 500;
ChoboMSC hoboMSC;

Cobserve_ime_wnd::Cobserve_ime_wnd() {
	m_timer_id = m_msc_timer = 0;
	m_trayMenu = 0;
	m_trayIcon = 0;
	m_trayIcon2 = 0;
	m_enabled = false;
	m_useMSC = false;
	m_keycode = VK_SCROLL;
	m_interval = observe_interval;
}

BEGIN_MESSAGE_MAP(Cobserve_ime_wnd, CWnd)
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_MESSAGE(PM_SHELLNOTIFY, &Cobserve_ime_wnd::OnPmShellnotify)
	ON_MESSAGE(IDM_EXIT_APP, &Cobserve_ime_wnd::OnIdmExitApp)
	ON_WM_POWERBROADCAST()
END_MESSAGE_MAP()

void Cobserve_ime_wnd::PostNcDestroy() {
	CWnd::PostNcDestroy();
	delete this;
}

bool Cobserve_ime_wnd::Create() {
	LPCTSTR lpcClass = AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW);
	HWND hwnd = ::FindWindow(lpcClass, AfxGetAppName());
	if (hwnd != 0)
		return false;
	return CreateEx(0, lpcClass, AfxGetAppName(), WS_POPUP, CRect(0, 0, 0, 0), NULL , NULL);
}

static CString tip_working = _T("observe_ime working.");
static CString tip_error = _T("observe_ime NO hoboNicola.");


int Cobserve_ime_wnd::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	m_trayIcon = theApp.LoadIcon(IDI_ICON2);
	m_trayIcon2 = theApp.LoadIcon(IDI_ICON3);
	m_keycode = theApp.GetProfileInt(_T("settings"), _T("key"), VK_SCROLL);
	m_interval = theApp.GetProfileInt(_T("settings"), _T("observe_interval"), observe_interval);
	if (m_interval < 50)
		m_interval = 50;
	else if (m_interval > 1000)
		m_interval = 1000;

	tip_working.Format(_T("observe_ime %s working."), theApp.version);
	tip_error.Format(_T("observe_ime %s NO hoboNicola."), theApp.version);

	m_enabled = true;
	if (theApp.GetProfileInt(_T("settings"), _T("useMSC"), 0)) {
		m_useMSC = true;
		if (!hoboMSC.find_msc_drive() || !hoboMSC.open()) { // hoboNicolaドライブが見つからない。
			m_enabled = false;
			m_msc_timer = SetTimer(2, msc_search_interval, NULL);
		}
	}
	if (m_enabled)
		m_timer_id = SetTimer(1, m_interval, NULL);
	SetNotifyIcon();
	return 0;
}

void Cobserve_ime_wnd::OnDestroy() {
	CWnd::OnDestroy();
	notify_keyboard(false);
	if (m_msc_timer) {
		KillTimer(m_msc_timer);
		m_msc_timer = 0;
	}
	if (m_timer_id) {
		KillTimer(m_timer_id);
		m_timer_id = 0;
	}
	m_enabled = false;
}

afx_msg LRESULT Cobserve_ime_wnd::OnIdmExitApp(WPARAM wParam, LPARAM lParam) {
	DestroyWindow();
	return 0;
}

void Cobserve_ime_wnd::SetMenuChecked() {
	if (!m_trayMenu)
		return;
	CheckMenuItem(m_trayMenu, IDM_OBSERVING, MF_BYCOMMAND | m_enabled ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(m_trayMenu, IDM_MSC_NOTIFY, MF_BYCOMMAND | m_useMSC ? MF_CHECKED : MF_UNCHECKED);

	CheckMenuItem(m_trayMenu, IDM_VKSCROLL, MF_BYCOMMAND | (m_keycode == VK_SCROLL) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(m_trayMenu, IDM_VKNUMLOCK, MF_BYCOMMAND | (m_keycode == VK_NUMLOCK) ? MF_CHECKED : MF_UNCHECKED);

	//EnableMenuItem(m_trayMenu, IDM_VKSCROLL, MF_BYCOMMAND | m_useMSC ? MF_GRAYED : MF_ENABLED);
	//EnableMenuItem(m_trayMenu, IDM_VKNUMLOCK, MF_BYCOMMAND | m_useMSC ? MF_GRAYED : MF_ENABLED);
}


BOOL Cobserve_ime_wnd::OnCommand(WPARAM wParam, LPARAM lParam) {
	int key = m_keycode;
	if (wParam == IDM_EXIT_APP) {
		PostMessage(WM_CLOSE);
		DeleteNotifyIcon();
	} else if (wParam == IDM_OBSERVING) {
		if (m_timer_id) {
			KillTimer(m_timer_id);
			m_timer_id = 0;
			m_enabled = false;
		}
		else {
			m_timer_id = SetTimer(1, m_interval, NULL);
			m_enabled = true;
			ChangeNotifyIcon(false);

		}
	} else if (wParam == IDM_MSC_NOTIFY) {
		if (m_useMSC) {
			hoboMSC.close();
			m_useMSC = false;
			theApp.WriteProfileInt(_T("settings"), _T("useMSC"), 0);
		} else {
			if (hoboMSC.find_msc_drive() && hoboMSC.open()) {
				m_useMSC = true;
				theApp.WriteProfileInt(_T("settings"), _T("useMSC"), 1);
			} else
				AfxMessageBox(_T("hoboNicola not found."));
		}
	}
	else if (wParam == IDM_VKNUMLOCK)
		m_keycode = VK_NUMLOCK;
	else if (wParam == IDM_VKSCROLL)
		m_keycode = VK_SCROLL;
	if (key != m_keycode)
		theApp.WriteProfileInt(_T("settings"), _T("key"), m_keycode);

	SetMenuChecked();

	return CWnd::OnCommand(wParam, lParam);
}

void Cobserve_ime_wnd::send_keycode(WORD vkey, key_action_t action) {

	if (action == on || action == off) {
		SHORT u = GetKeyState(vkey);
		if ((u & 1) && action == on)
			return;
		if (!(u & 1) && action == off)
			return;
	}
	INPUT input = { 0 };
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = vkey;
	input.ki.dwFlags = KEYEVENTF_EXTENDEDKEY;
	SendInput(1, &input, sizeof(INPUT));

	input.ki.dwFlags = KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP;
	SendInput(1, &input, sizeof(INPUT));
}

// NICOLAモードのオン・オフ通知
// MSCを使うときはScrLockやNumLockを送信しない。
void  Cobserve_ime_wnd::notify_keyboard(bool kana) {
	if (!m_enabled)
		return;
	if (!m_useMSC)
		send_keycode(m_keycode, kana ? on : off);
	else {
		if (!hoboMSC.msc_notify(kana)) {
//			m_useMSC = false;
			m_enabled = false;
			KillTimer(m_timer_id);
			m_timer_id = 0;
			MessageBeep(MB_ICONHAND);
			TRACE(_T("notify_keyboard(msc) failed.\n"));
			ChangeNotifyIcon(true);
			m_msc_timer = SetTimer(2, msc_search_interval, NULL);
		}
	}
}

#ifndef IMC_GETCONVERSIONMODE 
#define IMC_GETCONVERSIONMODE 1
#endif
#ifndef IMC_GETOPENSTATUS 
#define IMC_GETOPENSTATUS 5
#endif

void Cobserve_ime_wnd::OnTimer(UINT_PTR id) {
	CWnd::OnTimer(id);
	if (id == m_timer_id) {
		bool status = false;
		static bool last_status = false;
		GUITHREADINFO gti = { 0 };
		gti.cbSize = sizeof(GUITHREADINFO);
		GetGUIThreadInfo(NULL, &gti);
		if (IsWindow(gti.hwndFocus)) {
			HWND hwndime = ImmGetDefaultIMEWnd(gti.hwndFocus);
			if (!IsWindow(hwndime)) {
				TRACE(_T("ImmGetDefaultIMEWnd() failed\n"));
				return;
			}
			if (::SendMessage(hwndime, WM_IME_CONTROL, IMC_GETOPENSTATUS, 0) == 1)
				status = (::SendMessage(hwndime, WM_IME_CONTROL, IMC_GETCONVERSIONMODE, 0) & 1) != 0;
		}
		if (status != last_status) {
			last_status = status;
			notify_keyboard(status);
		}
	} else if (id == m_msc_timer) {
		if (hoboMSC.find_msc_drive() && hoboMSC.open()) {
			KillTimer(m_msc_timer);
			m_msc_timer = 0;
			if (!m_enabled) {
				m_timer_id = SetTimer(1, m_interval, NULL);
				m_enabled = true;
				ChangeNotifyIcon(false);
			}
		}
	}
}


void Cobserve_ime_wnd::SetNotifyIcon() {

	NOTIFYICONDATA	nic;
	memset(&nic, 0, sizeof(NOTIFYICONDATA));
	nic.cbSize = sizeof(NOTIFYICONDATA);
	nic.hWnd = m_hWnd;
	nic.uID = IDI_ICON2;

	nic.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nic.uCallbackMessage = PM_SHELLNOTIFY;
	nic.hIcon = m_trayIcon;
	lstrcpy(nic.szTip, tip_working);
	Shell_NotifyIcon(NIM_ADD, &nic);
}

void Cobserve_ime_wnd::DeleteNotifyIcon() {
	NOTIFYICONDATA	nic;
	memset(&nic, 0, sizeof(NOTIFYICONDATA));
	nic.cbSize = sizeof(NOTIFYICONDATA);
	nic.hWnd = m_hWnd;
	nic.uID = IDI_ICON2;
	Shell_NotifyIcon(NIM_DELETE, &nic);
}

void Cobserve_ime_wnd::ChangeNotifyIcon(bool error) {
	NOTIFYICONDATA	nic;
	memset(&nic, 0, sizeof(NOTIFYICONDATA));
	nic.cbSize = sizeof(NOTIFYICONDATA);
	nic.hWnd = m_hWnd;
	nic.uID = IDI_ICON2;
	nic.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nic.uCallbackMessage = PM_SHELLNOTIFY;
	nic.hIcon = error ? m_trayIcon2 : m_trayIcon;
	lstrcpy(nic.szTip, error ? tip_error : tip_working);
	Shell_NotifyIcon(NIM_MODIFY, &nic);
}


afx_msg LRESULT Cobserve_ime_wnd::OnPmShellnotify(WPARAM wParam, LPARAM lParam)
{
	POINT point;
	switch (LOWORD(lParam)) {
	case WM_RBUTTONDOWN:
		if (m_trayMenu == 0) {
			m_trayMenu = CreatePopupMenu();
			if (m_trayMenu) {
				AppendMenu(m_trayMenu, MF_STRING, IDM_OBSERVING, _T("IME監視"));
				AppendMenu(m_trayMenu, MF_SEPARATOR, 0, NULL);
				AppendMenu(m_trayMenu, MF_STRING, IDM_VKSCROLL, _T("ScrLock"));
				AppendMenu(m_trayMenu, MF_STRING, IDM_VKNUMLOCK, _T("NumLock"));
				AppendMenu(m_trayMenu, MF_SEPARATOR, 0, NULL);
				AppendMenu(m_trayMenu, MF_STRING, IDM_MSC_NOTIFY, _T("Use MSC"));
				AppendMenu(m_trayMenu, MF_SEPARATOR, 0, NULL);
				AppendMenu(m_trayMenu, MF_STRING, IDM_EXIT_APP, _T("終了"));
			}
		}
		SetMenuChecked();
		GetCursorPos(&point);
		SetForegroundWindow();
		TrackPopupMenu(m_trayMenu, TPM_VERTICAL, point.x, point.y, 0, m_hWnd, NULL);
		break;
	default:
		break;
	}
	return 1;
}

UINT Cobserve_ime_wnd::OnPowerBroadcast(UINT nPowerEvent, LPARAM nEventData)
{
	if (nPowerEvent == PBT_APMSUSPEND) {
		KillTimer(m_timer_id);
		m_timer_id = 0;
		notify_keyboard(false);
	}
	else if (nPowerEvent == PBT_APMRESUMEAUTOMATIC) {
		notify_keyboard(false);
		if (m_enabled && m_timer_id == 0)
			m_timer_id = SetTimer(1, m_interval, NULL);
	}

	return CWnd::OnPowerBroadcast(nPowerEvent, nEventData);
}


