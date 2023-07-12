// observe_ime.h Copyright (c) 2018 Takeshi Higasa, okiraku-camera.tokyo
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//
// The USB mass storage class (MSC) support.
//

#include "stdafx.h"
#include "hoboMSC.h"

static const TCHAR* hoboNiocola_filename = _T("HOBONICO.BIN");
static const int volume_label_buffer_length = 12;
typedef struct {
	DWORD sectorsPerCluster;
	DWORD bytesPerSector;
	DWORD numberOfFreeClusters; // この項目を評価するのはやめる。
	DWORD totalNumberOfClusters;
	DWORD volumeSerial;
	TCHAR volumeLabel[volume_label_buffer_length];
} drive_info_t;

#if _DEBUG
CString drive_type_string(UINT n) {
	CString cs;
	switch (n) {
	case DRIVE_NO_ROOT_DIR:	// 1
		return  _T("DRIVE_NO_ROOT_DIR");
	case DRIVE_REMOVABLE:	// 2
		return _T("DRIVE_REMOVABLE");
	case DRIVE_FIXED:	// 3
		return _T("DRIVE_FIXED");
	case DRIVE_REMOTE:	// 4
		return _T("DRIVE_REMOTE");
	case DRIVE_CDROM:	// 5
		return _T("DRIVE_CDROM");
	case DRIVE_RAMDISK:	// 6
		return _T("DRIVE_RAMDISK");
	case DRIVE_UNKNOWN:	// 0
	default:
		return _T("DRIVE_UNKNOWN");
	}
	return _T("");
}

static void dump_disk_info(drive_info_t* p) {
	TRACE(_T("label=%s, serial=%08x, sectorsPerCluster=%d, bytesPerSector=%d, numberOfFreeClusters=%d,totalNumberOfClusters=%d\n"),
		p->volumeLabel, p->volumeSerial,
		p->sectorsPerCluster, p->bytesPerSector, p->numberOfFreeClusters, p->totalNumberOfClusters);
}
#endif // _DEBUG

static const drive_info_t hobo_msc = { 1, 512, 9, 13, 0x23456789, _T("hoboNicola") };

static bool is_hoboNicola_msc(drive_info_t* p) {
	if (p && p->bytesPerSector == hobo_msc.bytesPerSector &&
		//			p->numberOfFreeClusters == hobo_msc.numberOfFreeClusters && // free clustersは変化する。
		p->sectorsPerCluster == hobo_msc.sectorsPerCluster &&
		p->totalNumberOfClusters == hobo_msc.totalNumberOfClusters &&
		p->volumeSerial == hobo_msc.volumeSerial &&
		_tcsnicmp(p->volumeLabel, hobo_msc.volumeLabel, volume_label_buffer_length) == 0)
		return true;
	return false;
}


bool ChoboMSC::find_msc_drive() {
	DWORD dw = GetLogicalDrives();
	TRACE(_T("GetLogicalDrives returns %08x\n"), dw);
	// list available drives.
	DWORD mask = 1;
	TCHAR drive = _T('A');
	TCHAR path[6] = _T("A:\\");
	UINT errorMode = GetErrorMode();
	if ((errorMode & SEM_FAILCRITICALERRORS) == 0)
		SetErrorMode(errorMode | SEM_FAILCRITICALERRORS);
	TRACE(_T("errorMode = %08x\n"), errorMode);
	for (int i = 0; i < sizeof(DWORD) << 3; i++) {
		drive_info_t info;
		if (dw & mask) {
			path[0] = drive;
			UINT type = GetDriveType(path);
#if _DEBUG
			TRACE(_T("%s : %s\n"), path, drive_type_string(type));
#endif
			if (type == DRIVE_REMOVABLE) {
				TCHAR label[MAX_PATH + 1] = _T("");
				if (GetVolumeInformation(path, label, sizeof(label), &info.volumeSerial, NULL, NULL, NULL, 0)) {
					_tcsncpy_s(info.volumeLabel, volume_label_buffer_length, label, volume_label_buffer_length);
					if (GetDiskFreeSpace(path, &info.sectorsPerCluster, &info.bytesPerSector, &info.numberOfFreeClusters, &info.totalNumberOfClusters)) {
#if _DEBUG
						dump_disk_info(&info);
#endif
						if (is_hoboNicola_msc(&info)) {
							LPTSTR lp = hoboNicola_path.GetBuffer(_MAX_PATH + 1);
							_tcsncpy_s(lp, _MAX_PATH, path, sizeof(path));
							_tcscat_s(lp, _MAX_PATH, hoboNiocola_filename);
							hoboNicola_path.ReleaseBuffer();
							return true;
						}
					}
				}
			}
		}
		mask = mask << 1;
		drive++;
	}
	SetErrorMode(errorMode);
	return false;
}

// ファイルを開く
bool ChoboMSC::open() {
	pFile = new CFile();
	if (!pFile->Open(hoboNicola_path, CFile::modeReadWrite | CFile::typeBinary | CFile::osWriteThrough | CFile::shareDenyWrite)) {
	delete pFile;
		pFile = 0;
		return false;
	}
	return true;
}

void ChoboMSC::close() {
	if (pFile) {
		pFile->Close();
		pFile = 0;
	}
}

bool ChoboMSC::msc_notify(bool kana) {
	char buffer[1];
	if (!pFile)
		return false;
	TRACE(_T("msc_notify %d\n"), kana);
	buffer[0] = kana ? 'H' : 'h';
	try {
		pFile->SeekToBegin();
		pFile->Write(&buffer, sizeof(buffer));
		pFile->Flush();
		return true;
	} catch (CFileException* pe) {
		TRACE(_T("msc_notify() CFileException  cause=%d, oserror=%d\n"), pe->m_cause, pe->m_lOsError);
//		pe->ReportError();
		pe->Delete();
		pFile->Close();
		delete pFile;
		pFile = 0;
	}
	return false;

}
