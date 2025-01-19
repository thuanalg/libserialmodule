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
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

#define SPSERIAL_BUFFER_SIZE        2048

#ifndef UNIX_LINUX
    #define SP_SERIAL_THREAD_ROUTINE LPTHREAD_START_ROUTINE
#else
    typedef void* (*SP_SERIAL_THREAD_ROUTINE)(void*);
#endif
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

#ifndef UNIX_LINUX
#else
#endif
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
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
        port_name[SPSERIAL_PORT_LEN];
#ifndef UNIX_LINUX
    void*
        hEvent;
    void*
#else
    int
#endif
        handle;

    void*
        mtx_off;
    void* 
        sem_off;    /*It need to wait for completing.*/
    SPSERIAL_module_cb
        cb;
} SP_SERIAL_INFO_ST;

typedef struct __SPSERIAL_ARR_LIST_LINED__ {
    SP_SERIAL_INFO_ST *item;
    struct __SPSERIAL_ARR_LIST_LINED__* prev;
    struct __SPSERIAL_ARR_LIST_LINED__* next;
} SPSERIAL_ARR_LIST_LINED;

typedef struct __SPSERIAL_ROOT_TYPE__ {
    int n;
    int count;
    void* mutex;
    void* sem;
    
    SPSERIAL_ARR_LIST_LINED* init_node;
    SPSERIAL_ARR_LIST_LINED* last_node;
}SPSERIAL_ROOT_TYPE;
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
static SPSERIAL_ROOT_TYPE
    spserial_root_node;
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#ifndef UNIX_LINUX
static DWORD WINAPI
    spserial_thread_operating_routine(LPVOID lpParam);
#else
static void*
    spserial_thread_operating_routine(void*);
#endif

static
    int spserial_module_isoff(SP_SERIAL_INFO_ST* obj);
static
    int spserial_get_objbyid(int, void **obj, int);
static
    int spserial_get_newid(SP_SERIAL_INPUT_ST *, int *);
static void* 
    spserial_mutex_create();
static void*
    spserial_sem_create();
static int
    spserial_module_openport(void*);
static
    int spserial_mutex_lock(void* obj);
static int 
    spserial_mutex_unlock(void* obj);
static int 
    spserial_rel_sem(void* sem);
static int
    spserial_wait_sem(void* sem);
static int
    spserial_create_thread(SP_SERIAL_THREAD_ROUTINE f, void* arg);
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spserial_module_create(void *obj, int  *idd) 
{
    int ret = 0;
    SP_SERIAL_INPUT_ST* p = (SP_SERIAL_INPUT_ST*)obj;
    SP_SERIAL_INPUT_ST* input_looper = 0;
    //int idd = 0;
	fprintf(stdout, "hi!\n");
    do {
        if (!idd) {
            ret = SPSERIAL_IDD_NULL;
            break;
        }
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
        /* Open the serial port with FILE_FLAG_OVERLAPPED for asynchronous operation */
        /* https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea */
        HANDLE hSerial = CreateFile(p->port_name,                 
            GENERIC_READ | GENERIC_WRITE,
            0,                          
            0,                          
            OPEN_EXISTING,              
            FILE_FLAG_OVERLAPPED,       
            0);                         

        if (hSerial == INVALID_HANDLE_VALUE) {
            DWORD dwError = GetLastError();
            spllog(SPL_LOG_ERROR, "Open port errcode: %lu", dwError);
            ret = SPSERIAL_PORT_OPEN;
            break;
        }
        else {
            CloseHandle(hSerial);
        }
        /*Validate parameter is done.*/
        spserial_malloc(sizeof(SP_SERIAL_INPUT_ST), input_looper, SP_SERIAL_INPUT_ST);
        if (!input_looper) {
            ret = SPSERIAL_MEM_NULL;
            break;
        }
        memcpy(input_looper, p, sizeof(SP_SERIAL_INPUT_ST));
        ret = spserial_get_newid(input_looper, idd);
        if ( *idd < 1) {
            ret = SPSERIAL_GEN_IDD;
            break;
        }
        
    } while (0);

    if (input_looper) {
        spserial_free(input_looper);
    }

	return ret;
}
int spserial_module_del(int iid) 
{
    void *p = 0;
    int ret = 0;
    SPSERIAL_ARR_LIST_LINED* node = 0;
    do {
        ret = spserial_get_objbyid(iid, &p, 1);
        if (p) {
            node = (SPSERIAL_ARR_LIST_LINED*)p;
            spserial_mutex_lock(node->item->mtx_off);
                node->item->isoff = 1;
            spserial_mutex_unlock(node->item->mtx_off);
            SetEvent(node->item->hEvent);

            WaitForSingleObject(node->item->sem_off, INFINITE);
            spserial_free(node->item);
            spserial_free(node);
        }
    } while (0);
	return 0;
}

