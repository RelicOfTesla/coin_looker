// RecvDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "btc_looker.h"
#include "RecvDialog.h"
#if _MFC_VER >= 0x0A00
#include "afxdialogex.h"
#endif
#include <boost/lexical_cast.hpp>
#include "IUserContext.h"

CString TT2CS(time_t nt)
{
	if (nt == 0)
	{
		return "";
	}
	CTime ct(nt);
	return ct.Format("%Y-%m-%d %H:%M:%S");
}

struct workdata 
{
	std::string RecvCoinAddr;
	double RecvMoney;
	double LastMoney;
	time_t LastTime;
};
// CRecvDialog 对话框

enum {
	ITEM_ID,
	ITEM_COIN_ADDR,
	ITEM_COIN_BALANCE,
	ITEM_LAST_DATE,
	ITEM_LAST_MONEY,
};

IMPLEMENT_DYNAMIC(CRecvDialog, CDialog)

CRecvDialog::CRecvDialog(shared_ptr<IUserContext> pWork, CWnd* pParent /*=NULL*/)
	: CDialog(CRecvDialog::IDD, pParent), m_pWork(pWork)
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
	ON_WM_TIMER()
//	ON_WM_CLOSE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CRecvDialog 消息处理程序

BOOL CRecvDialog::OnInitDialog() 
{
	__super::OnInitDialog();
//	m_list.SetExtendedStyle();
	m_list.InsertColumn(ITEM_ID, "*", LVCFMT_LEFT, 30);
	m_list.InsertColumn(ITEM_COIN_ADDR, "收款地址", LVCFMT_LEFT, 200);
	m_list.InsertColumn(ITEM_COIN_BALANCE, "余额", LVCFMT_LEFT, 70);
	m_list.InsertColumn(ITEM_LAST_DATE, "最后交易日期", LVCFMT_LEFT, 150);
	m_list.InsertColumn(ITEM_LAST_MONEY, "最后交易金额", LVCFMT_LEFT, 120);

	auto vlist = m_pWork->work_get_books();
	for (auto it = vlist.begin(); it != vlist.end(); ++it)
	{
		int nItem = m_list.InsertItem(m_list.GetItemCount(), "");
		m_list.SetItemText(nItem, 1, it->c_str());
	}

	SetTimer(1, 5000, 0);
	SetTimer(2, 1000, 0);

	return FALSE;
}

void CRecvDialog::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 1)
	{
		m_pWork->modal();
		for (int nItem = 0; nItem < m_list.GetItemCount(); ++nItem)
		{
			CString addr = m_list.GetItemText(nItem, ITEM_COIN_ADDR);
			auto data = m_pWork->work_get_data(addr);
			m_list.SetItemText(nItem, ITEM_COIN_BALANCE, boost::lexical_cast<std::string>(data.RecvMoney).c_str());
			m_list.SetItemText(nItem, ITEM_LAST_DATE, TT2CS(data.LastTime));
			m_list.SetItemText(nItem, ITEM_LAST_MONEY, boost::lexical_cast<std::string>(data.LastMoney).c_str());
		}
	}
	else if (nIDEvent == 2)
	{
		GetDlgItem(IDC_STATIC_CUR_TIME)->SetWindowText(""+TT2CS(m_pWork->work_get_current_time()));
	}
	CDialog::OnTimer(nIDEvent);
}

void CRecvDialog::uninit()
{
	if (m_pWork)
	{
		m_pWork->uninit();
		m_pWork.reset();
	}
}

void CRecvDialog::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	uninit();

	CDialog::OnClose();
}


void CRecvDialog::OnDestroy()
{
	CDialog::OnDestroy();
	uninit();
	// TODO: 在此处添加消息处理程序代码
}
