
// coin_looker.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// Ccoin_lookerApp:
// �йش����ʵ�֣������ coin_looker.cpp
//

class Ccoin_lookerApp : public CWinApp
{
public:
	Ccoin_lookerApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern Ccoin_lookerApp theApp;