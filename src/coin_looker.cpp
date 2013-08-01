
// coin_looker.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "coin_looker.h"
#include "coin_lookerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void dbg_printf(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	CString str;
	for (int buflen = 512; ; buflen*=2)
	{
		char* buf = str.GetBuffer(buflen);
		if( wvnsprintf(buf, buflen, format, ap) < buflen-1)
		{
			str.ReleaseBuffer();
			OutputDebugString(buf);
			break;
		}
	}
	
}

// Ccoin_lookerApp

BEGIN_MESSAGE_MAP(Ccoin_lookerApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

// Ccoin_lookerApp ����
Ccoin_lookerApp::Ccoin_lookerApp()
{
#if _MFC_VER >= 0x0A00
	// ֧����������������
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
#endif
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� Ccoin_lookerApp ����

Ccoin_lookerApp theApp;


// Ccoin_lookerApp ��ʼ��

BOOL Ccoin_lookerApp::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()�����򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	WSADATA wsad={0};
	WSAStartup(MAKEWORD(2,2), &wsad);

	AfxEnableControlContainer();

#if _MFC_VER >= 0x0A00
	// ���� shell ���������Է��Ի������
	// �κ� shell ����ͼ�ؼ��� shell �б���ͼ�ؼ���
	CShellManager *pShellManager = new CShellManager;
#endif

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

	Ccoin_lookerDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}

#if _MFC_VER >= 0x0A00
	// ɾ�����洴���� shell ��������
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}
#endif

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}

