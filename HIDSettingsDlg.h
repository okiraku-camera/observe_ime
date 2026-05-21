// HIDSettingsDlg.h - HID device settings dialog
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#pragma once
#include "afxdialogex.h"
#include "resource.h"
#include "DeviceHistory.h"

class CHIDSettingsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CHIDSettingsDlg)

public:
	CHIDSettingsDlg(CWnd* pParent = nullptr);
	virtual ~CHIDSettingsDlg();

	enum { IDD = IDD_HID_SETTINGS };

	USHORT m_vid;
	USHORT m_pid;
	USHORT m_usagePage;
	USHORT m_usage;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()

private:
	CString m_strVID;
	CString m_strPID;
	CString m_strUsagePage;
	CString m_strUsage;
	
	CComboBox m_deviceCombo;
	CButton m_deleteButton;
	CDeviceHistory m_deviceHistory;
	
	void UpdateDeviceList();
	void LoadDeviceFromList(int index);
	void UpdateDeleteButtonState();

public:
	afx_msg void OnCbnSelChangeDeviceCombo();
	afx_msg void OnBnClickedDeleteHistory();
};