// DeviceHistory.cpp
#include "stdafx.h"
#include "DeviceHistory.h"

CDeviceHistory::CDeviceHistory() : lastUsedIndex(-1), usagePage(0x0001), usage(0x0006) {
	registryKey = _T("Software\\hoboNicola\\observe_ime");
}

CDeviceHistory::~CDeviceHistory() {
	SaveToRegistry();
}

void CDeviceHistory::LoadFromRegistry() {
	history.RemoveAll();
	lastUsedIndex = -1;

	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, registryKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		DWORD lastUsed = 0;
		DWORD size = sizeof(DWORD);
		RegQueryValueEx(hKey, _T("LastUsedDevice"), NULL, NULL, (LPBYTE)&lastUsed, &size);
		lastUsedIndex = (int)lastUsed;

		DWORD up = usagePage;
		size = sizeof(DWORD);
		if (RegQueryValueEx(hKey, _T("UsagePage"), NULL, NULL, (LPBYTE)&up, &size) == ERROR_SUCCESS) {
			usagePage = (USHORT)up;
		}
		
		DWORD u = usage;
		size = sizeof(DWORD);
		if (RegQueryValueEx(hKey, _T("Usage"), NULL, NULL, (LPBYTE)&u, &size) == ERROR_SUCCESS) {
			usage = (USHORT)u;
		}

		for (int i = 0; i < MAX_HISTORY; i++) {
			CString keyName;
			keyName.Format(_T("Device%d"), i);
			
			DWORD data[2] = {0};
			size = sizeof(data);
			
			if (RegQueryValueEx(hKey, keyName, NULL, NULL, (LPBYTE)data, &size) == ERROR_SUCCESS) {
				DeviceInfo device((USHORT)data[0], (USHORT)data[1]);
				if (device.IsValid()) {
					history.Add(device);
				}
			}
		}
		RegCloseKey(hKey);
	}
}

void CDeviceHistory::SaveToRegistry() {
	HKEY hKey;
	DWORD disposition;
	
	if (RegCreateKeyEx(HKEY_CURRENT_USER, registryKey, 0, NULL, 0, 
	                   KEY_WRITE, NULL, &hKey, &disposition) == ERROR_SUCCESS) {
		
		DWORD lastUsed = (DWORD)lastUsedIndex;
		RegSetValueEx(hKey, _T("LastUsedDevice"), 0, REG_DWORD, 
		              (LPBYTE)&lastUsed, sizeof(DWORD));

		DWORD up = usagePage;
		RegSetValueEx(hKey, _T("UsagePage"), 0, REG_DWORD, 
		              (LPBYTE)&up, sizeof(DWORD));
		
		DWORD u = usage;
		RegSetValueEx(hKey, _T("Usage"), 0, REG_DWORD, 
		              (LPBYTE)&u, sizeof(DWORD));

		// 現在の履歴を保存
		for (int i = 0; i < history.GetCount() && i < MAX_HISTORY; i++) {
			CString keyName;
			keyName.Format(_T("Device%d"), i);
			
			DWORD data[2] = {
				history[i].vid,
				history[i].pid
			};
			
			RegSetValueEx(hKey, keyName, 0, REG_BINARY, 
			              (LPBYTE)data, sizeof(data));
		}
		
		// 削除された履歴のエントリをレジストリから削除
		for (int i = (int)history.GetCount(); i < MAX_HISTORY; i++) {
			CString keyName;
			keyName.Format(_T("Device%d"), i);
			RegDeleteValue(hKey, keyName);
		}
		
		RegCloseKey(hKey);
	}
}

void CDeviceHistory::AddDevice(USHORT vid, USHORT pid) {
	AddDevice(DeviceInfo(vid, pid));
}

void CDeviceHistory::AddDevice(const DeviceInfo& device) {
	if (!device.IsValid())
		return;

	for (int i = 0; i < history.GetCount(); i++) {
		if (history[i] == device) {
			history.RemoveAt(i);
			break;
		}
	}

	// constを外してコピーしてから追加
	DeviceInfo deviceCopy = device;
	history.InsertAt(0, deviceCopy);
	lastUsedIndex = 0;

	while (history.GetCount() > MAX_HISTORY) {
		history.RemoveAt(history.GetCount() - 1);
	}
}

const DeviceInfo& CDeviceHistory::GetLastUsed() const {
	if (lastUsedIndex >= 0 && lastUsedIndex < history.GetCount()) {
		return history[lastUsedIndex];
	}
	static DeviceInfo empty;
	return empty;
}

void CDeviceHistory::SetLastUsed(int index) {
	if (index >= 0 && index < history.GetCount()) {
		lastUsedIndex = index;
	}
}

void CDeviceHistory::Clear() {
	history.RemoveAll();
	lastUsedIndex = -1;
}

void CDeviceHistory::RemoveDevice(int index) {
	if (index < 0 || index >= history.GetCount())
		return;

	history.RemoveAt(index);
	
	// lastUsedIndexを調整
	if (lastUsedIndex == index) {
		lastUsedIndex = (history.GetCount() > 0) ? 0 : -1;
	} else if (lastUsedIndex > index) {
		lastUsedIndex--;
	}
}