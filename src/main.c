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
int TEST_CALLBACK_OBJ = 179;
int spsr_test_callback(void *data) {
    /* Data is borrowed from background thread, you should make a copy to use and delete. */
    /* Here, please note data type. */
    if (!data) {
        spllog(SPL_LOG_DEBUG, "Data is empty");
        return 0;
    }
    void* obj = 0;
    int total = 0;
    char* realdata = 0;
    int datalen = 0;
    /* Data is borrowed from background thread, you should make a copy to use and delete. */
    SP_SERIAL_GENERIC_ST* evt = (SP_SERIAL_GENERIC_ST*)data;
    /*char *realdata: from evt->pc to evt->pl.*/
    /* SPSERIAL_MODULE_EVENT evt->type */
    total = evt->total;
    spserial_malloc(total, evt, SP_SERIAL_GENERIC_ST); /* Clone evt memory. */
    if (!evt) {
        exit(1);
    }
    memcpy(evt, data, total);
    spllog(SPL_LOG_DEBUG, "total: %d, type: %d", evt->total, evt->type);
    if (sizeof(void*) == sizeof(unsigned int)) {
        /* 32 bit */
        unsigned int* temp = (unsigned int*)evt->data;
        obj = (void*)(*temp);
    } 
    else if (sizeof(void*) == sizeof(unsigned long long int)) {
        /* 64 bit */
        unsigned long long int* temp = (unsigned long long int*)evt->data;
        obj = (void*)(*temp);
    }
    spllog(0, "obj: 0x%p, value: %d", obj, *((int*)obj));
    do {
        datalen = evt->pl - evt->pc; /*datalen.*/
        realdata = evt->data + evt->pc; /*char *realdata: from evt->pc to evt->pl.*/
        if (evt->type == SPSERIAL_EVENT_READ_BUF) {
            /* Read data.*/
            spllog(0, "SPSERIAL_EVENT_READ_BUF, realdata: %s, datalen: %d", realdata, datalen);
            break;
        }
        if (evt->type == SPSERIAL_EVENT_WRITE_OK) {
            /* Port name .*/
            spllog(0, "SPSERIAL_EVENT_WRITE_OK, realdata: %s, datalen: %d", realdata, datalen);
            break;
        }
        if (evt->type == SPSERIAL_EVENT_WRITE_ERROR) {
            /* Port name .*/
            spllog(0, "SPSERIAL_EVENT_WRITE_ERROR, realdata: %s, datalen: %d", realdata, datalen);
            break;
        }
        if (evt->type == SPSERIAL_EVENT_OPEN_DEVICE_OK) {
            /* Port name .*/
            spllog(0, "SPSERIAL_EVENT_OPEN_DEVICE_OK, realdata: %s, datalen: %d", realdata, datalen);
            break;
        }
        if (evt->type == SPSERIAL_EVENT_OPEN_DEVICE_ERROR) {
            /* Port name .*/
            spllog(0, "SPSERIAL_EVENT_OPEN_DEVICE_ERROR, realdata: %s, datalen: %d", realdata, datalen);
            break;
        }         
        if (evt->type == SPSERIAL_EVENT_CLOSE_DEVICE_OK) {
            /* Port name .*/
            spllog(0, "SPSERIAL_EVENT_CLOSE_DEVICE_OK, realdata: %s, datalen: %d", realdata, datalen);
            break;
        }      
        if (evt->type == SPSERIAL_EVENT_CLOSE_DEVICE_ERROR) {
            /* Port name .*/
            spllog(0, "SPSERIAL_EVENT_CLOSE_DEVICE_ERROR, realdata: %s, datalen: %d", realdata, datalen);
            break;
        }               
    } while (0);
    spserial_free(evt);
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

    /*You MUST init the logger first. */
	ret = spl_init_log(cfgpath);
    if(ret) {
        spl_console_log("spl_init_log."    );
        exit(1);
    }
    /*You MUST init the serial module second. */
    ret = spsr_module_init();
    if(ret) {
        exit(1);
    }
    spl_console_log("spsr_module_init. OK"    );
    i = 0;
    p = strtok(is_port, ",");


    while(p) {
        test_spsr_list_ports[number_of_ports++] = p;
        memset(&obj, 0, sizeof(obj));
        spl_console_log("port: %s.", p );
        snprintf(obj.port_name, SPSERIAL_PORT_LEN, "%s", p);


        obj.baudrate = baudrate;
        obj.t_delay = 100;
        /* The callback will receive data from reading a port. */
        obj.cb_evt_fn = spsr_test_callback;
        obj.cb_obj = &TEST_CALLBACK_OBJ;
        
        if (ret) {
            spl_console_log("Cannot open port."	);
            return EXIT_FAILURE;
        }
        /* Open a port . */
        ret = spsr_inst_open(&obj);

        p = strtok(NULL, ",");
        spl_sleep(5);
        ++i;
    }
    /* Create a thread to writing. . */
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
    /* spl_sleep(2); */
    spl_sleep(1);
    /* You SHOULD close module before exiting.*/
	spsr_module_finish();
    /* You SHOULD close logger before exiting.*/
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
