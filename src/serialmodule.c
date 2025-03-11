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
    #include <stdlib.h> 
    #include <unistd.h> 
    #include <string.h> 
    #include <sys/types.h> 
    #include <sys/socket.h> 
    #include <arpa/inet.h> 
    #include <netinet/in.h> 
#ifdef __MACH__
#else
    #include <sys/epoll.h>
#endif	
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
    #define SPSR_MAXLINE                            1024
    #define SPSR_PORT_TRIGGER                       10024
    #define SPSR_PORT_CARTRIDGE                     (SPSR_PORT_TRIGGER + 10)
#endif

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#define SPSERIAL_MAX_AB(__a__, __b__)               ((__a__) > (__b__)) ? (__a__) : (__b__)
#define SPSERIAL_STEP_MEM                           1024
#define SPSERIAL_BUFFER_SIZE                        2048
#define SPSR_DATA_RANGE								1024

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
static int spsr_init_trigger(void*);
static int spserial_pull_trigger(void*);
static int spserial_start_listen(void*);
static int spserial_fetch_commands(int, char*, int n);
#endif
static int spsr_add2_list(SP_SERIAL_INFO_ST*);
static int spsr_remv_list(char *nameport);


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
static void* 
    spsr_init_trigger_routine(void*);
static void*
    spsr_init_cartridge_routine(void*);
static int
    spsr_send_cmd(int cmd, void* data);
#endif

static int 
    spserial_module_isoff(SP_SERIAL_INFO_ST* obj);

static int
    spserial_verify_info(SP_SERIAL_INPUT_ST* obj, SP_SERIAL_INFO_ST** output);

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

