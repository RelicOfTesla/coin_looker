
// coin_lookerDlg.h : 头文件
//

#pragma once

#include <vector>
#include "afxcmn.h"

// Ccoin_lookerDlg 对话框
class Ccoin_lookerDlg : public CDialogEx
{
// 构造
public:
	Ccoin_lookerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_COIN_LOOKER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
protected:
	std::vector< shared_ptr<class CRecvDialog> > m_vlist;
public:
	CTabCtrl m_tab1;
	afx_msg void OnTcnSelchangingTab1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnClose();
};
