
// btc_lookerDlg.h : ͷ�ļ�
//

#pragma once

#include <vector>
#include "afxcmn.h"

// Cbtc_lookerDlg �Ի���
class Cbtc_lookerDlg : public CDialogEx
{
// ����
public:
	Cbtc_lookerDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_BTC_LOOKER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
