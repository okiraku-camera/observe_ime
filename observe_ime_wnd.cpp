// observe_ime_wnd.cpp Copyright (c) 2018 Takeshi Higasa.
//	1.0.2 Nov.25 2019.
//  1.1.0 Feb.22 2022 support hoboNicola msc notification.
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#include "stdafx.h"
#include "observe_ime.h"
#include "observe_ime_wnd.h"
#include "hoboMSC.h"
#include "hoboHID.h"
#include "HIDSettingsDlg.h"
#include "DeviceHistory.h"
#include <imm.h>

static const int observe_interval = 200;
static const int msc_search_interval = 500;
ChoboMSC hoboMSC;
ChoboHID hoboHID;

UINT Cobserve_ime_wnd::m_uTaskbarRestart = 0;

Cobserve_ime_wnd::Cobserve_ime_wnd() {
	m_timer_id = m_msc_timer = 0;
	m_trayMenu = 0;
	m_trayIcon = 0;
	m_trayIcon2 = 0;
	m_fTrayIcon = false;
	m_fErrorNotify = false;

	m_enabled = false;
	m_notifyMethod = METHOD_LOCKKEY;
	m_keycode = VK_SCROLL;
	m_interval = observe_interval;
	m_lastThreadId = 0;
}

BEGIN_MESSAGE_MAP(Cobserve_ime_wnd, CWnd)
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_MESSAGE(PM_SHELLNOTIFY, &Cobserve_ime_wnd::OnPmShellnotify)
	ON_MESSAGE(IDM_EXIT_APP, &Cobserve_ime_wnd::OnIdmExitApp)
	ON_WM_POWERBROADCAST()
	ON_REGISTERED_MESSAGE(m_uTaskbarRestart, &OnTaskbarRestart)
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
	m_uTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));
	return CreateEx(0, lpcClass, AfxGetAppName(), WS_POPUP, CRect(0, 0, 0, 0), NULL, NULL);
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

	tip_working = _T("observe_ime ") + theApp.version + _T(" working.");
	tip_error = _T("observe_ime ") + theApp.version + _T(" NO hoboNicola.");
	m_enabled = true;

	// 通知方式の読み込み
	int method = theApp.GetProfileInt(_T("settings"), _T("notifyMethod"), METHOD_LOCKKEY);
	m_notifyMethod = (notify_method_t)method;

	if (m_notifyMethod == METHOD_MSC) {
		if (!hoboMSC.find_msc_drive() || !hoboMSC.open()) {
			m_enabled = false;
			m_msc_timer = SetTimer(2, msc_search_interval, NULL);
		}
	} else if (m_notifyMethod == METHOD_HID) {
		// DeviceHistoryから設定を読み込む
		CDeviceHistory deviceHistory;
		deviceHistory.LoadFromRegistry();
		const DeviceInfo& lastDevice = deviceHistory.GetLastUsed();

		if (lastDevice.IsValid()) {
			hoboHID.setDeviceAttributes(lastDevice.vid, lastDevice.pid,
				deviceHistory.GetUsagePage(), deviceHistory.GetUsage());
			if (!hoboHID.find_hid_device() || !hoboHID.open()) {
				m_enabled = false;
				m_msc_timer = SetTimer(2, msc_search_interval, NULL);
			}
		} else {
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

	CheckMenuItem(m_trayMenu, IDM_OBSERVING, MF_BYCOMMAND | (m_enabled ? MF_CHECKED : MF_UNCHECKED));

	// 通知方式のチェック
	CheckMenuItem(m_trayMenu, IDM_LOCKKEY_NOTIFY, MF_BYCOMMAND | ((m_notifyMethod == METHOD_LOCKKEY) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(m_trayMenu, IDM_MSC_NOTIFY, MF_BYCOMMAND | ((m_notifyMethod == METHOD_MSC) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(m_trayMenu, IDM_HID_NOTIFY, MF_BYCOMMAND | ((m_notifyMethod == METHOD_HID) ? MF_CHECKED : MF_UNCHECKED));

	// ScrLock/NumLock の有効/無効とチェック
	UINT enableFlag = (m_notifyMethod == METHOD_LOCKKEY) ? MF_ENABLED : MF_GRAYED;
	EnableMenuItem(m_trayMenu, IDM_VKSCROLL, MF_BYCOMMAND | enableFlag);
	EnableMenuItem(m_trayMenu, IDM_VKNUMLOCK, MF_BYCOMMAND | enableFlag);

	CheckMenuItem(m_trayMenu, IDM_VKSCROLL, MF_BYCOMMAND | ((m_keycode == VK_SCROLL) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(m_trayMenu, IDM_VKNUMLOCK, MF_BYCOMMAND | ((m_keycode == VK_NUMLOCK) ? MF_CHECKED : MF_UNCHECKED));
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
		} else {
			m_timer_id = SetTimer(1, m_interval, NULL);
			m_enabled = true;
			ChangeNotifyIcon(false);
		}
	} else if (wParam == IDM_LOCKKEY_NOTIFY) {
		if (m_notifyMethod != METHOD_LOCKKEY) {
			CloseCurrentNotifyDevice();
			m_notifyMethod = METHOD_LOCKKEY;
			theApp.WriteProfileInt(_T("settings"), _T("notifyMethod"), METHOD_LOCKKEY);
		}
	} else if (wParam == IDM_MSC_NOTIFY) {
		if (m_notifyMethod != METHOD_MSC) {
			CloseCurrentNotifyDevice();
			if (hoboMSC.find_msc_drive() && hoboMSC.open()) {
				m_notifyMethod = METHOD_MSC;
				theApp.WriteProfileInt(_T("settings"), _T("notifyMethod"), METHOD_MSC);
			} else {
				AfxMessageBox(_T("hoboNicola not found."));
			}
		}
	} else if (wParam == IDM_HID_NOTIFY) {
		if (m_notifyMethod != METHOD_HID) {
			CloseCurrentNotifyDevice();

			// DeviceHistoryから設定を読み込む
			CDeviceHistory deviceHistory;
			deviceHistory.LoadFromRegistry();
			const DeviceInfo& lastDevice = deviceHistory.GetLastUsed();

			if (!lastDevice.IsValid()) {
				AfxMessageBox(_T("HID device settings not configured.\nPlease configure VID/PID first."));
			} else {
				hoboHID.setDeviceAttributes(lastDevice.vid, lastDevice.pid,
					deviceHistory.GetUsagePage(), deviceHistory.GetUsage());
				if (hoboHID.find_hid_device() && hoboHID.open()) {
					m_notifyMethod = METHOD_HID;
					theApp.WriteProfileInt(_T("settings"), _T("notifyMethod"), METHOD_HID);
				} else {
					AfxMessageBox(_T("hoboNicola HID device not found."));
				}
			}
		}
	} else if (wParam == IDM_HID_SETTINGS) {
		CHIDSettingsDlg dlg;

		// DeviceHistoryから最後の設定を読み込む
		CDeviceHistory deviceHistory;
		deviceHistory.LoadFromRegistry();
		const DeviceInfo& lastDevice = deviceHistory.GetLastUsed();

		if (lastDevice.IsValid()) {
			dlg.m_vid = lastDevice.vid;
			dlg.m_pid = lastDevice.pid;
			dlg.m_usagePage = deviceHistory.GetUsagePage();
			dlg.m_usage = deviceHistory.GetUsage();
		} else {
			// 履歴がない場合はデフォルト値
			dlg.m_vid = 0;
			dlg.m_pid = 0;
			dlg.m_usagePage = 0xFF00;
			dlg.m_usage = 0x0001;
		}

		if (dlg.DoModal() == IDOK) {
			// ダイアログ内でDeviceHistoryに保存済み

			// HID通知モードの場合は再接続
			if (m_notifyMethod == METHOD_HID) {
				hoboHID.resetDevice();
				hoboHID.setDeviceAttributes(dlg.m_vid, dlg.m_pid, dlg.m_usagePage, dlg.m_usage);
				// デバイス情報をトレース出力
				TRACE(_T("HID Reconnection attempt - VID: 0x%04X, PID: 0x%04X, UsagePage: 0x%04X, Usage: 0x%04X\n"),
					dlg.m_vid, dlg.m_pid, dlg.m_usagePage, dlg.m_usage);

				if (hoboHID.find_hid_device() && hoboHID.open()) {
					AfxMessageBox(_T("HID device reconnected successfully."));
				} else {
					AfxMessageBox(_T("HID device not found with new settings.\nNotification disabled."));
					m_enabled = false;
					if (m_timer_id) {
						KillTimer(m_timer_id);
						m_timer_id = 0;
					}
					m_msc_timer = SetTimer(2, msc_search_interval, NULL);
					ChangeNotifyIcon(true);
				}
			}
		}
	} else if (wParam == IDM_VKNUMLOCK) {
		m_keycode = VK_NUMLOCK;
	} else if (wParam == IDM_VKSCROLL) {
		m_keycode = VK_SCROLL;
	}

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

void Cobserve_ime_wnd::notify_keyboard(bool kana) {
	if (!m_enabled)
		return;

	switch (m_notifyMethod) {
	case METHOD_LOCKKEY:
		send_keycode(m_keycode, kana ? on : off);
		break;

	case METHOD_MSC:
		if (!hoboMSC.msc_notify(kana)) {
			m_enabled = false;
			KillTimer(m_timer_id);
			m_timer_id = 0;
			MessageBeep(MB_ICONHAND);
			TRACE(_T("notify_keyboard(msc) failed.\n"));
			ChangeNotifyIcon(true);
			m_msc_timer = SetTimer(2, msc_search_interval, NULL);
		}
		break;

	case METHOD_HID:
		if (!hoboHID.hid_notify(kana)) {
			m_enabled = false;
			KillTimer(m_timer_id);
			m_timer_id = 0;
			MessageBeep(MB_ICONHAND);
			TRACE(_T("notify_keyboard(hid) failed.\n"));
			ChangeNotifyIcon(true);
			m_msc_timer = SetTimer(2, msc_search_interval, NULL);
		}
		break;
	}
}

#ifndef IMC_GETCONVERSIONMODE 
#define IMC_GETCONVERSIONMODE 1
#endif
#ifndef IMC_GETOPENSTATUS 
#define IMC_GETOPENSTATUS 5
#endif
#ifndef IME_CMODE_NATIVE
#define IME_CMODE_NATIVE    0x0001
#endif
#ifndef IME_CMODE_KATAKANA
#define IME_CMODE_KATAKANA  0x0002
#endif

void Cobserve_ime_wnd::OnTimer(UINT_PTR id) {
	CWnd::OnTimer(id);
	if (id == m_timer_id) {
		bool status = false;
		static bool last_status = false;

		HWND hwndTarget = NULL;
		DWORD currentThreadId = 0;

		GUITHREADINFO gti = { 0 };
		gti.cbSize = sizeof(GUITHREADINFO);

		if (GetGUIThreadInfo(NULL, &gti) && IsWindow(gti.hwndFocus)) {
			hwndTarget = gti.hwndFocus;
			currentThreadId = GetWindowThreadProcessId(hwndTarget, NULL);
		} else {
			hwndTarget = ::GetForegroundWindow();
			if (IsWindow(hwndTarget)) {
				currentThreadId = GetWindowThreadProcessId(hwndTarget, NULL);
			}
		}

		if (!IsWindow(hwndTarget)) {
			status = false;
			TRACE(_T("No valid hwndTarget, set status to false\n"));
		} else {
			if (m_lastThreadId != 0 && m_lastThreadId != currentThreadId) {
				TRACE(_T("Thread changed: %08x -> %08x\n"), m_lastThreadId, currentThreadId);
			}

			HWND hwndime = ImmGetDefaultIMEWnd(hwndTarget);
			if (!IsWindow(hwndime)) {
				TRACE(_T("ImmGetDefaultIMEWnd() failed\n"));
				status = false;
			} else {
				if (::SendMessage(hwndime, WM_IME_CONTROL, IMC_GETOPENSTATUS, 0) == 1)
					status = (::SendMessage(hwndime, WM_IME_CONTROL, IMC_GETCONVERSIONMODE, 0) & 1) != 0;
				else
					status = false;
			}

			m_lastThreadId = currentThreadId;
		}

		if (status != last_status) {
			last_status = status;
			notify_keyboard(status);
		}
	} else if (id == m_msc_timer) {
		bool reconnected = false;

		switch (m_notifyMethod) {
		case METHOD_MSC:
			reconnected = hoboMSC.find_msc_drive() && hoboMSC.open();
			break;
		case METHOD_HID:
			// 最新の設定で再接続を試みる
		{
			CDeviceHistory deviceHistory;
			deviceHistory.LoadFromRegistry();
			const DeviceInfo& lastDevice = deviceHistory.GetLastUsed();

			if (lastDevice.IsValid()) {
				hoboHID.setDeviceAttributes(lastDevice.vid, lastDevice.pid,
					deviceHistory.GetUsagePage(), deviceHistory.GetUsage());
				reconnected = hoboHID.find_hid_device() && hoboHID.open();
			}
		}
		break;
		}

		if (reconnected) {
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

void Cobserve_ime_wnd::InitNotifyIconData(NOTIFYICONDATA& nic, bool error) {
	memset(&nic, 0, sizeof(NOTIFYICONDATA));
	nic.cbSize = sizeof(NOTIFYICONDATA);
	nic.hWnd = m_hWnd;
	nic.uID = IDI_ICON2;
	nic.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nic.uCallbackMessage = PM_SHELLNOTIFY;
	nic.hIcon = error ? m_trayIcon2 : m_trayIcon;
	_tcsncpy_s(nic.szTip, error ? tip_error : tip_working, sizeof(nic.szTip) / sizeof(TCHAR));
}

void Cobserve_ime_wnd::CloseCurrentNotifyDevice() {
	switch (m_notifyMethod) {
	case METHOD_MSC:
		hoboMSC.close();
		break;
	case METHOD_HID:
		hoboHID.close();
		break;
	case METHOD_LOCKKEY:
		break;
	}
}

void Cobserve_ime_wnd::SetNotifyIcon(bool error) {
	m_fErrorNotify = error;
	NOTIFYICONDATA nic;
	InitNotifyIconData(nic, error);
	m_fTrayIcon = Shell_NotifyIcon(NIM_ADD, &nic);
}

void Cobserve_ime_wnd::ChangeNotifyIcon(bool error) {
	m_fErrorNotify = error;
	NOTIFYICONDATA nic;
	InitNotifyIconData(nic, error);
	m_fTrayIcon = Shell_NotifyIcon(NIM_MODIFY, &nic);
}

void Cobserve_ime_wnd::DeleteNotifyIcon() {
	NOTIFYICONDATA	nic;
	memset(&nic, 0, sizeof(NOTIFYICONDATA));
	nic.cbSize = sizeof(NOTIFYICONDATA);
	nic.hWnd = m_hWnd;
	nic.uID = IDI_ICON2;
	Shell_NotifyIcon(NIM_DELETE, &nic);
	m_fTrayIcon = false;
}

afx_msg LRESULT Cobserve_ime_wnd::OnTaskbarRestart(WPARAM wp, LPARAM lp) {
	if (m_fTrayIcon)
		DeleteNotifyIcon();
	SetNotifyIcon(m_fErrorNotify);
	return 1;
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
				AppendMenu(m_trayMenu, MF_STRING, IDM_LOCKKEY_NOTIFY, _T("Use LockKey"));
				AppendMenu(m_trayMenu, MF_STRING, IDM_VKSCROLL, _T("  ScrLock"));
				AppendMenu(m_trayMenu, MF_STRING, IDM_VKNUMLOCK, _T("  NumLock"));
				AppendMenu(m_trayMenu, MF_SEPARATOR, 0, NULL);
				AppendMenu(m_trayMenu, MF_STRING, IDM_MSC_NOTIFY, _T("Use MSC"));
				AppendMenu(m_trayMenu, MF_STRING, IDM_HID_NOTIFY, _T("Use HID"));
				AppendMenu(m_trayMenu, MF_STRING, IDM_HID_SETTINGS, _T("HID Settings..."));
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
	} else if (nPowerEvent == PBT_APMRESUMEAUTOMATIC || nPowerEvent == PBT_APMRESUMESUSPEND) {
		if (m_enabled && m_timer_id == 0)
			m_timer_id = SetTimer(1, m_interval, NULL);
	}

	return CWnd::OnPowerBroadcast(nPowerEvent, nEventData);
}


