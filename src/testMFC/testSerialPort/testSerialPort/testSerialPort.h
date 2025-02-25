
// testSerialPort.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CtestSerialPortApp:
// See testSerialPort.cpp for the implementation of this class
//

class CtestSerialPortApp : public CWinApp
{
public:
	CtestSerialPortApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CtestSerialPortApp theApp;
