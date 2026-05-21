// HIDSettingsDlg.cpp - HID device settings dialog
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#include "stdafx.h"
#include "observe_ime.h"
#include "HIDSettingsDlg.h"
#include "hoboHID.h"

IMPLEMENT_DYNAMIC(CHIDSettingsDlg, CDialogEx)

CHIDSettingsDlg::CHIDSettingsDlg(CWnd* pParent)
	: CDialogEx(IDD_HID_SETTINGS, pParent)
	, m_vid(0)
	, m_pid(0)
	, m_usagePage(0xFF00)
	, m_usage(0x0001)
{
}

CHIDSettingsDlg::~CHIDSettingsDlg()
{
}

void CHIDSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_VID, m_strVID);
	DDX_Text(pDX, IDC_EDIT_PID, m_strPID);
	DDX_Text(pDX, IDC_EDIT_USAGE_PAGE, m_strUsagePage);
	DDX_Text(pDX, IDC_EDIT_USAGE, m_strUsage);
	DDX_Control(pDX, IDC_COMBO_DEVICE_HISTORY, m_deviceCombo);
	DDX_Control(pDX, IDC_BUTTON_DELETE_HISTORY, m_deleteButton);
}

BEGIN_MESSAGE_MAP(CHIDSettingsDlg, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBO_DEVICE_HISTORY, &CHIDSettingsDlg::OnCbnSelChangeDeviceCombo)
	ON_BN_CLICKED(IDC_BUTTON_DELETE_HISTORY, &CHIDSettingsDlg::OnBnClickedDeleteHistory)
END_MESSAGE_MAP()

BOOL CHIDSettingsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 履歴をロード
	m_deviceHistory.SetUsagePageAndUsage(m_usagePage, m_usage);
	m_deviceHistory.LoadFromRegistry();

	// コンボボックスに履歴を表示
	UpdateDeviceList();

	// 最後に使用したデバイスまたは渡された値を表示
	const DeviceInfo& lastDevice = m_deviceHistory.GetLastUsed();
	if (m_vid == 0 && m_pid == 0 && lastDevice.IsValid()) {
		// 渡された値がない場合は履歴から復元
		m_vid = lastDevice.vid;
		m_pid = lastDevice.pid;
		m_usagePage = m_deviceHistory.GetUsagePage();
		m_usage = m_deviceHistory.GetUsage();
	}

	// 現在のVID/PIDに一致する履歴項目を検索して選択
	int matchIndex = -1;
	for (int i = 0; i < m_deviceHistory.GetCount(); i++) {
		const DeviceInfo& device = m_deviceHistory.GetDevice(i);
		if (device.vid == m_vid && device.pid == m_pid) {
			matchIndex = i;
			m_deviceHistory.SetLastUsed(i);
			break;
		}
	}

	if (matchIndex >= 0) {
		m_deviceCombo.SetCurSel(matchIndex);
	} else {
		// 一致する履歴がない場合は "新しいデバイスを追加" を選択
		m_deviceCombo.SetCurSel(m_deviceHistory.GetCount());
	}

	// 16進数文字列に変換
	m_strVID.Format(_T("%04X"), m_vid);
	m_strPID.Format(_T("%04X"), m_pid);
	m_strUsagePage.Format(_T("%04X"), m_usagePage);
	m_strUsage.Format(_T("%04X"), m_usage);

	UpdateData(FALSE);
	UpdateDeleteButtonState();

	return TRUE;
}

void CHIDSettingsDlg::UpdateDeviceList()
{
	m_deviceCombo.ResetContent();
	
	for (int i = 0; i < m_deviceHistory.GetCount(); i++) {
		const DeviceInfo& device = m_deviceHistory.GetDevice(i);
		m_deviceCombo.AddString(device.description);
	}
	
	// "新しいデバイス" オプション
	m_deviceCombo.AddString(_T("--- 新しいデバイスを追加 ---"));
}

void CHIDSettingsDlg::LoadDeviceFromList(int index)
{
	if (index < 0 || index >= m_deviceHistory.GetCount())
		return;

	const DeviceInfo& device = m_deviceHistory.GetDevice(index);
	
	m_strVID.Format(_T("%04X"), device.vid);
	m_strPID.Format(_T("%04X"), device.pid);
	m_strUsagePage.Format(_T("%04X"), m_deviceHistory.GetUsagePage());
	m_strUsage.Format(_T("%04X"), m_deviceHistory.GetUsage());
	
	UpdateData(FALSE);
	
	m_deviceHistory.SetLastUsed(index);
}

void CHIDSettingsDlg::UpdateDeleteButtonState()
{
	int index = m_deviceCombo.GetCurSel();
	// 履歴項目が選択されている場合のみ削除ボタンを有効化
	BOOL enable = (index >= 0 && index < m_deviceHistory.GetCount());
	m_deleteButton.EnableWindow(enable);
}

