// hoboMSC.h Copyright (c) 2022 Takeshi Higasa, okiraku-camera.tokyo
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//
// The USB mass storage class (MSC) support.
//
#pragma once

class ChoboMSC {
	CString hoboNicola_path;
	CFile* pFile;
public:
	ChoboMSC() {
		pFile = 0;
	}
	virtual ~ChoboMSC() {
		if (pFile) delete pFile;
	}
	void close();
	bool open(); 
	bool find_msc_drive();
	bool msc_notify(bool kana);
protected:
};
