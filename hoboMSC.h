#pragma once

class ChoboMSC {
	CString hoboNicola_path;
	CFile* pFile;
public:
	ChoboMSC() {
		pFile = 0;
	}
	virtual ~ChoboMSC() {
		if (pFile)
			delete pFile;
	}

	void close();	// �t�@�C�������B
	bool open();// �t�@�C�����J��

	bool find_msc_drive();

	bool msc_notify(bool kana);

protected:
};
