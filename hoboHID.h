// hoboHID.h Copyright (c) 2026 Takeshi Higasa, okiraku-camera.tokyo
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//
// The USB HID support for hoboNicola notification.
//
#pragma once

extern "C" {
#include <windows.h>
#include <setupapi.h>
#include <hidsdi.h>
}

class ChoboHID {
	HANDLE hDevice;
	CString devicePath;
	USHORT vid;
	USHORT pid;
	USHORT usagePage;
	USHORT usage;
	bool deviceFound;
	HIDP_CAPS caps;
public:
	ChoboHID() {
		hDevice = INVALID_HANDLE_VALUE;
		vid = 0;
		pid = 0;
		usagePage = 0;
		usage = 0;
		deviceFound = false;
		memset(&caps, 0, sizeof(caps));
	}
	virtual ~ChoboHID() {
		close();
	}
	void close();
	void setDeviceAttributes(USHORT vid, USHORT pid, USHORT usagePage, USHORT usage);
	bool find_hid_device();
	bool open();
	bool hid_notify(bool kana);
	bool isOpen() const { return hDevice != INVALID_HANDLE_VALUE; }
	bool isDeviceFound() const { return deviceFound; }
	void resetDevice();
protected:
	bool checkDeviceAttributes(HANDLE hDev);
};