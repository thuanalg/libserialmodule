#include "serialmodule.h"
#include <stdio.h>
#include <simplelog.h>

#ifndef UNIX_LINUX
    #include <Windows.h>
#else
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <pthread.h>
    #include <semaphore.h>
    #include <unistd.h>
    #include <sys/mman.h>
    #include <sys/stat.h> /* For mode constants */
    #include <fcntl.h> /* For O_* constants */
    #include <errno.h>
#endif
#define SPSERIAL_BUFFER_SIZE        2048



//#define spserial_malloc(__nn__, __obj__, __type__) { (__obj__) = (__type__*) malloc(__nn__); if(__obj__) \
//	{spllog(0, "Malloc: 0x%p\n", (__obj__)); memset((__obj__), 0, (__nn__));} \
//	else {spllog(SPL_LOG_ERROR, "Malloc: error.\n");}} 
//#define spserial_free(__obj__)   { if(obj) { spllog(0, "Free: %x", (__obj__)); free(__obj__); } }

//#ifndef UNIX_LINUX
//    #include <Windows.h>
//    #define YEAR_PADDING								0
//    #define MONTH_PADDING								0
//#else
//    #include <sys/types.h>
//    #include <sys/stat.h>
//    #include <pthread.h>
//    #include <semaphore.h>
//    #include <unistd.h>
//    #include <sys/mman.h>
//    #include <sys/stat.h> /* For mode constants */
//    #include <fcntl.h> /* For O_* constants */
//    #include <errno.h>
//    
//    #define YEAR_PADDING								1900
//    #define MONTH_PADDING								1
//    
//    #define SPL_LOG_UNIX__SHARED_MODE					(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)	
//    #define SPL_LOG_UNIX_CREATE_MODE					(O_CREAT | O_RDWR | O_EXCL)	
//    #define SPL_LOG_UNIX_OPEN_MODE						(O_RDWR | O_EXCL)	
//    #define SPL_LOG_UNIX_PROT_FLAGS						(PROT_READ | PROT_WRITE | PROT_EXEC)		
//
//#endif

//#ifndef UNIX_LINUX
//#else
//#endif

typedef struct __SP_SERIAL_INFO_ST__ {
    int
        iidd;
    char
        isoff;
    char
        is_retry;
    int
        baudrate;
    char
        port_name[32];
    void*
        hEvent;
#ifndef UNIX_LINUX
    void*
#else
    int
#endif
        handle;
    void*
        mtx_off;
    SPSERIAL_module_cb
        cb;
} SP_SERIAL_INFO_ST;

typedef struct __SPSERIAL_ARR_LIST_LINED__ {
    SP_SERIAL_INFO_ST *item;
    struct __SPSERIAL_ARR_LIST_LINED__* next;
} SPSERIAL_ARR_LIST_LINED;

typedef struct __SPSERIAL_ROOT_TYPE__ {
    void* mutex;
    void* sem;
    SPSERIAL_ARR_LIST_LINED* node;
}SPSERIAL_ROOT_TYPE;

#ifndef UNIX_LINUX
static DWORD WINAPI
    spserial_thread_operating_routine(LPVOID lpParam);
#else
static void*
    spserial_thread_operating_routine(void*);
#endif

static
    int spserial_module_isoff(SP_SERIAL_INFO_ST* obj);

static void* 
    spserial_mutex_create();

static int
    spserial_module_openport(void*);

int spserial_module_create(void *obj) 
{
    int ret = 0;
    SP_SERIAL_INPUT_ST* p = (SP_SERIAL_INPUT_ST*)obj;
	fprintf(stdout, "hi!\n");
    do {
        if (!p) {
            ret = SPSERIAL_PORT_INPUT_NULL;
            break;
        }
        if (p->baudrate < 1) {
            ret = SPSERIAL_PORT_BAUDRATE_ERROR;
            break;
        }
        if (!p->port_name[0]) {
            ret = SPSERIAL_PORT_NAME_ERROR;
            break;
        }
        // Open the serial port with FILE_FLAG_OVERLAPPED for asynchronous operation
        HANDLE hSerial = CreateFile(p->port_name,                 // Port name
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
            break;
        }
        else {
            CloseHandle(hSerial);
        }
    } while (0);
	return ret;
}
int spserial_module_setoff(int iid) 
{
	return 0;
}

