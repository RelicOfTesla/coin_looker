#pragma once
#include "afxcmn.h"


// CRecvDialog �Ի���

struct IUserContext;

class CRecvDialog : public CDialog
{
	DECLARE_DYNAMIC(CRecvDialog)

public:
	CRecvDialog(shared_ptr<IUserContext>, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CRecvDialog();

// �Ի�������
	enum { IDD = IDD_DIALOG_RECV };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_list;
	shared_ptr<IUserContext> m_pWork;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
public:
	void uninit();

};
