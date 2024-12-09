#include "serialmodule.h"
#include <stdio.h>
#include <simplelog.h>
int
serial_module_init(void *obj) {
	fprintf(stdout, "hi!\n");
	return 0;
}
int
serial_module_close(void* obj) {
	return 0;
}