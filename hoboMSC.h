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

	void close();	// ファイルを閉じる。
	bool open();// ファイルを開く

	bool find_msc_drive();

	bool msc_notify(bool kana);

protected:
};
