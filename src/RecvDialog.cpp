// RecvDialog.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "btc_looker.h"
#include "RecvDialog.h"
#if _MFC_VER >= 0x0A00
#include "afxdialogex.h"
#endif


// CRecvDialog �Ի���

IMPLEMENT_DYNAMIC(CRecvDialog, CDialog)

CRecvDialog::CRecvDialog(shared_ptr<ICoinOption> pCoinOption, CWnd* pParent /*=NULL*/)
	: CDialog(CRecvDialog::IDD, pParent), m_pCoinOption(pCoinOption)
{

}

CRecvDialog::~CRecvDialog()
{
}

void CRecvDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
}


BEGIN_MESSAGE_MAP(CRecvDialog, CDialog)
END_MESSAGE_MAP()


// CRecvDialog ��Ϣ�������

BOOL CRecvDialog::OnInitDialog() 
{
	m_list.InsertColumn(-1, "*");
	m_list.InsertColumn(-1, "�տ��ַ");
	m_list.InsertColumn(-1, "���");
	m_list.InsertColumn(-1, "���������");
	m_list.InsertColumn(-1, "����׽��");
	__super::OnInitDialog();

	return FALSE;
}
