// RecvDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "btc_looker.h"
#include "RecvDialog.h"
#if _MFC_VER >= 0x0A00
#include "afxdialogex.h"
#endif


// CRecvDialog 对话框

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


// CRecvDialog 消息处理程序

BOOL CRecvDialog::OnInitDialog() 
{
	m_list.InsertColumn(-1, "*");
	m_list.InsertColumn(-1, "收款地址");
	m_list.InsertColumn(-1, "余额");
	m_list.InsertColumn(-1, "最后交易日期");
	m_list.InsertColumn(-1, "最后交易金额");
	__super::OnInitDialog();

	return FALSE;
}
