#include "serialmodule.h"
#include <stdio.h>
#include <stdlib.h>



int main(int argc, char *argv[]) {
	SP_SERIAL_INPUT_ST obj;
	FILE* fp = 0;
	int myid = 0;
	int ret = 0;
	char cfgpath[1024];

#ifndef UNIX_LINUX
	snprintf(cfgpath, 1024, "C:/z/serialmodule/win32/Debug/simplelog.cfg");
#else
	snprintf(cfgpath, 1024, "simplelog.cfg");
#endif
	ret = spl_init_log(cfgpath);
	memset(&obj, 0, sizeof(obj));
	snprintf(obj.port_name, SPSERIAL_PORT_LEN, "COM2");
	obj.baudrate = 115200;
	ret = spserial_module_init();
	if (ret) {
		return EXIT_FAILURE;
	}
	ret = spserial_module_create(&obj, &myid);

	while (1) {
		spl_sleep(2);
		fp = fopen("C:/z/serialmodule/win32/Debug/trigger_serial.txt", "r");
		if (fp) {
			break;
		}
		spl_sleep(3);
	}
	if (fp) {
		fclose(fp);
	}
	if (myid > 0) {
		spserial_module_del(myid);
	}
	spserial_module_close();
	spl_finish_log();
	return 0;
}

