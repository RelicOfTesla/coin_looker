
// btc_looker.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// Cbtc_lookerApp:
// �йش����ʵ�֣������ btc_looker.cpp
//

class Cbtc_lookerApp : public CWinApp
{
public:
	Cbtc_lookerApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern Cbtc_lookerApp theApp;