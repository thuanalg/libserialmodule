
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

static int callback_to_GUI(void* obj) {
	/*You should clone memory to use*/
	if (!obj) {
		return 0;
	}
	void* hwm = 0;
	SPSR_GENERIC_ST* evt = (SPSR_GENERIC_ST*) obj;
	int n = evt->total;
	//if (evt->type != SPSR_EVENT_READ_BUF) {
	//	return 0;
	//}
	spsr_malloc(n, evt, SPSR_GENERIC_ST);
	if (!evt) {
		return 0;
	}

	memcpy((char*)evt, (char*)obj, n);

	spllog(SPL_LOG_INFO, "data length: %d.", evt->pl - evt->pc);
	if (sizeof(void*) == 4) {
		unsigned int *tmp = (unsigned int*)evt->data;
		hwm = (void*)(*tmp);
	}
	else if (sizeof(void*) == 8)  {
		unsigned long long int* tmp = (unsigned long long int*)evt->data;
		hwm = (void*)(*tmp);
	}
	//hwm = (void*)evt->data;
	spllog(SPL_LOG_INFO, "hwm: 0x%p", hwm);
	//::SendMessageA((HWND)hwm, WM_SPSR_CUSTOM_MESSAGE, 0, (LPARAM)evt);
	::PostMessageA((HWND)hwm, WM_SPSR_CUSTOM_MESSAGE, 0, (LPARAM)evt);
	//spserial_free(evt);
	return 0;
}

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
	ON_WM_CLOSE()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CtestSerialPortDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CtestSerialPortDlg::OnBnClickedCancel)
	ON_EN_CHANGE(IDC_EDIT_COMPORT, &CtestSerialPortDlg::OnEnChangeEditComport)
	ON_BN_CLICKED(IDC_BUTTON_msg, &CtestSerialPortDlg::OnBnClickedButtonmsg)
	ON_BN_CLICKED(IDC_BUTTON_INIT_MODULE, &CtestSerialPortDlg::OnBnClickedButtonInitModule)
	ON_BN_CLICKED(IDC_BUTTON_STOP_MODULE, &CtestSerialPortDlg::OnBnClickedButtonStopModule)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CtestSerialPortDlg::OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, &CtestSerialPortDlg::OnBnClickedButtonRemove)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, &CtestSerialPortDlg::OnBnClickedButtonRemove)
	ON_MESSAGE(WM_SPSR_CUSTOM_MESSAGE, &CtestSerialPortDlg::OnSpSerialCustomMessage)
END_MESSAGE_MAP()


// CtestSerialPortDlg message handlers

BOOL CtestSerialPortDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_listPort.clear();
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
	/*-----------------------------------------------------------------------------------------------------------------------*/
	p_CfgEdit = (CEdit*)GetDlgItem(IDC_EDIT_PATH_CFG);
	p_ComPort = (CEdit*)GetDlgItem(IDC_EDIT_COMPORT);
	p_Cdata = (CEdit*)GetDlgItem(IDC_EDIT_TEXT);
	p_CSent = (CEdit*)GetDlgItem(IDC_EDIT_SEND);
	p_CWCom = (CEdit*)GetDlgItem(IDC_EDIT_WCOM);
	pInitLog = (CButton *)GetDlgItem(IDC_BUTTON_INIT_MODULE);
	/*-----------------------------------------------------------------------------------------------------------------------*/
	return TRUE;
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
	int ret = 0;
	SPSR_INFO_ST* item = 0;

	while (m_listPort.size() > 0) {
		item = (SPSR_INFO_ST*)m_listPort.front();
		spsr_inst_close(item->port_name);
		m_listPort.pop_front();
	}
	ret = spsr_module_finish();
	ret = spl_finish_log();
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();

}