void CHIDSettingsDlg::OnCbnSelChangeDeviceCombo()
{
	int index = m_deviceCombo.GetCurSel();
	if (index >= 0 && index < m_deviceHistory.GetCount()) {
		LoadDeviceFromList(index);
	}
	UpdateDeleteButtonState();
}

void CHIDSettingsDlg::OnBnClickedDeleteHistory()
{
	int index = m_deviceCombo.GetCurSel();
	if (index < 0 || index >= m_deviceHistory.GetCount())
		return;

	const DeviceInfo& device = m_deviceHistory.GetDevice(index);
	CString msg;
	msg.Format(_T("履歴から削除しますか？\n\n%s"), device.description);
	
	if (AfxMessageBox(msg, MB_YESNO | MB_ICONQUESTION) == IDYES) {
		// 履歴から削除
		m_deviceHistory.RemoveDevice(index);
		m_deviceHistory.SaveToRegistry();
		
		// リストを更新
		UpdateDeviceList();
		
		// 選択を更新
		if (m_deviceHistory.GetCount() > 0) {
			int newIndex = (index < m_deviceHistory.GetCount()) ? index : m_deviceHistory.GetCount() - 1;
			m_deviceCombo.SetCurSel(newIndex);
			LoadDeviceFromList(newIndex);
		} else {
			// 履歴が空になった場合は "新しいデバイスを追加" を選択
			m_deviceCombo.SetCurSel(0);
			m_strVID = _T("0000");
			m_strPID = _T("0000");
			UpdateData(FALSE);
		}
		
		UpdateDeleteButtonState();
	}
}

void CHIDSettingsDlg::OnOK()
{
	UpdateData(TRUE);

	// 16進数文字列を数値に変換
	UINT vid, pid, usagePage, usage;
	
	if (_stscanf_s(m_strVID, _T("%X"), &vid) != 1 || vid > 0xFFFF) {
		AfxMessageBox(_T("Invalid VID format. Use 4-digit hex (e.g., 0ABC)"), MB_ICONERROR);
		return;
	}

	if (_stscanf_s(m_strPID, _T("%X"), &pid) != 1 || pid > 0xFFFF) {
		AfxMessageBox(_T("Invalid PID format. Use 4-digit hex (e.g., 1234)"), MB_ICONERROR);
		return;
	}

	if (_stscanf_s(m_strUsagePage, _T("%X"), &usagePage) != 1 || usagePage > 0xFFFF) {
		AfxMessageBox(_T("Invalid Usage Page format. Use 4-digit hex (e.g., FF00)"), MB_ICONERROR);
		return;
	}

	if (_stscanf_s(m_strUsage, _T("%X"), &usage) != 1 || usage > 0xFFFF) {
		AfxMessageBox(_T("Invalid Usage format. Use 4-digit hex (e.g., 0001)"), MB_ICONERROR);
		return;
	}

	USHORT newVID = (USHORT)vid;
	USHORT newPID = (USHORT)pid;
	USHORT newUsagePage = (USHORT)usagePage;
	USHORT newUsage = (USHORT)usage;

	// デバイスへの接続テスト
	ChoboHID testDevice;
	testDevice.setDeviceAttributes(newVID, newPID, newUsagePage, newUsage);
	
	if (!testDevice.find_hid_device()) {
		CString msg;
		msg.Format(_T("指定されたHIDデバイスが見つかりません。\n\nVID: %04X\nPID: %04X\nUsagePage: %04X\nUsage: %04X"),
			newVID, newPID, newUsagePage, newUsage);
		AfxMessageBox(msg, MB_ICONWARNING);
		return;
	}

	if (!testDevice.open()) {
		CString msg;
		msg.Format(_T("HIDデバイスを開くことができませんでした。\n\nVID: %04X\nPID: %04X\n\nデバイスが他のアプリケーションで使用中の可能性があります。"),
			newVID, newPID);
		AfxMessageBox(msg, MB_ICONWARNING);
		return;
	}

	// 接続成功 - デバイスをクローズ
	testDevice.close();

	// 値を更新
	m_vid = newVID;
	m_pid = newPID;
	m_usagePage = newUsagePage;
	m_usage = newUsage;

	// 履歴に追加して保存
	m_deviceHistory.AddDevice(m_vid, m_pid);
	m_deviceHistory.SetUsagePageAndUsage(m_usagePage, m_usage);
	m_deviceHistory.SaveToRegistry();

	TRACE(_T("HID device verified and saved: VID:%04X PID:%04X\n"), m_vid, m_pid);

	CDialogEx::OnOK();
}

