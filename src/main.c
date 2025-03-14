#include "serialmodule.h"
#include <stdio.h>
#include <stdlib.h>

int is_master = 0;
int baudrate = 0;
char is_port[32];
char cfgpath[1024];

#define __ISMASTER__				"--is_master="
#define __ISPORT__					"--is_port="
#define __ISCFG__					"--is_cfg="
#define __ISBAUDRATE__				"--is_baudrate="

#define TESTTEST "1234567"
/* --is_port=COM2  --is_cfg=C:/z/serialmodule/win32/Debug/simplelog.cfg --is_baudrate=115200*/
int main(int argc, char *argv[]) {
	int i = 0;
	SP_SERIAL_INPUT_ST obj;
	SP_SERIAL_INFO_ST *obj1 = 0;
	FILE* fp = 0;
	int k = 0;
	int myid = 0;
	int ret = 0;
	
	SPSERIAL_ARR_LIST_LINED* objId = 0;
#ifndef UNIX_LINUX
	snprintf(cfgpath, 1024, "C:/z/serialmodule/win32/Debug/simplelog.cfg");
#else
	snprintf(cfgpath, 1024, "simplelog.cfg");
#endif
	snprintf(is_port, 32,"%s", "COM2");
	baudrate = 115200;
	for (i = 0; i < argc; ++i) {
		if (strstr(argv[i], __ISMASTER__)) {
			 k = sscanf(argv[i], __ISMASTER__"%d", &is_master);
			 spl_console_log("k = %d.", k);
			continue;
		}
		if (strstr(argv[i], __ISPORT__)) {
			k = sscanf(argv[i], __ISPORT__"%s", is_port);
			spl_console_log("k = %d.", k);
			continue;
		}
		if (strstr(argv[i], __ISCFG__)) {
			k = sscanf(argv[i], __ISCFG__"%s", cfgpath);
			spl_console_log("k = %d.", k);
			continue;
		}
		if (strstr(argv[i], __ISBAUDRATE__)) {
			k = sscanf(argv[i], __ISBAUDRATE__"%d", &baudrate);
			spl_console_log("k = %d, baudrate: %d.", k, baudrate);
			continue;
		}
	}


	ret = spl_init_log(cfgpath);
	memset(&obj, 0, sizeof(obj));
	snprintf(obj.port_name, SPSERIAL_PORT_LEN, is_port);
	/*obj.baudrate = 115200;*/
	obj.baudrate = baudrate;
	ret = spserial_module_init();
	if (ret) {
		return EXIT_FAILURE;
	}
	ret = spserial_inst_create(&obj);
	if (ret) {
		return 1;
	}
	/*	
	ret = spserial_get_objbyid(myid, &objId, 0);
	if (ret) {
		return 1;
	}
	*/
	while (1) {
		spl_sleep(5);
		spl_console_log("-----------");
		/*
		if (!is_master) {
			spserial_inst_write_to_port(objId->item, TESTTEST, sizeof(TESTTEST));
		}
		*/
#ifndef UNIX_LINUX
		fp = fopen("C:/z/serialmodule/win32/Debug/trigger_serial.txt", "r");
#else
		fp = fopen("trigger_serial.txt", "r");
#endif
		if (fp) {
			break;
		}
		
	}
	if (fp) {
		fclose(fp);
	}
	if (obj1) {
		spserial_inst_del(obj1->port_name);
	}
	spserial_module_close();
	spl_finish_log();
	return 0;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
/*
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* readline();
char* ltrim(char*);
char* rtrim(char*);

int parse_int(char*);




#define MAX_GRASSHOPPER		(2 * 100000) 
const char* GRASSHOPPER[] = {"FizzBuzz", "Fizz", "Buzz", 0};
void fizzBuzz(int n) {
	int i = 1;
	int index = -1;
	if (n < 1) {
		return;
	}
	if (n > MAX_GRASSHOPPER) {
		return;
	}
	while (i <= n) {
		index = -1;
		do {
			if (i % 3 == 0) {
				if (i % 5 == 0) {
					index = 0;
					break;
				}
				index = 1;
				break;
			}
			if (i % 5 == 0) {
				index = 2;
				break;
			}
		} while (0);
		if (index > -1) {
			fprintf(stdout, "%s\n", GRASSHOPPER[index]);
		}
		else {
			fprintf(stdout, "%d\n", i);
		}
		++i;
	}
}

int __main__()
{
	int n = parse_int(ltrim(rtrim(readline())));

	fizzBuzz(n);

	return 0;
}

char* readline() {
	size_t alloc_length = 1024;
	size_t data_length = 0;

	char* data = malloc(alloc_length);

	while (true) {
		char* cursor = data + data_length;
		char* line = fgets(cursor, alloc_length - data_length, stdin);

		if (!line) {
			break;
		}

		data_length += strlen(cursor);

		if (data_length < alloc_length - 1 || data[data_length - 1] == '\n') {
			break;
		}

		alloc_length <<= 1;

		data = realloc(data, alloc_length);

		if (!data) {
			data = '\0';

			break;
		}
	}

	if (data[data_length - 1] == '\n') {
		data[data_length - 1] = '\0';

		data = realloc(data, data_length);

		if (!data) {
			data = '\0';
		}
	}
	else {
		data = realloc(data, data_length + 1);

		if (!data) {
			data = '\0';
		}
		else {
			data[data_length] = '\0';
		}
	}

	return data;
}

char* ltrim(char* str) {
	if (!str) {
		return '\0';
	}

	if (!*str) {
		return str;
	}

	while (*str != '\0' && isspace(*str)) {
		str++;
	}

	return str;
}

char* rtrim(char* str) {
	if (!str) {
		return '\0';
	}

	if (!*str) {
		return str;
	}

	char* end = str + strlen(str) - 1;

	while (end >= str && isspace(*end)) {
		end--;
	}

	*(end + 1) = '\0';

	return str;
}

int parse_int(char* str) {
	char* endptr;
	int value = strtol(str, &endptr, 10);

	if (endptr == str || *endptr != '\0') {
		exit(EXIT_FAILURE);
	}

	return value;
}
*/




















