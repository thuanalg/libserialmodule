#include "serialmodule.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int is_master = 0;
int baudrate = 0;
char is_port[1024];
char cfgpath[1024];
int number_of_ports = 0;

#define __ISMASTER__				"--is_master="
#define __ISPORT__					"--is_port="
#define __ISCFG__					"--is_cfg="
#define __ISBAUDRATE__				"--is_baudrate="

char *test_spsr_list_ports[100];
int spsr_test_callback(void *dta) {
    spllog(0, "callback--------");
    return 0;
}

#ifndef UNIX_LINUX
#include <windows.h>
DWORD dwThreadId_test = 0;
HANDLE hThread_test = 0;
DWORD WINAPI test_try_to_write(LPVOID arg);
#else
#include <pthread.h>
void * test_try_to_write(void *);
#endif

#define TESTTEST "1234567"
/* --is_port=COM2  --is_cfg=C:/z/serialmodule/win32/Debug/simplelog.cfg --is_baudrate=115200*/
int main(int argc, char *argv[]) {
	int i = 0;
    char *p = 0;
	SP_SERIAL_INPUT_ST obj;

	FILE* fp = 0;
	int k = 0;
	int ret = 0;
	

#ifndef UNIX_LINUX
	snprintf(cfgpath, 1024, "C:/z/serialmodule/win32/Debug/simplelog.cfg");
#else
	snprintf(cfgpath, 1024, "simplelog.cfg");
    pthread_t pthreadid = 0;
    
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
            spl_console_log("%s", is_port);
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
    if(ret) {
        spl_console_log("spl_init_log."    );
        exit(1);
    }
    
    ret = spsr_module_init();
    if(ret) {
        exit(1);
    }
    spl_console_log("spsr_module_init. OK"    );
    i = 0;
    p = strtok(is_port, ",");
    //test_spsr_list_ports[i] = p;
    //test_spsr_list_ports = 1;
    while(p) {
        test_spsr_list_ports[number_of_ports++] = p;
        memset(&obj, 0, sizeof(obj));
        spl_console_log("port: %s.", p );
        snprintf(obj.port_name, SPSERIAL_PORT_LEN, "%s", p);
        //snprintf(obj.port_name, SPSERIAL_PORT_LEN, "/dev/cu.Plser");
        /*obj.baudrate = 115200;*/
        obj.baudrate = baudrate;
        obj.t_delay = 100;
        obj.cb_evt_fn = spsr_test_callback;
        if (ret) {
            spl_console_log("Cannot open port."	);
            return EXIT_FAILURE;
        }
        ret = spsr_inst_open(&obj);

        p = strtok(NULL, ",");
        //test_spsr_list_ports[i] = p;
        spl_sleep(5);
        ++i;
        //number_of_ports++;
    }
    //number_of_ports = i+1;
    
#ifndef UNIX_LINUX

    hThread_test = CreateThread(NULL, 0, test_try_to_write, 0, 0, &dwThreadId_test);
    if (!hThread_test) {
        ret = SPSERIAL_THREAD_W32_CREATE;
        spllog(SPL_LOG_DEBUG, "CreateThread error: %d", (int)GetLastError());
    }
#else
    ret = pthread_create(&pthreadid, 0, test_try_to_write, 0);
#endif
	while (1) {
		spl_sleep(2);
		spl_console_log("-----------");
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
#ifndef UNIX_LINUX
    TerminateThread(hThread_test, 0);
#else
    pthread_cancel(pthreadid);
#endif
    spl_sleep(2);
	spsr_module_finish();
	spl_finish_log();
	return 0;
}
#ifndef UNIX_LINUX
#include <windows.h>
DWORD WINAPI test_try_to_write(void* arg)
#else
#include <pthread.h>
void * test_try_to_write(void *arg)
#endif
{
    char text_data[1024];
#define SPSR_TEST_TEXT       "hello"
    int i = 0;
    while(1) {
        spllog(0, "===================");
        for(i = 0; i < number_of_ports; ++i) {
            
            spllog(0, "port: %s", test_spsr_list_ports[i]);
            
            if(!test_spsr_list_ports[i]) {
                continue;
            }
            
            memset(text_data, 0, sizeof(text_data));
            snprintf(text_data, 1024, "%s-%d, port: %s", SPSR_TEST_TEXT, i, test_spsr_list_ports[i]);
            spllog(0, "text_data: %s", text_data);
            spsr_inst_write(test_spsr_list_ports[i], text_data, (int)strlen(text_data));
        }
        spl_sleep(10);
    }
    return 0;
}

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