int spserial_inst_create(void *obj, SP_SERIAL_INFO_ST **output)
{
    int ret = 0;
    SPSERIAL_ROOT_TYPE* t = &spserial_root_node;
    SP_SERIAL_INPUT_ST* p = (SP_SERIAL_INPUT_ST*)obj;
    //SP_SERIAL_INPUT_ST* input_looper = 0;
    do {
        if (!output) {
            ret = SPSERIAL_OUTPUT_NULL;
            break;
        }
        /*-------------------------------------------------------------------*/
        spserial_mutex_lock(t->mutex);
        /*do {*/
            ret = spserial_verify_info(p, output);
        /*} while (0); */
        spserial_mutex_unlock(t->mutex);
        /*-------------------------------------------------------------------*/
        spserial_mutex_lock(t->mutex);
#ifndef UNIX_LINUX
#else
        /*do {*/
            ret = spsr_send_cmd(SPSR_CMD_ADD, 0);
        /*} while (0); */
#endif
        spserial_mutex_unlock(t->mutex);
        /*-------------------------------------------------------------------*/
    } while (0);

	return ret;
}
int spserial_inst_del(char* portname)
{    
    int ret = 0;
    spllog(0, "-------------  Delete port --------------------------------------------------------------- : %s.", portname);
#ifndef UNIX_LINUX
    void *p = 0;
    SPSERIAL_ARR_LIST_LINED* node = 0;
    do {
        ret = spserial_get_objbyid(portname, &p, 1);
        if (p) {
            spllog(0, "Delete port: %s.", portname);
            node = (SPSERIAL_ARR_LIST_LINED*)p;
            spserial_clear_node(node);
        }
    } while (0);
#else
        SPSERIAL_ROOT_TYPE* t = &spserial_root_node;
        spllog(0, "-------------  Delete port --------------------------------------------------------------- : %s.", portname);
        spserial_mutex_lock(t->mutex);
        /*do {*/
            
            ret = spsr_send_cmd(SPSR_CMD_REM, portname);
        /*} while (0); */
        spserial_mutex_unlock(t->mutex);
#endif       
	return ret;
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
         //dcbSerialParams.StopBits
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
         spllog(SPL_LOG_DEBUG, "Create hEvent: 0x%p.", p->hEvent);
         /* Set timeouts(e.g., read timeout of 500ms, write timeout of 500ms) */
         //timeouts.ReadIntervalTimeout = 500;
         //timeouts.ReadTotalTimeoutConstant = 500;
         //timeouts.ReadTotalTimeoutMultiplier = 500;
         //timeouts.WriteTotalTimeoutConstant = 500;
         //timeouts.WriteTotalTimeoutMultiplier = 500;

         timeouts.ReadIntervalTimeout = 50;
         timeouts.ReadTotalTimeoutConstant = 50;
         timeouts.ReadTotalTimeoutMultiplier = 10;
         timeouts.WriteTotalTimeoutConstant = 50;
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
    DWORD bytesRead = 0;
    DWORD bytesWrite = 0;
    //DWORD bRead = 0;
    while (1) {
        DWORD dwError = 0;
        int wrote = 0;
        DWORD dwEvtMask = 0, flags = 0;
        OVERLAPPED olReadWrite = { 0 };
        //OVERLAPPED olRrite = { 0 };
        BOOL rs = FALSE;
        BOOL wrs = FALSE;
        int count = 0, cbInQue = 0;
        char readBuffer[SPSERIAL_BUFFER_SIZE + 1];
        COMSTAT csta = { 0 };
        olReadWrite.hEvent = p->hEvent;
        flags = EV_RXCHAR | EV_BREAK | EV_RXFLAG | EV_DSR;
        //flags = EV_BREAK | EV_RXFLAG;

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
            memset(&olReadWrite, 0, sizeof(olReadWrite));
            olReadWrite.hEvent = p->hEvent;
            wrs = TRUE;
            spserial_mutex_lock(p->mtx_off);
                do {
                    if (!p->buff) {
                        break;
                    }
                    if (p->buff->pl < 1) {
                        break;
                    }
                    buf->pl = p->buff->pl;
                    memcpy(buf->data, p->buff->data, buf->pl);
                    p->buff->pl = 0;
                    
                } while (0);
            spserial_mutex_unlock(p->mtx_off);
            while (buf->pl > 0) 
            {
                bytesWrite = 0;
                memset(&olReadWrite, 0, sizeof(olReadWrite));
                olReadWrite.hEvent = p->hEvent;
                wrs = WriteFile(p->handle, buf->data, buf->pl, &bytesWrite, &olReadWrite);
                
                if (!wrs) {
                    DWORD wErr = GetLastError();
                    spllog(SPL_LOG_DEBUG, "WriteFile: %d", (int)wErr);
                    if (wErr == ERROR_IO_PENDING) {
                        DWORD dwWaitResult = WaitForSingleObject(p->hEvent, INFINITE);
                        if (dwWaitResult == WAIT_OBJECT_0) {
                            bytesWrite = 0;
                            if (GetOverlappedResult(p->handle, &olReadWrite, &bytesWrite, TRUE)) {
                                if (buf->pl == (int)bytesWrite) {
                                    spllog(SPL_LOG_DEBUG, "Write DONE, %d.", buf->pl);
                                }
                                else {
                                    spllog(SPL_LOG_ERROR, "Write Error, %d.", buf->pl);
                                }
                            }
                            else {
                                spllog(SPL_LOG_ERROR, "Write Error code, %d.", buf->pl);
                            }
                        }
                        else {
                            if (bytesRead == buf->pl) {
                                spllog(SPL_LOG_DEBUG, "Write DONE, %d.", buf->pl);
                            }
                            else {
                                spllog(SPL_LOG_ERROR, "Write Error, %d.", (int)GetLastError());
                            }
                        }
                    }
                }
                else {
                    if (buf->pl == (int)bytesWrite) {
                        spllog(SPL_LOG_DEBUG, "Write DONE, %d.", buf->pl);
                    }
                    else {
                        spllog(SPL_LOG_ERROR, "Write Error, %d.", (int)GetLastError());
                    }
                }
                buf->pl = 0; 
            }

            rs = SetCommMask(p->handle, flags);
            dwEvtMask = flags;
            memset(&olReadWrite, 0, sizeof(olReadWrite));
            olReadWrite.hEvent = p->hEvent;
            rs = WaitCommEvent(p->handle, &dwEvtMask, &olReadWrite);
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
                spllog(SPL_LOG_DEBUG, "WaitCommEvent OK");
            }
            memset(&csta, 0, sizeof(csta));
            ClearCommError(p->handle, &dwError, &csta);
            cbInQue = csta.cbInQue;
            if (!cbInQue) {
                WaitForSingleObject(p->hEvent, INFINITE);
                if (GetOverlappedResult(p->handle, &olReadWrite, &bytesRead, TRUE)) {
                }
                else {
                    PurgeComm(p->handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
                }
                continue;
            }
            //Sleep(1000);
            
            bytesRead = 0;
            memset(readBuffer, 0, sizeof(readBuffer));
            do {
                rs = ReadFile(p->handle, readBuffer, SPSERIAL_BUFFER_SIZE, &bytesRead, &olReadWrite);
                spllog(SPL_LOG_DEBUG, "olRead.InternalHigh: %d, olRead.Internal: %d, rs : %s!!!", 
                    (int)olReadWrite.InternalHigh, (int)olReadWrite.Internal, rs ? "true" : "false");
                if (!rs) {
                    BOOL rs1 = FALSE;
                    DWORD readErr = GetLastError();
                    if (readErr == ERROR_IO_PENDING) {
                        bytesRead = 0;
                        WaitForSingleObject(p->hEvent, INFINITE);
                        rs1 = GetOverlappedResult(p->handle, &olReadWrite, &bytesRead, 1);
                        if (rs1) {
                            spllog(SPL_LOG_DEBUG, "bRead: %d", (int)bytesRead);
                        }
                        else {
                            spllog(SPL_LOG_ERROR, "PurgeComm: %d", (int)GetLastError());
                            PurgeComm(p->handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
                        }
                        if (!bytesRead) {
                            /*PurgeComm(p->handle, PURGE_RXCLEAR | PURGE_TXCLEAR);*/
                        }
                    }
                    else {
                        spllog(SPL_LOG_ERROR, "Read error readErr: %d", (int)readErr);
                    }
                }
                else {
                    spllog(SPL_LOG_INFO, "Read OK");
                }
            } while (0);
            memset(&csta, 0, sizeof(csta));
            ClearCommError(p->handle, &dwError, &csta);
            
            if (csta.cbInQue > 0) {
                spllog(SPL_LOG_ERROR, "Read Com not finished!!!");
            }
            else if(bytesRead > 0)/*else start*/
            {
                spllog(SPL_LOG_DEBUG, " ---------------->>>>>>>>>>>>>>>>  [[[ %s, cbInQue: %d, bRead: %d ]]]", 
                    readBuffer, cbInQue, bytesRead);
                if (p->cb_evt_fn) 
                {
                    int nnnn = 1 + sizeof(SP_SERIAL_GENERIC_ST) + bytesRead + sizeof(void*);
                    SP_SERIAL_GENERIC_ST* evt = 0;
                    spserial_malloc(nnnn, evt, SP_SERIAL_GENERIC_ST);
                    
                    if (evt) 
                    {
                        evt->total = nnnn;
                        evt->type = SPSERIAL_EVENT_READ_BUF;
                        
                        evt->pc = sizeof(void*);
                        if (sizeof(void*) == 4) {
                            unsigned int tmp = (unsigned int)p->cb_obj;
                            memcpy((char*)evt->data, (char*)&tmp, evt->pc);
                        }
                        else  if (sizeof(void*) == 8) {
                            unsigned long long int tmp = (unsigned long long int)p->cb_obj;
                            memcpy((char*)evt->data, (char *)&tmp, evt->pc);
                        }
                        memcpy(evt->data + evt->pc, readBuffer, bytesRead);
                        evt->pl = evt->pc + bytesRead;
                        p->cb_evt_fn(evt);
                        spserial_free(evt);
                    }
                }
                /*p->cb end*/
            }
            bytesRead = 0;

        }

        p->is_retry = 1;
        if (isoff) {
            break;
        }
    }
    spserial_free(buf);
    SPSERIAL_CloseHandle(p->hEvent);
    SPSERIAL_CloseHandle(p->handle);
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
    SPSERIAL_ROOT_TYPE* t = &spserial_root_node;
#ifndef UNIX_LINUX
#else
    pthread_t idd = 0;
    int err = 0;
    int nsize = 0;
#endif
    do {

        t->mutex = spserial_mutex_create();
        if (!t->mutex) {
            ret = SPSERIAL_MTX_CREATE;
            break;
        }
        t->sem = spserial_sem_create();
        if (!t->sem) {
            ret = SPSERIAL_SEM_CREATE;
            break;
        }
#ifndef UNIX_LINUX
#else
        t->sem_spsr = spserial_sem_create();
        if (!t->sem_spsr) {
            ret = SPSERIAL_SEM_CREATE;
            break;
        }
/*
        ret = spsr_init_trigger(0);
        if (ret) {
            break;
        }
	    ret = spserial_start_listen(0);
        if (ret) {
            break;
        }
*/
        err = pthread_create(&idd, 0, spsr_init_trigger_routine, t);
        if (err) {
            ret = PSERIAL_CREATE_THREAD_ERROR;
            break;
        }

        idd = 0;
        err = pthread_create(&idd, 0, spsr_init_cartridge_routine, t);
        if (err) {
            ret = PSERIAL_CREATE_THREAD_ERROR;
            break;
        }
        //t->cmd_buff
        nsize = SPSERIAL_BUFFER_SIZE + sizeof(int);
        spserial_malloc(nsize, t->cmd_buff, SP_SERIAL_GENERIC_ST);
        if (!t->cmd_buff) {
            spllog(SPL_LOG_ERROR, "spserial_malloc error");
            exit(1);
        }
        t->cmd_buff->total = nsize;
        t->cmd_buff->range = SPSERIAL_BUFFER_SIZE;
#endif
        spllog(SPL_LOG_DEBUG, "spserial_module_init: DONE");

    } while (0);
    return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spserial_module_close() {
    SPSERIAL_ROOT_TYPE* t = &spserial_root_node;
#ifndef UNIX_LINUX
    SPSERIAL_CloseHandle(t->mutex);
    SPSERIAL_CloseHandle(t->sem);
#else
    spserial_mutex_lock(t->mutex);
    /*do {*/
        t->spsr_off = 1;
    /*} while (0);*/
    spserial_mutex_unlock(t->mutex);
    /*----------------------------------------*/
    while (1) {
        int is_off = 0;
        spserial_rel_sem(t->sem);
        /*t->sem_spsr*/
        spserial_wait_sem(t->sem_spsr);

        spserial_mutex_lock(t->mutex);
        /*do {*/
            is_off = t->spsr_off;
        /*} while (0);*/
        spserial_mutex_unlock(t->mutex);
        if (is_off > 2) {
            break;
        }
    }
#endif
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
                /**/
            }
            else {
                t->last_node->next = obj;
                t->last_node = obj;
                /*t->last_node->next = 0;*/
            }
            t->count++;
        /* } while (0);*/
        spserial_mutex_unlock(t->mutex);
        
        snprintf(obj->item->port_name, SPSERIAL_PORT_LEN, "%s", p->port_name);
        obj->item->baudrate = p->baudrate;
        obj->item->cb_evt_fn = p->cb_evt_fn;
        obj->item->cb_obj = p->cb_obj;
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
        ret = pthread_mutex_lock((pthread_mutex_t*)obj);
        if (ret) {
            spllog(SPL_LOG_ERROR, "pthread_mutex_lock: ret: %d, errno: %d, text: %s.",
                ret, errno, strerror(errno));
        }
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
        ret = pthread_mutex_unlock((pthread_mutex_t*)obj);
        if (ret) {
            spllog(SPL_LOG_ERROR, "pthread_mutex_unlock: ret: %d, errno: %d, text: %s.", 
                ret, errno, strerror(errno));
        }
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
            spllog(SPL_LOG_DEBUG, "sem_post: ret: %d, errno: %d, text: %s.", ret, errno, strerror(errno));
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
int spserial_get_obj_by_name(char* portname, void** obj, int takeoff) {
    int ret = 0;
    SPSERIAL_ROOT_TYPE* t = &spserial_root_node;
    SPSERIAL_ARR_LIST_LINED* node = 0, * prev = 0;;
    int found = 0;
    do {
        if (!obj) {

            break;
        }
        spserial_mutex_lock(t->mutex);
        node = t->init_node;
        while (node) {
            if ( strcmp(portname, node->item->port_name) == 0) {
                *obj = node;
                if (takeoff)
                {
                    if (node->item->iidd == t->init_node->item->iidd)
                    {
                        t->init_node = t->init_node->next;
                    }
                    else {
                        if (prev) {
                            prev->next = node->next;
                            if (!prev->next) {
                                t->last_node = prev;
                            }
                        }
                    }
                    t->count--;
                    if (t->count < 1) {
                        t->init_node = 0;
                        t->last_node = 0;
                    }
                }
                found = 1;
                break;
            }
            prev = node;
            node = node->next;
        }
        spserial_mutex_unlock(t->mutex);
    } while (0);

    if (!found) {
        spllog(0, "Cannot find port: %s", portname);
        ret = SPSERIAL_ITEM_NOT_FOUND;
    }

    return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spserial_get_objbyid(char* portname, void** obj, int takeoff) {
    int ret = 0;
    SPSERIAL_ROOT_TYPE* t = &spserial_root_node;
    SPSERIAL_ARR_LIST_LINED* node = 0, * prev = 0;;
    int found = 0;
    do {
        if (!obj) {

            break;
        }
        spserial_mutex_lock(t->mutex);
        node = t->init_node;
        while (node) {
            if (strcmp(node->item->port_name, portname) == 0) {
                *obj = node;
                if (takeoff) 
                {
                    if (node->item->iidd == t->init_node->item->iidd)
                    {
                        t->init_node = t->init_node->next;
                    }
                    else {
                        if (prev) {
                            prev->next = node->next;
                            if (!prev->next) {
                                t->last_node = prev;
                            }
                        }
                    }
                    t->count--;
                    if (t->count < 1) {
                        t->init_node = 0;
                        t->last_node = 0;
                    }
                }
                found = 1;
                break;
            }
            prev = node;
            node = node->next;
        }
        spserial_mutex_unlock(t->mutex);
    } while (0);

    if (!found) {
        spllog(0, "Cannot find port: %s", portname);
        ret = SPSERIAL_ITEM_NOT_FOUND;
    }

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
        spllog(SPL_LOG_DEBUG, "CreateThread error: %d", (int)GetLastError());
    }
#else
    pthread_t tidd = 0;
    ret = pthread_create(&tidd, 0, f, arg);
    if (ret) {
        ret = SPL_LOG_THREAD_PX_CREATE;
        spllog(SPL_LOG_DEBUG, "pthread_create: ret: %d, errno: %d, text: %s.", ret, errno, strerror(errno));
    }
#endif
    return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spserial_clear_node(SPSERIAL_ARR_LIST_LINED* node) {
    int ret = 0;
    int i = 0;
    int found = 0;
    SPSERIAL_ROOT_TYPE* t = &spserial_root_node;
    SPSERIAL_ARR_LIST_LINED* tnode = 0, * prev = 0;;
//    int iddd = 0;
    do {
        if (!node) {
            ret = SPSERIAL_PARAM_NULL;
            break;
        }
        spserial_mutex_lock(node->item->mtx_off);
        /*do {*/
            node->item->isoff = 1;
        /*} while (0); */
        spserial_mutex_unlock(node->item->mtx_off);
#ifndef UNIX_LINUX
        SetEvent(node->item->hEvent);
#else
#endif
        spserial_wait_sem(node->item->sem_off);

#ifndef UNIX_LINUX
        SPSERIAL_CloseHandle(node->item->mtx_off);
        SPSERIAL_CloseHandle(node->item->sem_off);
#else
#endif
        spserial_free(node->item->buff);

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
#ifndef UNIX_LINUX
		SetEvent(item->hEvent);
#else
#endif		
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
#ifndef UNIX_LINUX		
        SetEvent(item->hEvent);
#else
#endif		
    } while (0);
    return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#ifndef UNIX_LINUX
#else
#define SPSR_SIZE_CARTRIDGE         10
#define SPSR_SIZE_TRIGGER           2
#define SPSR_SIZE_MAX_EVENTS        10
#define SPSR_MSG_OFF        		"SPSR_MSG_OFF"
    void* spsr_init_trigger_routine(void* obj) {
        spsr_init_trigger(obj);
        return 0;
    }

    void* spsr_init_cartridge_routine(void* obj) {
        SPSERIAL_ROOT_TYPE* t = &spserial_root_node;
        int ret = 0;
        int epollfd = 0;;
        int sockfd = 0;
        int n = 0;
        int isoff = 0; 
        int flags = 0;
        socklen_t len = 0;
        char buffer[SPSR_MAXLINE + 1];
        const char* hello = "Hello from server";
        struct sockaddr_in cartridge_addr, client_addr;
        int i = 0;
		ssize_t lenmsg = 0;
        socklen_t client_len = sizeof(client_addr);

#ifdef __MACH__
#else
        struct epoll_event event, events[SPSR_SIZE_MAX_EVENTS];
        
#endif	

        spllog(SPL_LOG_DEBUG, "cartridge: ");
        /* Creating socket file descriptor */
		do {
			sockfd = socket(AF_INET, SOCK_DGRAM, 0);
			if (sockfd < 0) {
				spllog(SPL_LOG_ERROR, "fcntl: ret: %d, errno: %d, text: %s.", sockfd, errno, strerror(errno));
				ret = PSERIAL_CREATE_SOCK;
				break;
			}
	
			memset(&cartridge_addr, 0, sizeof(cartridge_addr));
	
			/* Filling server information */
	
			cartridge_addr.sin_family = AF_INET;
			/*
			cartridge_addr.sin_addr.s_addr = INADDR_ANY;
			
			*/
			cartridge_addr.sin_addr.s_addr = inet_addr("127.0.0.1");;
			cartridge_addr.sin_port = htons(SPSR_PORT_CARTRIDGE);
	
			/* Set socket to non - blocking mode */
	
            flags = fcntl(sockfd, F_GETFL, 0);
			if (flags == -1) {
				spllog(SPL_LOG_ERROR, "fcntl: ret: %d, errno: %d, text: %s.", ret, errno, strerror(errno));
				ret = PSERIAL_FCNTL_SOCK;
				break;
			}
			spllog(SPL_LOG_DEBUG, "fcntl------------------------flags: %d", flags);
			ret = fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
			if (ret == -1) {
				spllog(SPL_LOG_ERROR, "fcntl: ret: %d, errno: %d, text: %s.", ret, errno, strerror(errno));
				ret = PSERIAL_FCNTL_SOCK;
				break;
			}
	
			/* Bind the socket with the server address */
			spllog(SPL_LOG_DEBUG, "bind------------------------");
			ret = bind(sockfd, (const struct sockaddr*)&cartridge_addr,
				sizeof(cartridge_addr));
			if (ret < 0)
			{
				spllog(SPL_LOG_ERROR, "bind failed: ret: %d, errno: %d, text: %s.", ret, errno, strerror(errno));
				ret = PSERIAL_BIND_SOCK;
				break;
			}
	
			while (1) {
				/*
				spllog(SPL_LOG_DEBUG, "spserial_wait_sem------------------------");
				spserial_wait_sem(t->sem);
				*/
				spserial_mutex_lock(t->mutex);
				/*do {*/
					isoff = t->spsr_off;
				/*} while (0);*/
				spserial_mutex_unlock(t->mutex);
				if (isoff) {
					break;
				}
				/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
				
           #ifdef __MACH__

           #else
                /* Start epoll */
				spllog(SPL_LOG_DEBUG, "epoll_create------------------------");
                epollfd = epoll_create(SPSR_SIZE_CARTRIDGE);
                if (epollfd < 0) {
                    spllog(SPL_LOG_ERROR, "epoll_create, epollfd: %d, errno: %d, text: %s.",
                        epollfd, errno, strerror(errno));
                    ret = PSERIAL_EPOLL_CREATE;
                    break;
                }
                event.events = EPOLLIN | EPOLLET;
				event.data.fd = sockfd;
				
				spllog(SPL_LOG_DEBUG, "epollfd------------------------");
                ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event);
                if (ret < 0) {
                    spllog(SPL_LOG_ERROR, "epoll_ctl, ret: %d, errno: %d, text: %s.",
                        ret, errno, strerror(errno));
                    ret = PSERIAL_EPOLL_CTL;
                    break;
                }
                while (1) {
					if (isoff) {
						break;
					}					
					spllog(SPL_LOG_DEBUG, "epoll_wait------------------------");
                    int nfds = epoll_wait(epollfd, events, SPSR_SIZE_MAX_EVENTS, -1);
					spllog(SPL_LOG_DEBUG, "epoll_wait------------------------, nfds: %d", nfds);
                    for (i = 0; i < nfds; i++) 
                    {
						spllog(SPL_LOG_DEBUG, "(data.fd, sockfd)------------------------(%d, %d)", events[i].data.fd, sockfd);
                        if (events[i].data.fd == sockfd) 
                        {
							char *p = 0;
							while(1) {
								int lp = 0;
								memset(&client_addr, 0, sizeof(client_addr));
								client_len = sizeof(client_addr);
								memset(buffer, 0, sizeof(buffer));
								spllog(SPL_LOG_DEBUG, "recvfrom------------------------");
								lenmsg = recvfrom(sockfd, buffer, SPSR_MAXLINE, 0,
									(struct sockaddr*)&client_addr, &client_len);
								if (lenmsg < 1) {
									spllog(SPL_LOG_ERROR, "epoll_ctl, lenmsg: %d, errno: %d, text: %s.",
										(int)lenmsg, errno, strerror(errno));
									break;
								}
								
								buffer[lenmsg] = 0;
								spllog(SPL_LOG_DEBUG, "buffer: %s", buffer);
								if (strcmp(buffer, SPSR_MSG_OFF) == 0) {
									spllog(SPL_LOG_DEBUG, SPSR_MSG_OFF);
									isoff = 1;
									break;
								}
								if(isoff) {
									break;
								}
								lp = 0;
								spserial_mutex_lock(t->mutex);
								/*SPSERIAL_BUFFER_SIZE*/
								spserial_malloc(SPSERIAL_BUFFER_SIZE, p, char);
								    do {
								    	if(t->cmd_buff){
								    		lp = t->cmd_buff->pl;
								    		spllog(0, "lp: ================= %d.", lp);
								    		if(lp) {
								    			if(lp > SPSERIAL_BUFFER_SIZE) {
								    				p = realloc(p, lp);
								    			}
								    			/*
								    			spserial_malloc(lp, p, char);
								    			*/
								    			if(!p) {
								    				break;
								    			}
								    			memcpy(p, t->cmd_buff->data, lp);
								    			t->cmd_buff->pl = 0;
								    		}
								    	}
								    } while (0);
                                    ret = spserial_fetch_commands(epollfd, p, lp);
								spserial_mutex_unlock(t->mutex);	
								spllog(0, "lppppppppppppppppppppppppppp: %d", lp);
								
								spserial_free(p);
							}
							continue;
                        } 
						if (events[i].data.fd >= 0) {
							int nr = 0;
							int didread = 0;
							int comfd = events[i].data.fd;
							memset(buffer, 0, sizeof(buffer));
							while(1) {
								nr = (int)read(comfd, buffer + didread, sizeof(buffer) - didread -1);
								if(nr == -1) {
									break;
								}
								didread += nr;
							}
							buffer[didread] = 0;
							spllog(0, "------------>>> data read didread: %d: %s", didread, buffer);
							//buffer[nr] = 0;
                            //nr = write(comfd, buffer, didread);
                            //spllog(0, "------------>>> data read didwrite: %d: %s", nr, buffer);
						}
                        /*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
                    }
                    /*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
                }
           #endif	
				/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
			}
		} while(0);
        spserial_mutex_lock(t->mutex);
        /*do {*/
             t->spsr_off++;
        /*} while (0);*/
        spserial_mutex_unlock(t->mutex);
		if(sockfd > 0) {
			ret = close(sockfd);
			if (ret) {
				spllog(SPL_LOG_ERROR, "close: ret: %d, errno: %d, text: %s.", ret, errno, strerror(errno));
				ret = PSERIAL_CLOSE_SOCK;
            }
            else {
                spllog(SPL_LOG_DEBUG, "close socket done.");
            }
		}
#ifdef __MACH__
#else
        if (epollfd > -1) {

        }
#endif	

        spserial_rel_sem(t->sem_spsr);

        return 0;
    }

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

    int spsr_init_trigger(void* obj) { 
        SPSERIAL_ROOT_TYPE* t = &spserial_root_node;
        int ret = 0;
        int sockfd = 0;
        int n = 0;
        int isoff = 0;
        int flags = 0;
        socklen_t len = 0;
        char buffer[SPSR_MAXLINE];
        const char* hello = "Hello from server";
        struct sockaddr_in trigger_addr, cartridge_addr;
        spllog(SPL_LOG_DEBUG, "trigger: ");
        char had_cmd = 0;
        do {
        
            /* Creating socket file descriptor */
            if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                spllog(SPL_LOG_DEBUG, "fcntl: ret: %d, errno: %d, text: %s.", sockfd, errno, strerror(errno));
                ret = PSERIAL_CREATE_SOCK;
                break;
            }

            memset(&trigger_addr, 0, sizeof(trigger_addr));
            memset(&cartridge_addr, 0, sizeof(cartridge_addr));

            /* Filling server information */
            trigger_addr.sin_family = AF_INET; 
            trigger_addr.sin_addr.s_addr = inet_addr("127.0.0.1");;;
            trigger_addr.sin_port = htons(SPSR_PORT_TRIGGER);

            cartridge_addr.sin_family = AF_INET; 
            cartridge_addr.sin_addr.s_addr = inet_addr("127.0.0.1");;
            cartridge_addr.sin_port = htons(SPSR_PORT_CARTRIDGE);

            // Set socket to non-blocking mode
            ret = fcntl(sockfd, F_GETFL, 0);
            if (ret == -1) {
                spllog(SPL_LOG_DEBUG, "fcntl: ret: %d, errno: %d, text: %s.", ret, errno, strerror(errno));
                ret = PSERIAL_FCNTL_SOCK;
                break;
            }

            ret = fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
            if ( ret == -1) {
                spllog(SPL_LOG_DEBUG, "fcntl: ret: %d, errno: %d, text: %s.", ret, errno, strerror(errno));
                ret = PSERIAL_FCNTL_SOCK;
                break;
            }

            /* Bind the socket with the server address */
            ret = bind(sockfd, (const struct sockaddr*)&trigger_addr, sizeof(trigger_addr));
            if ( ret < 0)
            {
                /* perror("bind failed"); */
                spllog(SPL_LOG_ERROR, "bind failed: ret: %d, errno: %d, text: %s.", ret, errno, strerror(errno));
                ret = PSERIAL_BIND_SOCK;
                break;
            }
            len = sizeof(trigger_addr);
            while (1) {

                spserial_wait_sem(t->sem);

                spserial_mutex_lock(t->mutex);
                /*do {*/
                    isoff = t->spsr_off;
                    if (isoff) {
                        t->spsr_off++;
                    }
                    if (t->cmd_buff) {
                        if (t->cmd_buff->pl > 0) {
                            had_cmd = 1;
                        }
                    }
                /*} while (0);*/
                spserial_mutex_unlock(t->mutex);

                if (isoff) {
                    int kkk = sendto(sockfd, (const char*)SPSR_MSG_OFF, strlen(SPSR_MSG_OFF),
                        MSG_CONFIRM, (const struct sockaddr*)&cartridge_addr, len);
					spllog(SPL_LOG_DEBUG, "sendto kkk: %d", kkk);
                    break;
                }
                if (had_cmd) {
                    sendto(sockfd, (const char*)"CMD", strlen("CMD"),
                        MSG_CONFIRM, (const struct sockaddr*)&cartridge_addr, len);
                }
                had_cmd = 0;
                /*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
                /*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
            }



            ret = close(sockfd);
            if (ret) {
                spllog(SPL_LOG_ERROR, "close: ret: %d, errno: %d, text: %s.", ret, errno, strerror(errno));
                ret = PSERIAL_CLOSE_SOCK;
            }
            else {
                spllog(SPL_LOG_DEBUG, "close socket done.");
            }
			/* Clean linked list. TODO 2.*/
            spserial_rel_sem(t->sem_spsr);
        } 
        while (0);
        return 0;
    }
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spserial_fetch_commands(int epollfd, char* info,int n) {
	int ret = 0;
	SPSERIAL_ROOT_TYPE* t = &spserial_root_node;
	SPSERIAL_ARR_LIST_LINED * temp = 0;
	SP_SERIAL_GENERIC_ST* item = 0;;
	SP_SERIAL_GENERIC_ST* obj = (SP_SERIAL_GENERIC_ST*)info;;
	SP_SERIAL_INFO_ST *input = 0;
	int i = 0;
	int fd = 0;
	int rerr = 0;
	struct termios options = {0};
	struct epoll_event event = {0};
	int count = n/sizeof(SP_SERIAL_GENERIC_ST);
    int step = 0;
	spllog(0, "-------------------------------------------------------------------enterfetch command, n: %d", n);
	do {
		for(step = 0; step < n;) {
			item = (SP_SERIAL_GENERIC_ST*) (info + step);
            step += item->total;
			spllog(0, "-------------------------------------------------------------------CMD: %d", item->type);
			if(item->type == SPSR_CMD_ADD) {
				temp = t->init_node;
				spllog(0, "-------------------------------------------------------------------CMD_ADD");
				while(temp) {
					if(temp->item->handle > -1) {
						temp = temp->next;
						continue;
					}
					input = temp->item;
					fd = open(input->port_name, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
					if (fd == -1) {
						spllog(SPL_LOG_ERROR, "open port error, fd: %d, errno: %d, text: %s.", fd, errno, strerror(errno));
						ret = PSERIAL_UNIX_OPEN_PORT;
						break;
					}		
					spllog(0, "fd: %d, portname: %s", fd, input->port_name);
					memset(&options, 0, sizeof(options));
					rerr = tcgetattr(fd, &options);
					if ( rerr < 0) {
						spllog(SPL_LOG_ERROR, "tcgetattr error, fd: %d, errno: %d, text: %s.", fd, errno, strerror(errno));
						ret = PSERIAL_UNIX_GET_ATTR;
						break;
					}
			
					spllog(0, "fd: %d, portname: %s, rate: %d", fd, input->port_name, input->baudrate);
					cfsetispeed(&options, input->baudrate);
					cfsetospeed(&options, input->baudrate);			
					
					options.c_cflag &= ~PARENB;    
					options.c_cflag &= ~CSTOPB;    
					options.c_cflag &= ~CSIZE;
					options.c_cflag |= CS8;        
					options.c_cflag &= ~CRTSCTS;   
					options.c_cflag |= CREAD | CLOCAL; 	
					options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); 
					options.c_iflag &= ~(IXON | IXOFF | IXANY);         
					options.c_oflag &= ~OPOST;      
					
                    /*
					options.c_cc[VMIN]= 1;      
					options.c_cc[VTIME]= 0;      
					*/

					rerr = tcsetattr(fd, TCSANOW, &options);
					if (rerr < 0) {
						spllog(SPL_LOG_ERROR, "tcsetattr error, fd: %d, errno: %d, text: %s.", fd, errno, strerror(errno));
						ret = PSERIAL_UNIX_SET_ATTR;
						break;
					}
                    else {
                        spllog(0, "tcsetattr -------------> DONE.")
                    }
					memset(&event, 0, sizeof(event));
					event.events = EPOLLIN | EPOLLET;
					event.data.fd = fd;
					rerr = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
					if (rerr == -1) {
						spllog(SPL_LOG_ERROR, "epoll_ctl error, fd: %d, errno: %d, text: %s.", fd, errno, strerror(errno));
						ret = PSERIAL_UNIX_EPOLL_CTL;
						break;
					}
					temp->item->handle = fd;
                    int k = write(fd, "buffer", strlen("buffer"));
                    spllog(0, "write: %d", k);
					temp = temp->next;
				}
                continue;
			}
            if(item->type == SPSR_CMD_REM) {
                char *portname = 0;
                portname = item->data;
                temp = t->init_node;
                spllog(0, "----------------SPSR_CMD_REM-------------------pl: %d, portname: %s, total: %d, initnode: 0x%p", 
                    item->pl, portname, item->total, temp);                
                while(temp) {
                    spllog(0, "portname: %s", temp->item->port_name);
                    if( strcmp(temp->item->port_name, portname) == 0) {
                        if(temp->item->handle >= 0) {
                            spllog(0, ">>>>>>>>>>>>>>--handle: %d", temp->item->handle);
                        }
                        break;
                    }
                    temp = temp->next;
                } 
                continue;
            }
			if(ret) {
				break;
			}
		}
	} while(0);
	if(ret) {
		if(fd >= 0) {
			close(fd);
		}
	}
	return ret;
}

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spsr_send_cmd(int cmd, void* data) {
    int ret = 0;
    int nsize = 0;
    int* pend = 0;
    SPSERIAL_ROOT_TYPE* t = &spserial_root_node;
    do {
        if (cmd == SPSR_CMD_ADD ) {
            
            SP_SERIAL_GENERIC_ST *obj = 0;
            nsize = sizeof(SP_SERIAL_GENERIC_ST);
            
            if (t->cmd_buff->range > t->cmd_buff->pl + sizeof(obj)) {
                obj = (SP_SERIAL_GENERIC_ST *) (t->cmd_buff->data + t->cmd_buff->pl);
                memset(obj, 0, nsize);
                obj->total = nsize;
                obj->type = cmd;
                
                t->cmd_buff->pl += nsize;
                pend = (int*)(t->cmd_buff->data + t->cmd_buff->pl);
                *pend = 0;
                spllog(0, "cmd------------------------------------: %d, size: %d", obj->type, obj->total);
            }
            break;
        }
        if (cmd == SPSR_CMD_REM) {
            char *portname = (char *)data;
            int lport = 0;
            int len = strlen(portname);

            SP_SERIAL_GENERIC_ST *obj = 0;
            lport = len + 1;            
            nsize = sizeof(SP_SERIAL_GENERIC_ST) + lport;
            spllog(0, "SPSR_CMD_REM, nsize: %d, portname: %s", nsize, portname);
            if (t->cmd_buff->range > t->cmd_buff->pl + nsize) {
                obj = (SP_SERIAL_GENERIC_ST *) (t->cmd_buff->data + t->cmd_buff->pl);
                memset(obj, 0, nsize);
                obj->total = nsize;
                obj->type = cmd;
                obj->range = lport;
                memcpy(obj->data, portname, lport);
                obj->data[len] = 0;
                obj->pl = lport;
                t->cmd_buff->pl += nsize;
                pend = (int*)(t->cmd_buff->data + t->cmd_buff->pl);
                *pend = 0;
            }
            break;
        }
    } while (0);
    spserial_rel_sem(t->sem);
    return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
    int spserial_pull_trigger(void* obj) { return 0;}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
    int spserial_start_listen(void* obj) { return 0;}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#endif
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spsr_add2_list(SP_SERIAL_INFO_ST* input) 
{
	int ret = 0;
	SPSERIAL_ROOT_TYPE* t = &spserial_root_node;
	SPSERIAL_ARR_LIST_LINED *obj = 0;
    SP_SERIAL_INFO_ST* item = 0;
	do {
        /*do {*/
        spserial_malloc(sizeof(SPSERIAL_ARR_LIST_LINED), obj, SPSERIAL_ARR_LIST_LINED);
        if (!obj) {
            ret = 1;
            break;
        }
        spserial_malloc(sizeof(SP_SERIAL_INFO_ST), item, SP_SERIAL_INFO_ST);
        if (!item) {
            ret = 1;
            break;
        }

        memcpy(item, input, sizeof(SP_SERIAL_INFO_ST));
		/* Create Mutex here TODO */
        obj->item = item;
        if (!t->init_node) {
            t->init_node = obj;
            t->last_node = obj;
            /**/
        }
        else {
            t->last_node->next = obj;
            t->last_node = obj;
            /*t->last_node->next = 0;*/
        }
        t->count++;
        /* } while (0);*/
	} while(0);
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spsr_remv_list(char * portname)
{
    int ret = 0;
    SPSERIAL_ROOT_TYPE* t = &spserial_root_node;
    SPSERIAL_ARR_LIST_LINED* node = 0, * prev = 0;;
    int found = 0;
    do {
        if (!portname) {
            break;
        }
        node = t->init_node;
        while (node) {
            if (strcmp(portname, node->item->port_name) == 0) 
            {
                if (node->item->iidd == t->init_node->item->iidd)
                {
                    t->init_node = t->init_node->next;
                }
                else {
                    if (prev) {
                        prev->next = node->next;
                        if (!prev->next) {
                            t->last_node = prev;
                        }
                    }
                }
                t->count--;
                if (t->count < 1) {
                    t->init_node = 0;
                    t->last_node = 0;
                }
                found = 1;
                break;
            }
            prev = node;
            node = node->next;
        }
    } while (0);

    if (!found) {
        ret = SPSERIAL_ITEM_NOT_FOUND;
    }
    return ret;
} 
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spserial_verify_info(SP_SERIAL_INPUT_ST* p, SP_SERIAL_INFO_ST** output) {
    SPSERIAL_ROOT_TYPE* t = &spserial_root_node;
    int ret = 0;
    SP_SERIAL_INFO_ST* item = 0;
    SPSERIAL_ARR_LIST_LINED* node = 0;
    spllog(SPL_LOG_DEBUG, "spserial_verify_info:");
#ifndef UNIX_LINUX
    HANDLE hSerial = 0;
#else
    int fd = 0;
#endif
    do {
        spllog(SPL_LOG_DEBUG, "spserial_verify_info:");
        if (!output) {
            ret = SPSERIAL_OUTPUT_NULL;
            break;
        }
        spllog(SPL_LOG_DEBUG, "spserial_verify_info:");
        if (!p) {
            ret = SPSERIAL_PORT_INPUT_NULL;
            break;
        }
        spllog(SPL_LOG_DEBUG, "spserial_verify_info:");
        if (p->baudrate < 1) {
            ret = SPSERIAL_PORT_BAUDRATE_ERROR;
            break;
        }
        spllog(SPL_LOG_DEBUG, "spserial_verify_info:");
        if (!p->port_name[0]) {
            spllog(SPL_LOG_DEBUG, "spserial_verify_info, portname: %s", p->port_name);
            ret = SPSERIAL_PORT_NAME_ERROR;
            break;
        }
        spllog(SPL_LOG_DEBUG, "spserial_verify_info:");
        if (t->init_node) 
        {
            SPSERIAL_ARR_LIST_LINED* tmp = 0;
            tmp = t->init_node;
            while (tmp) 
            {
                if (strcmp(tmp->item->port_name, p->port_name) == 0) {
                    spllog(SPL_LOG_DEBUG, "did existed port_name: \"%s\".", p->port_name);
                    ret = PSERIAL_PORTNAME_EXISTED;
                    break;
                }
                tmp = tmp->next;
            }
        }
        spllog(SPL_LOG_DEBUG, "spserial_verify_info:");
        if (ret) 
        {
            break;
        }

#ifndef UNIX_LINUX
        /* Open the serial port with FILE_FLAG_OVERLAPPED for asynchronous operation */
        /* https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea */
        hSerial = CreateFile(p->port_name,
            GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);

        if (hSerial == INVALID_HANDLE_VALUE) {
            DWORD dwError = GetLastError();
            spllog(SPL_LOG_ERROR, "Open port errcode: %lu", dwError);
            ret = SPSERIAL_PORT_OPEN;
            break;
        }
        else {
            spllog(SPL_LOG_DEBUG, "Create hSerial: 0x%p.", hSerial);
            SPSERIAL_CloseHandle(hSerial);
        }
#else
        fd = open(p->port_name, O_RDWR | O_NOCTTY | O_NDELAY);
        if (fd == -1) {
            ret = SPSERIAL_PORT_OPEN_UNIX;
            spllog(SPL_LOG_ERROR, "open port: ret: %d, errno: %d, text: %s.", ret, errno, strerror(errno));
            break;
        }
        spllog(SPL_LOG_DEBUG, "open portname: %s, fd: %d.", p->port_name,fd);
        ret = close(fd);
        if (ret) {
            ret = SPSERIAL_PORT_CLOSE_UNIX;
            spllog(SPL_LOG_ERROR, "close port fd: %d, ret: %d, errno: %d, text: %s.", fd, ret, errno, strerror(errno));
            break;
        }
#endif

        spserial_malloc(sizeof(SPSERIAL_ARR_LIST_LINED), node, SPSERIAL_ARR_LIST_LINED);
        if (!node) {
            ret = SPSERIAL_MEM_NULL;
            spllog(SPL_LOG_ERROR, "SPSERIAL_MEM_NULL");
            break;
        }
        spserial_malloc(sizeof(SP_SERIAL_INFO_ST), item, SP_SERIAL_INFO_ST);
        if (!item) {
            ret = SPSERIAL_MEM_NULL;
            spllog(SPL_LOG_ERROR, "SPSERIAL_MEM_NULL");
            break;
        }
        
        /*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

        snprintf(item->port_name, SPSERIAL_PORT_LEN, "%s", p->port_name);
        item->baudrate = p->baudrate;
        item->cb_evt_fn = p->cb_evt_fn;
        item->cb_obj = p->cb_obj;
        node->item = item;
        *output = item;
        /*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

        if (!t->init_node) {
            t->init_node = node;
            t->last_node = node;
            spllog(SPL_LOG_DEBUG, "------------------------------------------------------>>>> t->init_node: 0x%p", t->init_node);
        }
        else {
            t->last_node->next = node;
            t->last_node = node;
            /*t->last_node->next = 0;*/
        }
#ifndef UNIX_LINUX
        ret = spserial_create_thread(spserial_thread_operating_routine, node);
#else
        /* TODO; */
        item->handle = -1;
        spllog(SPL_LOG_DEBUG, "--------------------------->>>> t->init_node: 0x%p", t->init_node);

#endif


    } while (0);

    if (ret) {
        if (item) {
            spserial_free(item);
        }
        if (node) {
            spserial_free(node);
        }
    }
    return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#ifndef UNIX_LINUX
#else
#endif