int
    spserial_module_openport(void* obj) {
	int ret = 0;
	//int baudrate = 11520;
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
         if (p->is_retry) {
             break;
         }
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
         p->hEvent = CreateEvent(0, TRUE, FALSE, 0);
         if (!p->hEvent) {
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
         p->mtx_off = spserial_mutex_create();
         if (!p->mtx_off) {
             DWORD dwError = GetLastError();
             spllog(SPL_LOG_ERROR, "spserial_mutex_create: %lu", dwError);
             ret = SPSERIAL_PORT_SPSERIAL_MUTEX_CREATE;
             break;
         }
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
/*===========================================================================================================================*/
void* spserial_mutex_create() {
    void* ret = 0;
    do {
#ifndef UNIX_LINUX
        ret = CreateMutexA(0, 0, 0);
#else
        /*https://linux.die.net/man/3/pthread_mutex_init*/
        spl_malloc(sizeof(pthread_mutex_t), ret, void);
        //ret = malloc(sizeof(pthread_mutex_t));
        if (!ret) {
            break;
        }
        memset(ret, 0, sizeof(pthread_mutex_t));
        pthread_mutex_init((pthread_mutex_t*)ret, 0);
#endif 
    } while (0);
    return ret;
}
/*===========================================================================================================================*/
int spserial_module_isoff(SP_SERIAL_INFO_ST* obj) {
    int ret = 0;
    ret = obj->isoff;
    return ret;
}

/*===========================================================================================================================*/
#ifndef UNIX_LINUX
DWORD WINAPI
    spserial_thread_operating_routine(LPVOID arg)
#else
void*
    spserial_thread_operating_routine(void* arg)
#endif
{
    SP_SERIAL_INFO_ST* p = (SP_SERIAL_INFO_ST*)arg;
    int isoff = 0;
    int ret = 0;
    while (1) {
        isoff = spserial_module_isoff(p);
        if (isoff) {
            break;
        }
        ret = spserial_module_openport(p);
        if (ret) {
        }
        DWORD dwError = 0;
        DWORD dwEvtMask = 0, flags = 0, bytesRead = 0;;
        OVERLAPPED olRead = { 0 };
        BOOL rs = FALSE;
        int count = 0, cbInQue = 0;
        char readBuffer[SPSERIAL_BUFFER_SIZE + 1];
        COMSTAT csta = { 0 };
        olRead.hEvent = p->hEvent;
        flags = EV_RXCHAR | EV_BREAK | EV_RXFLAG;
        
        while (1) {
            isoff = spserial_module_isoff(p);
            if (isoff) {
                break;
            }
            dwEvtMask = 0;
            rs = WaitCommEvent(p->handle, &dwEvtMask, &olRead);
            if (!rs) {
                DWORD dwRet = GetLastError();
                if (ERROR_IO_PENDING != dwRet) {
                    ++count;
                }
                if (count > 3) {
                    break;
                }
            }
            memset(&csta, 0, sizeof(csta));
            ClearCommError(p->handle, &dwError, &csta);
            if (csta.cbInQue > 0) {
                cbInQue = csta.cbInQue;
                bytesRead = 0;
                memset(readBuffer, 0, sizeof(readBuffer));
                rs = ReadFile(p->handle, readBuffer, SPSERIAL_BUFFER_SIZE, &bytesRead, &olRead);
                memset(&csta, 0, sizeof(csta));
                ClearCommError(p->handle, &dwError, &csta);
                if (csta.cbInQue > 0) {
                    spllog(SPL_LOG_ERROR, "Read Com not finished!!!");
                }
                else {
                    if (p->cb) {
                        int n = 1 + sizeof(SPSERIAL_MODULE_EVENT) + cbInQue;
                        SP_SERIAL_GENERIC_ST* evt = 0;
                        spserial_malloc(n, evt, SP_SERIAL_GENERIC_ST);
                        if (evt) {
                            evt->total = n;
                            evt->type = SPSERIAL_EVENT_READ_BUF;
                            evt->pl = cbInQue;
                            evt->pc = 0;
                            memcpy(evt->data, readBuffer, cbInQue);
                            p->cb(evt);
                        }
                    }
                }
            }
            
        }
        p->is_retry = 1;
    }
    return 0;
}
/*===========================================================================================================================*/
int spserial_module_write_data(int id, char* data, int sz) {
    int ret = 0;
    return ret;
}
/*===========================================================================================================================*/
int spserial_module_init() {
    return 0;
}
/*===========================================================================================================================*/
int spserial_module_close() {
    return 0;
}
/*===========================================================================================================================*/
/*===========================================================================================================================*/
/*===========================================================================================================================*/