
// coin_lookerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "coin_looker.h"
#include "coin_lookerDlg.h"
#include "RecvDialog.h"
#if _MFC_VER >= 0x0A00
#include "afxdialogex.h"
#endif

#include <ICoinOption.h>
#include <list>

#include <boost/thread/thread.hpp>
#include "IUserContext.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Ccoin_lookerDlg 对话框

std::string GetAppDir();
std::list<shared_ptr< ICoinOption > > LoadCoinList(const std::string& dir);


std::list<shared_ptr< ICoinOption > > g_CoinList;


Ccoin_lookerDlg::Ccoin_lookerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(Ccoin_lookerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Ccoin_lookerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB1, m_tab1);
}

BEGIN_MESSAGE_MAP(Ccoin_lookerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &Ccoin_lookerDlg::OnBnClickedOk)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &Ccoin_lookerDlg::OnTcnSelchangeTab1)
	ON_NOTIFY(TCN_SELCHANGING, IDC_TAB1, &Ccoin_lookerDlg::OnTcnSelchangingTab1)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// Ccoin_lookerDlg 消息处理程序

BOOL Ccoin_lookerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	auto g_CoinList = LoadCoinList(GetAppDir());
	if (g_CoinList.size())
	{
		int idx = 0;
		for (auto it = g_CoinList.begin(); it != g_CoinList.end(); ++it)
		{
			shared_ptr<ICoinOption> pCoinOption = *it;
			shared_ptr<IUserContext> pWork = create_coin_work(pCoinOption);
			boost::thread th(boost::bind(&IUserContext::load_db, pWork));
			shared_ptr<CRecvDialog> pDlg(new CRecvDialog(pWork, &m_tab1));
			pDlg->Create(CRecvDialog::IDD);
			CRect r;
			pDlg->GetWindowRect(r);
			r.top += 18;
			r.left += 14;
			pDlg->MoveWindow(r);
			m_tab1.InsertItem(m_tab1.GetItemCount(), pCoinOption->prev_name.c_str());
			m_vlist.push_back(pDlg);
		}
		m_vlist[0]->ShowWindow(SW_SHOW);
	}

	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void Ccoin_lookerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR Ccoin_lookerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void bitcoin_test();
#include <stdexcept>


void Ccoin_lookerDlg::OnBnClickedOk()
{
	try
	{
		bitcoin_test();
	}
	catch(std::exception& e)
	{
		AfxMessageBox(e.what());
	}
}


void Ccoin_lookerDlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	m_vlist[m_tab1.GetCurSel()]->ShowWindow(SW_SHOW);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}


void Ccoin_lookerDlg::OnTcnSelchangingTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	m_vlist[m_tab1.GetCurSel()]->ShowWindow(SW_HIDE);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}


void Ccoin_lookerDlg::OnClose()
{
	for (auto it = m_vlist.begin(); it != m_vlist.end(); ++it)
	{
		shared_ptr<CRecvDialog> p = *it;
		if (p)
		{
			p->uninit();
		}
	}
	m_vlist.clear();
	CDialogEx::OnClose();
}
