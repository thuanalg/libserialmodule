
// testSerialPortDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "testSerialPort.h"
#include "testSerialPortDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CtestSerialPortDlg dialog

int is_master = 0;
char is_port[32];
#define __ISMASTER__			"--is_master="
#define __ISPORT__				"--is_port="

#define TESTTEST "1234567--------------------------------"

CtestSerialPortDlg::CtestSerialPortDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TESTSERIALPORT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CtestSerialPortDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CtestSerialPortDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CtestSerialPortDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CtestSerialPortDlg::OnBnClickedCancel)
	ON_EN_CHANGE(IDC_EDIT_COMPORT, &CtestSerialPortDlg::OnEnChangeEditComport)
	ON_BN_CLICKED(IDC_BUTTON_msg, &CtestSerialPortDlg::OnBnClickedButtonmsg)
END_MESSAGE_MAP()


// CtestSerialPortDlg message handlers

BOOL CtestSerialPortDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	int i = 0;
	SP_SERIAL_INPUT_ST obj;
	FILE* fp = 0;
	int k = 0;
	
	int ret = 0;
	char cfgpath[1024];
	SPSERIAL_ARR_LIST_LINED* objId = 0;
#ifndef UNIX_LINUX
	snprintf(cfgpath, 1024, "C:/z/serialmodule/win32/Debug/simplelog.cfg");
	//snprintf(cfgpath, 1024, "D:/reserach/serialmodule/xwin64/Debug/simplelog.cfg");
#else
	snprintf(cfgpath, 1024, "simplelog.cfg");
#endif
	snprintf(is_port, 32, "%s", "COM3");
	ret = spl_init_log(cfgpath);
	memset(&obj, 0, sizeof(obj));
	snprintf(obj.port_name, SPSERIAL_PORT_LEN, is_port);
	/*obj.baudrate = 115200;*/
	//obj.baudrate = 115200;
	obj.baudrate = 115200;
	ret = spserial_module_init();
	if (ret) {
		return EXIT_FAILURE;
	}
	ret = spserial_inst_create(&obj, &m_myid);
	ret = spserial_get_objbyid(m_myid, (void **) & objId, 0);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CtestSerialPortDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CtestSerialPortDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CtestSerialPortDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CtestSerialPortDlg::OnBnClickedOk()
{
	if (m_myid > 0) {
		spserial_inst_del(m_myid);
	}
	spserial_module_close();
	spl_finish_log();
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();

}


void CtestSerialPortDlg::OnBnClickedCancel()
{
	if (m_myid > 0) {
		spserial_inst_del(m_myid);
	}
	spserial_module_close();
	spl_finish_log();
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}


void CtestSerialPortDlg::OnEnChangeEditComport()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}


void CtestSerialPortDlg::OnBnClickedButtonmsg()
{
	// TODO: Add your control notification handler code here
	SPSERIAL_ARR_LIST_LINED* objId = 0;
	int ret = spserial_get_objbyid(m_myid, (void **) & objId, 0);
	spserial_inst_write_to_port(objId->item, TESTTEST, sizeof(TESTTEST));
	spserial_inst_write_to_port(objId->item, TESTTEST, sizeof(TESTTEST));
	spserial_inst_write_to_port(objId->item, TESTTEST, sizeof(TESTTEST));
}
