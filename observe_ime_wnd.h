// observe_ime_wnd.h Copyright (c) 2018 okiraku-camera.tokyo
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#pragma once
#include <afxwin.h>
#define TRAY_NOTIFY_ID		WM_USER + 2001
#define PM_SHELLNOTIFY		WM_USER + 2002		// Shell notification 

typedef enum { toggle = 1, on = 2, off = 3 } key_action_t;


class Cobserve_ime_wnd : public CWnd
{
	UINT_PTR m_timer_id;
	UINT_PTR m_msc_timer;
	int m_interval;

	bool m_enabled;
	HICON m_trayIcon;
	HICON m_trayIcon2;
	HMENU m_trayMenu;

	bool m_useMSC;
	static UINT m_uTaskbarRestart;
	bool m_fTrayIcon;
	bool m_fErrorNotify;

public:
	Cobserve_ime_wnd();
	virtual ~Cobserve_ime_wnd() {}
	bool Create();

	int m_keycode;

	void SetNotifyIcon(bool error = false);
	void DeleteNotifyIcon();
	void ChangeNotifyIcon(bool error);


	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

protected:
	virtual void PostNcDestroy();
	afx_msg LRESULT OnPmShellnotify(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnIdmExitApp(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT  OnTaskbarRestart(WPARAM w, LPARAM l);

	void send_keycode(WORD vkey, key_action_t action);
	void SetMenuChecked();

	void notify_keyboard(bool kana);	// NICOLAモードのオン・オフ通知


public:
	afx_msg UINT OnPowerBroadcast(UINT nPowerEvent, LPARAM nEventData);
};

