#pragma once
#include "afxcmn.h"


// CRecvDialog �Ի���

struct ICoinOption;

class CRecvDialog : public CDialog
{
	DECLARE_DYNAMIC(CRecvDialog)

public:
	CRecvDialog(shared_ptr<ICoinOption>, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CRecvDialog();

// �Ի�������
	enum { IDD = IDD_DIALOG_RECV };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_list;
	shared_ptr<ICoinOption> m_pCoinOption;
};
