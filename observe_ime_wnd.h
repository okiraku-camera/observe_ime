// observe_ime_wnd.h Copyright (c) 2018 okiraku-camera.tokyo
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#pragma once
#include <afxwin.h>
#include "DeviceHistory.h"

#define TRAY_NOTIFY_ID		WM_USER + 2001
#define PM_SHELLNOTIFY		WM_USER + 2002		// Shell notification 

typedef enum { toggle = 1, on = 2, off = 3 } key_action_t;

// ’Ê’m•ûŽ®
typedef enum {
	METHOD_LOCKKEY = 0,  // ScrollLock/NumLock
	METHOD_MSC = 1,      // Mass Storage Class
	METHOD_HID = 2       // USB HID
} notify_method_t;

class Cobserve_ime_wnd : public CWnd
{
	UINT_PTR m_timer_id;
	UINT_PTR m_msc_timer;
	int m_interval;
	DWORD m_lastThreadId;

	bool m_enabled;
	HICON m_trayIcon;
	HICON m_trayIcon2;
	HMENU m_trayMenu;

	notify_method_t m_notifyMethod;
	static UINT m_uTaskbarRestart;
	bool m_fTrayIcon;
	bool m_fErrorNotify;

public:
	Cobserve_ime_wnd();
	virtual ~Cobserve_ime_wnd() {}
	bool Create();

	int m_keycode;

protected:
	virtual void PostNcDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnPmShellnotify(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnIdmExitApp(WPARAM wParam, LPARAM lParam);
	afx_msg UINT OnPowerBroadcast(UINT nPowerEvent, LPARAM nEventData);
	afx_msg LRESULT OnTaskbarRestart(WPARAM wp, LPARAM lp);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	void send_keycode(WORD vkey, key_action_t action = toggle);
	void notify_keyboard(bool kana);
	void SetNotifyIcon(bool error = false);
	void ChangeNotifyIcon(bool error);
	void DeleteNotifyIcon();
	void SetMenuChecked();
	void InitNotifyIconData(NOTIFYICONDATA& nic, bool error);
	void CloseCurrentNotifyDevice();
};