int spserial_module_openport(void* obj) {
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
         p->sem_off = spserial_sem_create();
         if (!p->sem_off) {
             DWORD dwError = GetLastError();
             spllog(SPL_LOG_ERROR, "spserial_sem_create: %lu", dwError);
             ret = SPSERIAL_PORT_SPSERIAL_SEM_CREATE;
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
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
void* spserial_sem_create() {
    //hd = CreateSemaphoreA(0, 0, 1, nameobj);
    void* ret = 0;
    do {
#ifndef UNIX_LINUX
        ret = CreateSemaphoreA(0, 0, 1, 0);
#else
        /*https://linux.die.net/man/3/sem_init*/
        spserial_malloc(sizeof(sem_t), ret, void);
        if (!ret) {
            break;
        }
        memset(ret, 0, sizeof(sem_t));
        sem_init((sem_t*)ret, 0, 1);
#endif 
    } while (0);
    return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
void* spserial_mutex_create() {
    void* ret = 0;
    do {
#ifndef UNIX_LINUX
        ret = CreateMutexA(0, 0, 0);
#else
        /*https://linux.die.net/man/3/pthread_mutex_init*/
        spserial_malloc(sizeof(pthread_mutex_t), ret, void);
        if (!ret) {
            break;
        }
        memset(ret, 0, sizeof(pthread_mutex_t));
        pthread_mutex_init((pthread_mutex_t*)ret, 0);
#endif 
    } while (0);
    return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spserial_module_isoff(SP_SERIAL_INFO_ST* obj) {
    int ret = 0;
    spserial_mutex_lock(obj->mtx_off);
        ret = obj->isoff;
    spserial_mutex_unlock(obj->mtx_off);
    return ret;
}

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#ifndef UNIX_LINUX
DWORD WINAPI
    spserial_thread_operating_routine(LPVOID arg)
#else
void*
    spserial_thread_operating_routine(void* arg)
#endif
{
    SPSERIAL_ARR_LIST_LINED* pp = (SPSERIAL_ARR_LIST_LINED*)arg;
    SP_SERIAL_INFO_ST* p = pp->item;
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
            rs = SetCommMask(p->handle, flags);
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
            if (!csta.cbInQue) {
                WaitForSingleObject(p->hEvent, INFINITE);
                continue;
            }
            cbInQue = csta.cbInQue;
            bytesRead = 0;
            memset(readBuffer, 0, sizeof(readBuffer));
            rs = ReadFile(p->handle, readBuffer, SPSERIAL_BUFFER_SIZE, &bytesRead, &olRead);
            memset(&csta, 0, sizeof(csta));
            ClearCommError(p->handle, &dwError, &csta);
            
            if (csta.cbInQue > 0) {
                spllog(SPL_LOG_ERROR, "Read Com not finished!!!");
            }
            else /*else start*/
            {
                /*p->cb start*/
                if (p->cb) 
                {
                    int n = 1 + sizeof(SPSERIAL_MODULE_EVENT) + cbInQue;
                    SP_SERIAL_GENERIC_ST* evt = 0;
                    spserial_malloc(n, evt, SP_SERIAL_GENERIC_ST);
                    if (evt) 
                    {
                        evt->total = n;
                        evt->type = SPSERIAL_EVENT_READ_BUF;
                        evt->pl = cbInQue;
                        evt->pc = 0;
                        memcpy(evt->data, readBuffer, cbInQue);
                        p->cb(evt);
                    }
                }
                /*p->cb end*/
            }
        }
        p->is_retry = 1;
    }
    ReleaseSemaphore(p->sem_off, 1, 0);
    return 0;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spserial_module_write_data(int id, char* data, int sz) {
    int ret = 0;
    do {

        //TO DO
    } while (0);
    return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spserial_module_init() {
    int ret = 0;
    do {
        spserial_root_node.mutex = spserial_mutex_create();
        if (!spserial_root_node.mutex) {
            ret = SPSERIAL_MTX_CREATE;
            break;
        }
        spserial_root_node.sem = spserial_sem_create();
        if (!spserial_root_node.mutex) {
            ret = SPSERIAL_SEM_CREATE;
            break;
        }
    } while (0);
    return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spserial_module_close() {
    SPSERIAL_ROOT_TYPE* t = &spserial_root_node;
    CloseHandle(t->mutex);
    CloseHandle(t->sem);
    return 0;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spserial_get_newid(SP_SERIAL_INPUT_ST *p, int *idd) {
    int ret = 0;
    SPSERIAL_ROOT_TYPE* t = &spserial_root_node;
    SPSERIAL_ARR_LIST_LINED* obj = 0;
    do {
        if (!t->mutex) {
            ret = SPSERIAL_MUTEX_NULL_ERROR;
            break;
        }
        if (!t->sem) {
            ret = SPSERIAL_SEM_NULL_ERROR;
            break;
        }
        //SPSERIAL_ARR_LIST_LINED
        if (!p) {
            ret = SPSERIAL_INPUT_NULL_ERROR;
            break;
        }
        spserial_malloc(sizeof(SPSERIAL_ARR_LIST_LINED), obj, SPSERIAL_ARR_LIST_LINED);
        if (!obj) {
            ret = SPSERIAL_MEM_NULL;
            break;
        }
        spserial_malloc(sizeof(SP_SERIAL_INFO_ST), obj->item, SP_SERIAL_INFO_ST)
        if (!obj->item) {
            ret = SPSERIAL_MEM_NULL;
            break;
        }
        spserial_mutex_lock(t->mutex);
        /*do {*/
            t->n++;
            (*idd) = t->n;
            if (!t->init_node) {
                t->init_node = obj;
                t->last_node = obj;
            }
            else {
                obj->prev = t->last_node;
                t->last_node->next = obj;
                t->last_node = obj;
            }
            t->count++;
        /* } while (0);*/
        spserial_mutex_unlock(t->mutex);
        
        snprintf(obj->item->port_name, SPSERIAL_PORT_LEN, "%s", p->port_name);
        obj->item->baudrate = p->baudrate;
        obj->item->cb = p->cb;
        obj->item->iidd = *idd;
        ret = spserial_create_thread(spserial_thread_operating_routine, obj);
    } while (0);
    if (ret) {
        spserial_free(obj);
    }
    return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spserial_mutex_lock(void* obj) {
    int ret = 0;
#ifndef UNIX_LINUX
    DWORD err = 0;
#else
#endif	
    do {
        if (!obj) {
            ret = SPSERIAL_MUTEX_NULL_ERROR;
            break;
        }
#ifndef UNIX_LINUX
        err = WaitForSingleObject(obj, INFINITE);
        if (err != WAIT_OBJECT_0) {
            ret = 1;
            break;
        }
#else
        SPL_pthread_mutex_lock((pthread_mutex_t*)obj, ret);
#endif
    } while (0);
    return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spserial_mutex_unlock(void* obj) {
    int ret = 0;
#ifndef UNIX_LINUX
    DWORD done = 0;
#else
#endif	
    do {
        if (!obj) {
            ret = SPSERIAL_MUTEX_NULL_ERROR;
            break;
        }
#ifndef UNIX_LINUX
        done = ReleaseMutex(obj);
        if (!done) {
            DWORD dwError = GetLastError();
            spllog(SPL_LOG_ERROR, "WaitForSingleObject errcode: %lu", dwError);
            ret = 1;
            break;
        }
#else
        SPL_pthread_mutex_unlock((pthread_mutex_t*)obj, ret);
#endif
    } while (0);
    return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spserial_rel_sem(void* sem) {
    int ret = 0;
#ifndef UNIX_LINUX
#else
    int err = 0, val = 0;
#endif
    do {
        if (!sem) {
            ret = SPSERIAL_SEM_NULL_ERROR;
            break;
        }
#ifndef UNIX_LINUX
        ReleaseSemaphore(sem, 1, 0);
#else
        err = sem_getvalue((sem_t*)sem, &val);
        if (!err) {
            if (val < 1) {
                ret = sem_post((sem_t*)sem);
                if (ret) {
                    spl_console_log("sem_post: ret: %d, errno: %d, text: %s.", ret, errno, strerror(errno));
                }
            }
        }
#endif 
    } while (0);
    return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spserial_wait_sem(void* sem) {
    int ret = 0;
    do {
        if (!sem) {
            ret = SPSERIAL_SEM_NULL_ERROR;
            break;
        }
#ifndef UNIX_LINUX
        int iswell = WaitForSingleObject((HANDLE)sem, INFINITE);
        if (iswell != WAIT_OBJECT_0) {
            DWORD dwError = GetLastError();
            spllog(SPL_LOG_ERROR, "WaitForSingleObject errcode: %lu", dwError);
            ret = SPSERIAL_SEM_POST_ERROR;
        }
#else
        ret = sem_wait((sem_t*)sem);
#endif 
    } while (0);
    return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spserial_get_objbyid(int idd, void** obj, int takeoff) {
    int ret = 0;
    SPSERIAL_ROOT_TYPE* t = &spserial_root_node;
    SPSERIAL_ARR_LIST_LINED* node = 0, * prev = 0, *next  = 0;;
    do {
        if (!obj) {

            break;
        }
        spserial_mutex_lock(t->mutex);
        node = t->init_node;
        while (node) {
            if (node->item->iidd == idd) {
                *obj = node;
                if (takeoff) {
                    prev = node->prev;
                    next = node->next;
                    if (!prev) {
                        if (!next) {
                            t->init_node = 0;
                            t->last_node = 0;
                        }
                        else {
                            t->init_node = next;
                            t->init_node->prev = 0;
                        }
                    }
                    else {
                        if (!next) {
                            prev->next = 0;
                            t->last_node = prev;
                        }
                        else {
                            prev->next = next;
                            next->prev = prev;
                        }
                    }
                    t->count--;
                }
                break;
            }
            node = node->next;
        }
        spserial_mutex_unlock(t->mutex);
    } while (0);
    return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/


int spserial_create_thread(SP_SERIAL_THREAD_ROUTINE f, void* arg) {
    int ret = 0;
#ifndef UNIX_LINUX
    DWORD dwThreadId = 0;
    HANDLE hThread = 0;
    hThread = CreateThread(NULL, 0, f, arg, 0, &dwThreadId);
    if (!hThread) {
        ret = SPSERIAL_THREAD_W32_CREATE;
        spl_console_log("CreateThread error: %d", (int)GetLastError());
    }
#else
    pthread_t tidd = 0;
    ret = pthread_create(&tidd, 0, f, arg);
    if (ret) {
        ret = SPL_LOG_THREAD_PX_CREATE;
        spl_console_log("pthread_create: ret: %d, errno: %d, text: %s.", ret, errno, strerror(errno));
    }
#endif
    return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/