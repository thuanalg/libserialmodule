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
    #include <termios.h>

    #ifdef __TRUE_LINUX__
        #include <sys/epoll.h>
    #else
        #include <poll.h>
    #endif 
    /* https://gist.github.com/reterVision/8300781 */
#endif

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#ifndef UNIX_LINUX
#define SPSERIAL_CloseHandle(__o0bj__) \
		{void *__serialpp__ = (__o0bj__);\
            if(__serialpp__) { \
                ;int bl = CloseHandle((__serialpp__));\
                ;if(!bl) { \
                    ;spllog(SPL_LOG_ERROR, "CloseHandle error: %lu", GetLastError()); \
                ;}\
                ;spllog(0, "SPSERIAL_CloseHandle 0x%p -->> %s", __serialpp__, (bl ? "DONE": "ERROR")); \
                ;(__o0bj__) = 0;\
            ;}\
        }
#else
#endif

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#define SPSERIAL_MAX_AB(__a__, __b__)               ((__a__) > (__b__)) ? (__a__) : (__b__)
#define SPSERIAL_STEP_MEM                           1024
#define SPSERIAL_BUFFER_SIZE                        2048

#ifndef UNIX_LINUX
    #define SP_SERIAL_THREAD_ROUTINE LPTHREAD_START_ROUTINE
#else
    typedef void* (*SP_SERIAL_THREAD_ROUTINE)(void*);
#endif

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

#ifndef UNIX_LINUX
#else
#endif


#ifndef UNIX_LINUX
#else
static int spserial_init_trigger(void*);
static int spserial_pull_trigger(void*);
static int spserial_start_listen(void*);
#endif



void thuan() { }

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

static int 
    spserial_module_isoff(SP_SERIAL_INFO_ST* obj);

static int 
    spserial_get_newid(SP_SERIAL_INPUT_ST *, int *);
static int
    spserial_module_openport(void*);
static int
    spserial_create_thread(SP_SERIAL_THREAD_ROUTINE f, void* arg);
static int 
    spserial_clear_node(SPSERIAL_ARR_LIST_LINED *);

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
/* Group of sync tool. */
static void*
    spserial_mutex_create();
static void*
    spserial_sem_create();
static  int 
    spserial_mutex_lock(void* obj);
static int
    spserial_mutex_unlock(void* obj);
static int
    spserial_rel_sem(void* sem);
static int
    spserial_wait_sem(void* sem);

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

