// hoboHID.cpp Copyright (c) Takeshi Higasa, okiraku-camera.tokyo
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//
// The USB HID support for hoboNicola notification.
//

#include "stdafx.h"
#include "hoboHID.h"

#pragma comment(lib, "hid.lib")
#pragma comment(lib, "setupapi.lib")

void ChoboHID::setDeviceAttributes(USHORT _vid, USHORT _pid, USHORT _usagePage, USHORT _usage) {
	vid = _vid;
	pid = _pid;
	usagePage = _usagePage;
	usage = _usage;
}

bool ChoboHID::checkDeviceAttributes(HANDLE hDev) {
	HIDD_ATTRIBUTES attrib;
	attrib.Size = sizeof(HIDD_ATTRIBUTES);
	if (!HidD_GetAttributes(hDev, &attrib)) {
		return false;
	}
	
	TRACE(_T("HID Device: VID=%04X, PID=%04X\n"), attrib.VendorID, attrib.ProductID);
	
	// VID/PIDチェック
	if (attrib.VendorID != vid || attrib.ProductID != pid) {
		return false;
	}
	
	// UsagePage/Usageチェック
	PHIDP_PREPARSED_DATA preparsedData;
	if (!HidD_GetPreparsedData(hDev, &preparsedData)) {
		return false;
	}
	
	HIDP_CAPS caps;
	NTSTATUS status = HidP_GetCaps(preparsedData, &caps);
	HidD_FreePreparsedData(preparsedData);
	
	if (status != HIDP_STATUS_SUCCESS) {
		return false;
	}
	
	TRACE(_T("  UsagePage=%04X, Usage=%04X\n"), caps.UsagePage, caps.Usage);
	
	if (caps.UsagePage != usagePage || caps.Usage != usage) {
		return false;
	}
	
	return true;
}

bool ChoboHID::find_hid_device() {
	// 既に見つかっている場合はスキップ
	if (deviceFound && !devicePath.IsEmpty()) {
		return true;
	}

	if (vid == 0 || pid == 0) {
		TRACE(_T("HID device attributes not set\n"));
		return false;
	}

	GUID hidGuid;
	HidD_GetHidGuid(&hidGuid);

	HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&hidGuid, NULL, NULL,
		DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (deviceInfoSet == INVALID_HANDLE_VALUE) {
		return false;
	}

	SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
	deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	CString vidPidPattern;
	vidPidPattern.Format(_T("vid_%04x&pid_%04x"), vid, pid);
	vidPidPattern.MakeLower();

	bool found = false;
	for (DWORD i = 0; SetupDiEnumDeviceInterfaces(deviceInfoSet, NULL, &hidGuid, i, &deviceInterfaceData); i++) {
		DWORD requiredSize = 0;
		SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, NULL, 0, &requiredSize, NULL);

		PSP_DEVICE_INTERFACE_DETAIL_DATA detailData =
			(PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(requiredSize);
		if (!detailData)
			continue;

		detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		if (SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData,
			detailData, requiredSize, NULL, NULL)) {

			CString path(detailData->DevicePath);
			path.MakeLower();

			if (path.Find(vidPidPattern) != -1) {
				HANDLE hDev = CreateFile(detailData->DevicePath,
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL, OPEN_EXISTING, 0, NULL);

				if (hDev != INVALID_HANDLE_VALUE) {
					if (checkDeviceAttributes(hDev)) {
						devicePath = detailData->DevicePath;
						deviceFound = true;
						TRACE(_T("Found hoboNicola HID device: %s\n"), (LPCTSTR)devicePath);
						found = true;
						CloseHandle(hDev);
						free(detailData);
						break;
					}
					CloseHandle(hDev);
				}
			}
		}
		free(detailData);
	}

	SetupDiDestroyDeviceInfoList(deviceInfoSet);
	return found;
}

bool ChoboHID::open() {
	if (devicePath.IsEmpty())
		return false;
	
	hDevice = CreateFile(devicePath,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, 0, NULL);
	
	if (hDevice == INVALID_HANDLE_VALUE) {
		TRACE(_T("Failed to open HID device\n"));
		return false;
	}
	
	// デバイスのキャパビリティを取得（一度だけ）
	PHIDP_PREPARSED_DATA preparsedData;
	if (!HidD_GetPreparsedData(hDevice, &preparsedData)) {
		TRACE(_T("Failed to get preparsed data\n"));
		CloseHandle(hDevice);
		hDevice = INVALID_HANDLE_VALUE;
		return false;
	}
	
	NTSTATUS status = HidP_GetCaps(preparsedData, &caps);
	HidD_FreePreparsedData(preparsedData);
	
	if (status != HIDP_STATUS_SUCCESS) {
		TRACE(_T("Failed to get HID caps\n"));
		CloseHandle(hDevice);
		hDevice = INVALID_HANDLE_VALUE;
		return false;
	}
	
	TRACE(_T("HID device opened successfully\n"));
	TRACE(_T("  OutputReportLength=%d, InputReportLength=%d\n"), 
		caps.OutputReportByteLength, caps.InputReportByteLength);
	return true;
}

void ChoboHID::close() {
	if (hDevice != INVALID_HANDLE_VALUE) {
		CloseHandle(hDevice);
		hDevice = INVALID_HANDLE_VALUE;
	}
	memset(&caps, 0, sizeof(caps));
}

void ChoboHID::resetDevice() {
	close();
	deviceFound = false;
	devicePath.Empty();
	TRACE(_T("HID device reset - will search again on next attempt\n"));
}

#define REPORT_ID_HOBO_NICOLA	0x05
#define SET_NICOLA_MODE	0x30
#define GET_NICOLA_MODE	0x31

bool ChoboHID::hid_notify(bool kana) {
	if (hDevice == INVALID_HANDLE_VALUE || caps.OutputReportByteLength == 0)
		return false;

	USHORT reportLength = caps.OutputReportByteLength;

	// レポートバッファを確保（ゼロ初期化）
	BYTE* report = (BYTE*)calloc(reportLength, 1);
	if (!report)
		return false;

	// Report ID と データをセット
	report[0] = REPORT_ID_HOBO_NICOLA;
	if (reportLength >= 3) {
		report[1] = SET_NICOLA_MODE;
		report[2] = kana ? 1 : 0;
	}

	// HidD_SetOutputReport で送信（WriteFile より確実）
	BOOL result = HidD_SetOutputReport(hDevice, report, reportLength);

	if (!result) {
		DWORD err = GetLastError();
		TRACE(_T("HID notify failed, error=%d\n"), err);
		if (err == ERROR_BAD_COMMAND || err == ERROR_DEVICE_NOT_CONNECTED) {
			resetDevice();
		}
		free(report);
		return false;
	}

	TRACE(_T("HID notify success: ReportID=%02X, Cmd=%02X, Data=%02X\n"),
		REPORT_ID_HOBO_NICOLA, SET_NICOLA_MODE, kana ? 1 : 0);
	free(report);
	return true;
}
