
// testSerialPortDlg.h : header file
//
#include "serialmodule.h"
#include <stdio.h>
#include <stdlib.h>
#pragma once


// CtestSerialPortDlg dialog
class CtestSerialPortDlg : public CDialogEx
{
// Construction
public:
	CtestSerialPortDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TESTSERIALPORT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	int m_myid;
	afx_msg void OnBnClickedCancel();
	afx_msg void OnEnChangeEditComport();
	afx_msg void OnBnClickedButtonmsg();
	afx_msg void OnBnClickedButtonInitModule();
	afx_msg void OnBnClickedButtonStopModule();
	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonRemove();
};