void CtestSerialPortDlg::OnBnClickedCancel()
{
	int ret = 0;
	SPSR_INFO_ST* item = 0;

	while (m_listPort.size() > 0) {
		item = (SPSR_INFO_ST*)m_listPort.front();
		spsr_inst_close(item->port_name);
		m_listPort.pop_front();
	}
	ret = spsr_module_finish();
	ret = spl_finish_log();
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
	CString cdata;
	p_CSent->GetWindowText(cdata);
	int n = cdata.GetLength();
	void* pcomid = 0;
	char portport[64] = { 0 };
	char data[1024] = { 0 };
	SPSR_ARR_LIST_LINED* objId = 0;
	SPSR_INFO_ST* item = 0;
	int comid = 0;
	int i = 0;
	std::list<void *>::iterator it = m_listPort.begin();
	n = cdata.GetLength();
	if (!n) { return; }
	if (n) {
		for (i = 0; i < n; ++i) {
			data[i] = (char)cdata[i];
		}
	}
	/*----------------------------------------------------------------------------------------*/
	p_CWCom->GetWindowText(cdata);
	n = cdata.GetLength();
	if (!n) { return; }
	if (n) {
		for (i = 0; i < n; ++i) {
			portport[i] = (char)cdata[i];
		}
	}
	/*----------------------------------------------------------------------------------------*/
	//n = m_listPort.size();
	//for (i = 0; i < n; ++i) {
	//	std::advance(it, i);
	//	pcomid = *it;
	//	item = (SPSR_INFO_ST*)pcomid;
	//	if (strcmp(portport, item->port_name) == 0) {
	//		spserial_inst_write_to_port(item, data, strlen(data));
	//	}
	//}
	spsr_inst_write(portport, data, strlen(data));
}


void CtestSerialPortDlg::OnBnClickedButtonInitModule()
{
	// TODO: Add your control notification handler code here
	int ret = 0, i =0;
	char cfgpath[1024] = {0};
	int n = 0;
	CString txt;
	p_CfgEdit->GetWindowText(txt);
	n = txt.GetLength();
	for (i = 0; i < n; ++i) {
		cfgpath[i] = txt[i];
	}
	ret = spl_init_log(cfgpath);
	if (ret) {
		exit(1);
	}
	spllog(SPL_LOG_INFO, "test");
	ret = spsr_module_init();
	if (ret) {
		exit(1);
	}
	pInitLog->EnableWindow(FALSE);
	
}


void CtestSerialPortDlg::OnBnClickedButtonStopModule()
{
	// TODO: Add your control notification handler code here
}


void CtestSerialPortDlg::OnBnClickedButtonAdd()
{
	// TODO: Add your control notification handler code here
	int ret = 0, i = 0;
	char port[1024] = { 0 };
	int n = 0;
	CString txt = _T("");
	p_ComPort->GetWindowText(txt);
	n = txt.GetLength();
	for (i = 0; i < n; ++i) {
		port[i] = (char)txt[i];
	}
	SPSR_INPUT_ST obj;
	SPSR_INFO_ST* output = 0;;
	FILE* fp = 0;
	int k = 0;

	SPSR_ARR_LIST_LINED* objId = 0;

	memset(&obj, 0, sizeof(obj));
	snprintf(obj.port_name, SPSR_PORT_LEN, port);
	obj.cb_evt_fn = callback_to_GUI;
	obj.cb_obj = this->m_hWnd;
	obj.checkDSR = 1;
	spllog(SPL_LOG_INFO, "this->m_hWnd: 0x%p.", this->m_hWnd);
	/*obj.baudrate = 115200;*/
	//obj.baudrate = 115200;
	obj.t_delay = 100;
	obj.baudrate = 115200;
	if (ret) {
		exit(EXIT_FAILURE);
	}

	ret = spsr_inst_open(&obj);
	if (ret) {
		//exit(1);
	}
}


void CtestSerialPortDlg::OnBnClickedButtonRemove()
{
	// TODO: Add your control notification handler code here
	char port[1024] = { 0 };
	int n = 0, i = 0;
	CString txt = _T("");
	p_ComPort->GetWindowText(txt);
	n = txt.GetLength();
	for (i = 0; i < n; ++i) {
		port[i] = txt[i];
	}
	/*----------------------------------------------------------------------------------------*/
	int ret = 0;
	ret = spsr_inst_close(port);
}


