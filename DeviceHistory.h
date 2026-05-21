// DeviceHistory.h
#pragma once

struct DeviceInfo {
	USHORT vid;
	USHORT pid;
	CString description;

	DeviceInfo() : vid(0), pid(0) {}
	DeviceInfo(USHORT v, USHORT p) : vid(v), pid(p) {
		description.Format(_T("VID:%04X PID:%04X"), vid, pid);
	}

	bool operator==(const DeviceInfo& other) const {
		return vid == other.vid && pid == other.pid;
	}

	bool IsValid() const {
		return vid != 0 && pid != 0;
	}
};

class CDeviceHistory {
	static const int MAX_HISTORY = 5;
	CArray<DeviceInfo, DeviceInfo&> history;
	int lastUsedIndex;
	CString registryKey;
	USHORT usagePage;
	USHORT usage;

public:
	CDeviceHistory();
	~CDeviceHistory();

	void LoadFromRegistry();
	void SaveToRegistry();
	
	void SetUsagePageAndUsage(USHORT up, USHORT u) { usagePage = up; usage = u; }
	USHORT GetUsagePage() const { return usagePage; }
	USHORT GetUsage() const { return usage; }
	
	void AddDevice(const DeviceInfo& device);
	void AddDevice(USHORT vid, USHORT pid);
	
	int GetCount() const { return (int)history.GetCount(); }
	const DeviceInfo& GetDevice(int index) const { return history[index]; }
	const DeviceInfo& GetLastUsed() const;
	
	void SetLastUsed(int index);
	int GetLastUsedIndex() const { return lastUsedIndex; }
	
	void RemoveDevice(int index);  // ’Ç‰Á
	void Clear();
};