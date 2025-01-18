#include "serialmodule.h"
#include <stdio.h>
#include <stdlib.h>



int main(int argc, char *argv[]) {
	SP_SERIAL_INPUT_ST obj;
	FILE* fp = 0;
	int myid = 0;
	int ret = 0;
	snprintf(obj.port_name, SPSERIAL_PORT_LEN, "COM2");
	obj.baudrate = 115200;
	ret = spserial_module_init();
	if (ret) {
		return EXIT_FAILURE;
	}
	myid = spserial_module_create(&obj);

	while (1) {
		fp = fopen("trigger_serial.txt", "r");
		if (fp) {
			break;
		}
		spl_sleep(5);
	}
	if (fp) {
		fclose(fp);
	}
	if (myid > 0) {
		spserial_module_del(myid);
	}
	spserial_module_close();
	return 0;
}

