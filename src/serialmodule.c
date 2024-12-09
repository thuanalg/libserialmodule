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
	const char* portname = "\\\\.\\COM1";
	do {
#ifndef UNIX_LINUX
		DCB dcb = {0};
		HANDLE hCom = 0;
#else
#endif
	} while (0);

	return ret;
}