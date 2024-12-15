#include "serialmodule.h"
#include <stdio.h>
#include <simplelog.h>
#ifndef UNIX_LINUX
#include <Windows.h>
#else
#endif

//#ifndef UNIX_LINUX
//#else
//#endif




static int
serial_module_openport(void*);

int
serial_module_init(void *obj) {
	fprintf(stdout, "hi!\n");
	return 0;
}
int
serial_module_close(void* obj) {
	return 0;
}

int
serial_module_openport(void* obj) {
	int ret = 0;
	int baudrate = 11520;
    SP_SERIAL_INFO_ST* p = (SP_SERIAL_INFO_ST*)obj;
    //-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	do {
#ifndef UNIX_LINUX
		DCB dcb = {0};
		HANDLE hSerial = 0;
        COMSTAT comStat = {0};
        DWORD dwError = 0;
        BOOL fSuccess = FALSE;
        DCB dcbSerialParams = { 0 };  // Serial port parameters
        COMMTIMEOUTS timeouts = {0};
        if (!p) {
            ret= SPSERIAL_PORT_INFO_NULL;
            break;
        }

        // Open the serial port with FILE_FLAG_OVERLAPPED for asynchronous operation
        hSerial = CreateFile(p->port_name,                 // Port name
            GENERIC_READ | GENERIC_WRITE,
            0,                          // No sharing
            0,                          // Default security
            OPEN_EXISTING,              // Open an existing port
            FILE_FLAG_OVERLAPPED,       // Asynchronous I/O
            0);                         // No template file

            if (hSerial == INVALID_HANDLE_VALUE) {
                DWORD dwError = GetLastError();
                spllog(SPL_LOG_ERROR, "Open port errcode: %lu", dwError);
                ret = SPSERIAL_PORT_OPEN;
                hSerial = 0;
                break;
            }
            p->handle = hSerial;
            // Set up the serial port parameters (baud rate, etc.)
            dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
            if (!GetCommState(hSerial, &dcbSerialParams)) {
                DWORD dwError = GetLastError();
                spllog(SPL_LOG_ERROR, "GetCommState: %lu", dwError);
                ret = SPSERIAL_PORT_GETCOMMSTATE;
                break;
            }
            dcbSerialParams.BaudRate = p->baudrate;  // Baud rate
            dcbSerialParams.ByteSize = 8;         // 8 data bits
            dcbSerialParams.StopBits = ONESTOPBIT;
            dcbSerialParams.Parity = NOPARITY;
            if (!SetCommState(hSerial, &dcbSerialParams)) {
                DWORD dwError = GetLastError();
                spllog(SPL_LOG_ERROR, "SetCommState: %lu", dwError);
                ret = SPSERIAL_PORT_SETCOMMSTATE;
            }
            p->trigger = CreateEvent(0, TRUE, FALSE, 0);
            if (!p->trigger) {
                DWORD dwError = GetLastError();
                spllog(SPL_LOG_ERROR, "CreateEvent: %lu", dwError);
                ret = SPSERIAL_PORT_CREATEEVENT;
                break;
            }
            // Set timeouts (e.g., read timeout of 500ms, write timeout of 500ms)
            timeouts.ReadIntervalTimeout = 50;
            timeouts.ReadTotalTimeoutConstant = 500;
            timeouts.ReadTotalTimeoutMultiplier = 10;
            timeouts.WriteTotalTimeoutConstant = 500;
            timeouts.WriteTotalTimeoutMultiplier = 10;

            if (!SetCommTimeouts(hSerial, &timeouts)) {
                DWORD dwError = GetLastError();
                spllog(SPL_LOG_ERROR, "SetCommTimeouts: %lu", dwError);
                ret = SPSERIAL_PORT_SETCOMMTIMEOUTS;
                break;
            }
            // Output current COM port status
            //printf("Bytes in queue: %ld\n", comStat.dwInQue);
            //printf("Bytes out queue: %ld\n", comStat.dwOutQue);
            //printf("Error flags: 0x%lx\n", dwError);

            // Example of asynchronous read or write would go here.
            // For simplicity, this is omitted, as it involves working with OVERLAPPED structures.

            // Close the serial port when done
            

#else
#endif
	} while (0);
    //-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    if (ret && p) {
#ifndef UNIX_LINUX
        if (p->handle) {
            CloseHandle(p->handle);
            p->handle = 0;
        }
#else
#endif
    }
    //-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	return ret;
}