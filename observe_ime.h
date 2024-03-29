// observe_ime.h Copyright (c) 2018 Takeshi Higasa, okiraku-camera.tokyo
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#pragma once
#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif
#include "resource.h"       // メイン シンボル


class CobserveimeApp : public CWinApp
{
	void readVersionInfo();

public:
	CobserveimeApp() noexcept;
	CString version;

// オーバーライド
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
public:
	DECLARE_MESSAGE_MAP()
};

extern CobserveimeApp theApp;
