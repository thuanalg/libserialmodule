#include "serialmodule.h"



int main(int argc, char *argv[]) {
	SP_SERIAL_INPUT_ST obj;
	int myid = spserial_module_create(&obj);
	return 0;
}