LRESULT CtestSerialPortDlg::OnSpSerialCustomMessage(WPARAM wParam, LPARAM lParam) {
	SPSR_GENERIC_ST* evt = (SPSR_GENERIC_ST*)lParam;
	char* p = evt->data + evt->pc;
	char buff[1024];
	if (evt->type == SPSR_EVENT_READ_BUF) {
		
		CString txt;
		p_Cdata->GetWindowText(txt);
		CString nstr(p);
		txt.Insert(0, nstr);
		txt.Insert(0, _T("\r\n"));
		p_Cdata->SetWindowText(txt);
	} 
	else if (evt->type == SPSR_EVENT_READ_ERROR) {
		CString txt;
		p_Cdata->GetWindowText(txt);
		CString nstr(p);
		txt.Insert(0, nstr);
		txt.Insert(0, _T("\r\n"));
		p_Cdata->SetWindowText(txt);
	}
	else if (evt->type == SPSR_EVENT_WRITE_OK) {
		CString txt;
		p_Cdata->GetWindowText(txt);
		CString nstr(p);
		CString nstr1(" SPSR_EVENT_WRITE_OK ");
		txt.Insert(0, nstr);
		txt.Insert(0, nstr1);
		txt.Insert(0, _T("\r\n"));
		p_Cdata->SetWindowText(txt);
	}
	else if (evt->type == SPSR_EVENT_WRITE_ERROR) {
		CString txt;
		p_Cdata->GetWindowText(txt);
		//CString nstr(p);
		//CString nstr1(" SPSR_EVENT_WRITE_ERROR ");
		//txt.Insert(0, nstr);
		snprintf(buff, 1024, "\r\n%s|%s|%s", 
			"SPSR_EVENT_WRITE_ERROR",
			spsr_err_txt(evt->err_code),
			p);
		CString nstr(buff);
		txt.Insert(0, nstr);
		//txt.Insert(0, _T("\r\n"));
		p_Cdata->SetWindowText(txt);
	}
	else if (evt->type == SPSR_EVENT_CLOSE_DEVICE_OK) {
		CString txt;
		p_Cdata->GetWindowText(txt);
		CString nstr(p);
		CString nstr1(" SPSR_EVENT_CLOSE_DEVICE_OK ");
		txt.Insert(0, nstr);
		txt.Insert(0, nstr1);
		txt.Insert(0, _T("\r\n"));
		p_Cdata->SetWindowText(txt);
	}
	else if (evt->type == SPSR_EVENT_CLOSE_DEVICE_ERROR) {
		CString txt;
		p_Cdata->GetWindowText(txt);
		CString nstr(p);
		CString nstr1(" SPSR_EVENT_CLOSE_DEVICE_ERROR ");
		txt.Insert(0, nstr);
		txt.Insert(0, nstr1);
		txt.Insert(0, _T("\r\n"));
		p_Cdata->SetWindowText(txt);
	}
	else if (evt->type == SPSR_EVENT_OPEN_DEVICE_OK) {
		CString txt;
		p_Cdata->GetWindowText(txt);
		CString nstr(p);
		CString nstr1(" SPSR_EVENT_OPEN_DEVICE_OK ");
		txt.Insert(0, nstr);
		txt.Insert(0, nstr1);
		txt.Insert(0, _T("\r\n"));
		p_Cdata->SetWindowText(txt);
	}
	else if (evt->type == SPSR_EVENT_OPEN_DEVICE_ERROR) {
		CString txt;
		p_Cdata->GetWindowText(txt);
		CString nstr(p);
		CString nstr1(" SPSR_EVENT_OPEN_DEVICE_ERROR ");
		txt.Insert(0, nstr);
		txt.Insert(0, nstr1);
		txt.Insert(0, _T("\r\n"));
		p_Cdata->SetWindowText(txt);
	} 

	spsr_free(evt);
	return 0;
}


void CtestSerialPortDlg::OnClose()
{
	// TODO: Add your control notification handler code here
	CDialog::OnClose();
	//spl_finish_log();
}