int spserial_inst_create(void *obj, int  *idd) 
{
    int ret = 0;
    SP_SERIAL_INPUT_ST* p = (SP_SERIAL_INPUT_ST*)obj;
    SP_SERIAL_INPUT_ST* input_looper = 0;
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
            spllog(0, "Create hSerial: 0x%p.", hSerial);
            SPSERIAL_CloseHandle(hSerial);
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
int spserial_inst_del(int iid) 
{
    void *p = 0;
    int ret = 0;
    SPSERIAL_ARR_LIST_LINED* node = 0;
    do {
        ret = spserial_get_objbyid(iid, &p, 1);
        if (p) {
            node = (SPSERIAL_ARR_LIST_LINED*)p;
            spserial_clear_node(node);
        }
    } while (0);
	return 0;
}

int spserial_module_openport(void* obj) {
	int ret = 0;
    SP_SERIAL_INFO_ST* p = (SP_SERIAL_INFO_ST*)obj;

    /*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
	do {
#ifndef UNIX_LINUX
	DCB dcb = {0};
	HANDLE hSerial = 0;
        COMSTAT comStat = {0};
        DWORD dwError = 0;
        BOOL fSuccess = FALSE;
        DCB dcbSerialParams = { 0 };  
        COMMTIMEOUTS timeouts = {0};
        if (!p) {
            ret= SPSERIAL_PORT_INFO_NULL;
            break;
        }

        /*Open the serial port with FILE_FLAG_OVERLAPPED for asynchronous operation*/
        hSerial = CreateFile(p->port_name,                 
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
             hSerial = 0;
             break;
         }
         p->handle = hSerial;
         spllog(0, "Create hSerial: 0x%p.", hSerial);
         if (p->is_retry) {
             break;
         }
         /* Set up the serial port parameters(baud rate, etc.) */
         dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
         if (!GetCommState(hSerial, &dcbSerialParams)) {
             DWORD dwError = GetLastError();
             spllog(SPL_LOG_ERROR, "GetCommState: %lu", dwError);
             ret = SPSERIAL_PORT_GETCOMMSTATE;
             break;
         }
         dcbSerialParams.BaudRate = p->baudrate;  
         dcbSerialParams.ByteSize = 8;         
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
         spllog(0, "Create hEvent: 0x%p.", p->hEvent);
         /* Set timeouts(e.g., read timeout of 500ms, write timeout of 500ms) */
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
    /*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
    if (ret && p) {
#ifndef UNIX_LINUX
        if (p->handle) {
            SPSERIAL_CloseHandle(p->handle);
        }
#else
#endif
    }
    /*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
void* spserial_sem_create() {
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
    spllog(0, "Create: 0x%p.", ret);
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
    spllog(0, "Create: 0x%p.", ret);
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
DWORD WINAPI spserial_thread_operating_routine(LPVOID arg)
{

    SPSERIAL_ARR_LIST_LINED* pp = (SPSERIAL_ARR_LIST_LINED*)arg;
    SP_SERIAL_INFO_ST* p = pp->item;
    int isoff = 0;
    int ret = 0;
    SP_SERIAL_GENERIC_ST* buf = 0;
    int kkkk = 0;
    int step = sizeof(SP_SERIAL_GENERIC_ST) + SPSERIAL_STEP_MEM;
    spserial_malloc(step, buf, SP_SERIAL_GENERIC_ST);
    buf->total = step;
    buf->range = buf->total - sizeof(SP_SERIAL_GENERIC_ST);
    while (1) {
        DWORD dwError = 0;
        int wrote = 0;
        DWORD dwEvtMask = 0, flags = 0, bytesRead = 0;;
        OVERLAPPED olRead = { 0 };
        OVERLAPPED olRrite = { 0 };
        BOOL rs = FALSE;
        int count = 0, cbInQue = 0;
        char readBuffer[SPSERIAL_BUFFER_SIZE + 1];
        COMSTAT csta = { 0 };
        olRead.hEvent = p->hEvent;
        flags = EV_RXCHAR | EV_BREAK | EV_RXFLAG;

        if (!buf) {
            break;
        }
        isoff = spserial_module_isoff(p);
        if (isoff) {
            break;
        }
        ret = spserial_module_openport(p);
        if (ret) {
        }
        
        while (1) {
            isoff = spserial_module_isoff(p);
            if (isoff) {
                break;
            }
            memset(&olRead, 0, sizeof(olRead));
            olRead.hEvent = p->hEvent;
            spserial_mutex_lock(p->mtx_off);
                do {
                    int kkk = 0;
                    if (!p->buff) {
                        break;
                    }
                    if (p->buff->pl < 1) {
                        break;
                    }
                    rs = WriteFile(p->handle, p->buff->data, p->buff->pl, &kkk, &olRead);
                    //spl_console_log("WriteFile OK");
                    p->buff->pl = 0;
                    wrote = 1;
                } while (0);
            spserial_mutex_unlock(p->mtx_off);

            if (wrote) {
                wrote = 0;
                /*
                SetEvent(p->hEvent);
                */
            }

            rs = SetCommMask(p->handle, flags);
            dwEvtMask = flags;
            memset(&olRead, 0, sizeof(olRead));
            olRead.hEvent = p->hEvent;
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
            else {
                spl_console_log("WaitCommEvent OK");
            }
            memset(&csta, 0, sizeof(csta));
            ClearCommError(p->handle, &dwError, &csta);
            cbInQue = csta.cbInQue;
            if (!cbInQue) {
                //WaitForSingleObject(p->hEvent, 10);
                
                WaitForSingleObject(p->hEvent, INFINITE);
                OVERLAPPED overlapped = { 0 };
                overlapped.hEvent = p->hEvent;
                if (GetOverlappedResult(p->handle, &overlapped, &bytesRead, TRUE)) {
                }
                else {
                    PurgeComm(p->handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
                }
                continue;
            }
            //Sleep(1000);
            
            bytesRead = 0;
            memset(readBuffer, 0, sizeof(readBuffer));
            rs = ReadFile(p->handle, readBuffer, SPSERIAL_BUFFER_SIZE, &bytesRead, &olRead);
            bytesRead = olRead.InternalHigh;
            if (!bytesRead) {
                
                ResetEvent(p->hEvent);
            }
            spllog(SPL_LOG_ERROR, "olRead.InternalHigh: %d, olRead.Internal: %d!!!", (int)olRead.InternalHigh, (int)olRead.Internal);
            if (!rs) {
                if (GetLastError() == ERROR_IO_PENDING) {
                    OVERLAPPED overlapped = { 0 };
                    overlapped.hEvent = p->hEvent;
                    if (GetOverlappedResult(p->handle, &overlapped, &bytesRead, TRUE)) {
                        //if (!olRead.InternalHigh) {
                        //    memset(readBuffer, 0, sizeof(readBuffer));
                        //    rs = ReadFile(p->handle, readBuffer, SPSERIAL_BUFFER_SIZE, &bytesRead, &olRead);
                        //    spllog(SPL_LOG_ERROR, "olRead.InternalHigh: %d!!!", (int)olRead.InternalHigh);
                        //}
                    }
                    else {
                        PurgeComm(p->handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
                    }
                }
                else {
                }
            }
            memset(&csta, 0, sizeof(csta));
            ClearCommError(p->handle, &dwError, &csta);
            
            if (csta.cbInQue > 0) {
                spllog(SPL_LOG_ERROR, "Read Com not finished!!!");
            }
            else if(cbInQue > 0)/*else start*/
            {
                if (cbInQue == 1) {
                    int a = 0;
                }
                spllog(SPL_LOG_INFO, "            [[[ %s, cbInQue: %d ]]]", readBuffer, cbInQue);
                //spl_console_log("%s", readBuffer);
                /* ResetEvent(p->hEvent); */
                //ResetEvent(p->hEvent);
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
        if (isoff) {
            break;
        }
    }
    spserial_free(buf);
    SPSERIAL_CloseHandle(p->handle);
    SPSERIAL_CloseHandle(p->hEvent);
    /*ReleaseSemaphore(p->sem_off, 1, 0);*/
    spserial_rel_sem(p->sem_off);
    return 0;
}
#else 
    void* spserial_thread_operating_routine(void* obj) {
        return 0;
    }
#endif
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spserial_module_init() {
    int ret = 0;
    do {
#ifndef UNIX_LINUX
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
#else
        ret = spserial_init_trigger(0);
        if (ret) {
            break;
        }
	    ret = spserial_start_listen(0);
        if (ret) {
            break;
        }
#endif
        spl_console_log("spserial_module_init: DONE");
    } while (0);
    return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spserial_module_close() {
    SPSERIAL_ROOT_TYPE* srl = &spserial_root_node;
#ifndef UNIX_LINUX
    SPSERIAL_CloseHandle(srl->mutex);
    SPSERIAL_CloseHandle(srl->sem);
#else
#endif
    return 0;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spserial_get_newid(SP_SERIAL_INPUT_ST *p, int *idd) {
    int ret = 0;
    SPSERIAL_ROOT_TYPE* srl = &spserial_root_node;
    SPSERIAL_ARR_LIST_LINED* obj = 0;
    do {
        if (!srl->mutex) {
            ret = SPSERIAL_MUTEX_NULL_ERROR;
            break;
        }
        if (!srl->sem) {
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
        spserial_mutex_lock(srl->mutex);
        /*do {*/
            srl->n++;
            (*idd) = srl->n;
            if (!srl->init_node) {
                srl->init_node = obj;
                srl->last_node = obj;
            }
            else {
                obj->prev = srl->last_node;
                srl->last_node->next = obj;
                srl->last_node = obj;
            }
            srl->count++;
        /* } while (0);*/
        spserial_mutex_unlock(srl->mutex);
        
        snprintf(obj->item->port_name, SPSERIAL_PORT_LEN, "%s", p->port_name);
        obj->item->baudrate = p->baudrate;
        obj->item->cb = p->cb;
        obj->item->iidd = *idd;
#ifndef UNIX_LINUX
        ret = spserial_create_thread(spserial_thread_operating_routine, obj);
#else
        spl_console_log("Need to do here.");
#endif
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
        /*
        err = sem_getvalue((sem_t*)sem, &val);
        if (!err) {
            if (val < 1) {
                ret = sem_post((sem_t*)sem);
                if (ret) {
                    spl_console_log("sem_post: ret: %d, errno: %d, text: %s.", ret, errno, strerror(errno));
                }
            }
        }
        */
        ret = sem_post((sem_t*)sem);
        if (ret) {
            spl_console_log("sem_post: ret: %d, errno: %d, text: %s.", ret, errno, strerror(errno));
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
    SPSERIAL_ROOT_TYPE* srl = &spserial_root_node;
    SPSERIAL_ARR_LIST_LINED* node = 0, * prev = 0, *next  = 0;;
    do {
        if (!obj) {

            break;
        }
        spserial_mutex_lock(srl->mutex);
        node = srl->init_node;
        while (node) {
            if (node->item->iidd == idd) {
                *obj = node;
                if (takeoff) {
                    prev = node->prev;
                    next = node->next;
                    if (!prev) {
                        if (!next) {
                            srl->init_node = 0;
                            srl->last_node = 0;
                        }
                        else {
                            srl->init_node = next;
                            srl->init_node->prev = 0;
                        }
                    }
                    else {
                        if (!next) {
                            prev->next = 0;
                            srl->last_node = prev;
                        }
                        else {
                            prev->next = next;
                            next->prev = prev;
                        }
                    }
                    srl->count--;
                }
                break;
            }
            node = node->next;
        }
        spserial_mutex_unlock(srl->mutex);
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
int spserial_clear_node(SPSERIAL_ARR_LIST_LINED* node) {
    int ret = 0;
    do {
        spserial_mutex_lock(node->item->mtx_off);
        node->item->isoff = 1;
        spserial_mutex_unlock(node->item->mtx_off);
        SetEvent(node->item->hEvent);
		spserial_wait_sem(node->item->sem_off);

        SPSERIAL_CloseHandle(node->item->mtx_off);
        SPSERIAL_CloseHandle(node->item->sem_off);
        spserial_free(node->item->buff);
        spserial_free(node->item);
        spserial_free(node);
    } while (0);
    return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spserial_inst_write_to_port(SP_SERIAL_INFO_ST* item, char* data, int sz) {
    int ret = 0;
    do {
        if (!item) {
            ret = SPSERIAL_INFO_NULL;
            break;
        }
        spserial_mutex_lock(item->mtx_off);
        do {
            if (item->buff) {
                if (item->buff->range > item->buff->pl + sz) {
                    memcpy(item->buff->data + item->buff->pl, data, sz);
                    item->buff->pl += sz;
                    break;
                }
                else {
                    int range = 0;
                    int total = 0;
                    int addition = 0;
                    SP_SERIAL_GENERIC_ST* tmp = 0;
                    addition = SPSERIAL_MAX_AB(sz, SPSERIAL_STEP_MEM);
                    range = item->buff->range;
                    total = item->buff->total;
                    tmp = (SP_SERIAL_GENERIC_ST*)realloc(item->buff, total + addition);
                    if (!tmp) {
                        ret = SPSERIAL_REALLOC_ERROR;
                        break;
                    }
                    item->buff = tmp;
                    item->buff->range = addition + range;
                    item->buff->total = addition + total;

                    memcpy(item->buff->data + item->buff->pl, data, sz);
                    item->buff->pl += sz;
                    break;
                }
            }
            else {
                int step = 0;
                step = SPSERIAL_MAX_AB(sz, SPSERIAL_STEP_MEM);
                step += sizeof(SP_SERIAL_GENERIC_ST);
                spserial_malloc(step, item->buff, SP_SERIAL_GENERIC_ST);
                if (!item->buff) {
                    ret = SPSERIAL_MALLOC_ERROR;
                    break;
                }
                item->buff->total = step;
                item->buff->range = item->buff->total - sizeof(SP_SERIAL_GENERIC_ST);
                memcpy(item->buff->data + item->buff->pl, data, sz);
                item->buff->pl += sz;
                break;
            }
        } while (0);
        spserial_mutex_unlock(item->mtx_off);
        SetEvent(item->hEvent);
    } while (0);
    return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

int spserial_inst_write_data(int idd, char* data, int sz) {
    int ret = 0;
    void* p = 0;
    SPSERIAL_ARR_LIST_LINED* node = 0;
    SP_SERIAL_INFO_ST* item = 0;
    do {
        ret = spserial_get_objbyid(idd, &p, 0);
        if (!p) {
            ret = SPSERIAL_NOT_FOUND_IDD;
            break;
        }
        node = (SPSERIAL_ARR_LIST_LINED*) p;
        item = node->item;
        spserial_mutex_lock(item->mtx_off);
            do {
                if (item->buff) {
                    if (item->buff->range > item->buff->pl + sz) {
                        memcpy(item->buff->data + item->buff->pl, data, sz);
                        item->buff->pl += sz;
                        break;
                    }
                    else {
                        int range = 0;
                        int total = 0;
                        int addition = 0; 
                        SP_SERIAL_GENERIC_ST* tmp = 0;
                        addition = SPSERIAL_MAX_AB(sz, SPSERIAL_STEP_MEM);
                        range = item->buff->range;
                        total = item->buff->total;
                        tmp = (SP_SERIAL_GENERIC_ST*)realloc(item->buff, total + addition);
                        if (!tmp) {
                            ret = SPSERIAL_REALLOC_ERROR;
                            break;
                        }
                        item->buff = tmp;
                        item->buff->range = addition + range;
                        item->buff->total = addition + total;

                        memcpy(item->buff->data + item->buff->pl, data, sz);
                        item->buff->pl += sz;
                        break;
                    }
                }
                else {
                    int step = 0;
                    step = SPSERIAL_MAX_AB(sz, SPSERIAL_STEP_MEM);
                    step += sizeof(SP_SERIAL_GENERIC_ST);
                    spserial_malloc(step, item->buff, SP_SERIAL_GENERIC_ST);
                    if (!item->buff) {
                        ret = SPSERIAL_MALLOC_ERROR;
                        break;
                    }
                    item->buff->total = step;
                    item->buff->range = item->buff->total - sizeof(SP_SERIAL_GENERIC_ST);
                    memcpy(item->buff->data + item->buff->pl, data, sz);
                    item->buff->pl += sz;
                    break;
                }
            } while (0);
        spserial_mutex_unlock(item->mtx_off);
        SetEvent(item->hEvent);
    } while (0);
    return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#ifndef UNIX_LINUX
#else
    int spserial_init_trigger(void* obj) { 
        return 0;
    }
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
    int spserial_pull_trigger(void* obj) { return 0;}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
    int spserial_start_listen(void* obj) { return 0;}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#endif
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
