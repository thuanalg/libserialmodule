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
#ifndef __SPSR_EPOLL__
	#include <poll.h>
#else
    #include <sys/epoll.h>
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
	#ifndef __SPSR_EPOLL__
		static int spserial_fetch_commands(void *, int *,char*, int n);
	#else
		static int spserial_fetch_commands(int, char*, int n);
	#endif

#endif

static int spsr_clear_all();
void thuan() { }

static SPSERIAL_ROOT_TYPE
    spserial_root_node;

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

#ifndef UNIX_LINUX
static int 
    spserial_clear_node(SPSERIAL_ARR_LIST_LINED *);
static int 
    spserial_module_isoff(SP_SERIAL_INFO_ST* obj);
static int
    spserial_module_openport(void*);
static DWORD WINAPI
    spserial_thread_operating_routine(LPVOID lpParam);
static int
    spserial_create_thread(SP_SERIAL_THREAD_ROUTINE f, void* arg);
static int 
    spsr_get_obj(char* portname, void** obj, int takeoff);
#else
static void* 
    spsr_init_trigger_routine(void*);
static void*
    spsr_init_cartridge_routine(void*);
static int
    spsr_send_cmd(int cmd, char *portname, void* data, int lendata);
    
#define SPSR_MAX_NUMBER_OF_PORT     10

static void *spsr_hash_fd_arr[SPSR_MAX_NUMBER_OF_PORT];
//static void *spsr_hash_name_arr[SPSR_MAX_NUMBER_OF_PORT];


typedef struct __SPSR_HASH_FD_NAME__ {
    int fd;
    char port_name[SPSERIAL_PORT_LEN];
    SPSERIAL_module_cb cb_evt_fn;
    void* cb_obj;    
    struct __SPSR_HASH_FD_NAME__ *next;
    int t_delay;
} SPSR_HASH_FD_NAME;
#define SPSR_HASH_FD(__fd__)    (__fd__%SPSR_MAX_NUMBER_OF_PORT)
//static int spsr_hash_port(char* port, int len);
static int spsr_clear_hash();
static int spsr_open_fd(char *port_name, int brate, int*);
static int spsr_read_fd(int fd, char *buffer, int n, char *chk_delay);
#endif

static int
    spserial_verify_info(SP_SERIAL_INPUT_ST* obj);

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
static int 
	spsr_invoke_cb(SPSERIAL_module_cb fn_cb, void *obj, char *data, int len);
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

int spsr_inst_open(SP_SERIAL_INPUT_ST *p)
{
    int ret = 0;
    SPSERIAL_ROOT_TYPE* t = &spserial_root_node;
    //SP_SERIAL_INPUT_ST* input_looper = 0;
    do {
        //if (!output) {
        //    ret = SPSERIAL_OUTPUT_NULL;
        //    break;
        //}
        /*-------------------------------------------------------------------*/
        spserial_mutex_lock(t->mutex);
        /*do {*/
            ret = spserial_verify_info(p);
        /*} while (0); */
        spserial_mutex_unlock(t->mutex);
        /*-------------------------------------------------------------------*/
        if(ret) {
            spllog(SPL_LOG_ERROR, "==================>>> ret error: %d.", ret);
            break;
        }
        spserial_mutex_lock(t->mutex);
#ifndef UNIX_LINUX
#else
        /*do {*/
            ret = spsr_send_cmd(SPSR_CMD_ADD, 0, 0, 0);
        /*} while (0); */
#endif
        spserial_mutex_unlock(t->mutex);
        /*-------------------------------------------------------------------*/
    } while (0);

	return ret;
}

int spsr_inst_close(char* portname)
{    
    int ret = 0;
    spllog(0, "-------------  Delete port --------------------------------------------------------------- : %s.", portname);
#ifndef UNIX_LINUX
    void *p = 0;
    SPSERIAL_ARR_LIST_LINED* node = 0;
    do {
        ret = spsr_get_obj(portname, &p, 1);
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
            ret = spsr_send_cmd(SPSR_CMD_REM, portname, 0, 0);
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
        hSerial = CreateFile(p->port_name, GENERIC_READ | GENERIC_WRITE,
                            0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);                         

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

        // Enable hardware flow control (RTS/CTS)
        dcbSerialParams.fOutxCtsFlow = TRUE;    // Enable CTS output flow control
        //dcbSerialParams.fCtsHandshake = TRUE;   // Enable CTS handshake
        dcbSerialParams.fOutxDsrFlow = FALSE;   // Disable DSR output flow control
        dcbSerialParams.fDsrSensitivity = FALSE;// DSR sensitivity disabled
        dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE; // Enable DTR
        dcbSerialParams.fRtsControl = RTS_CONTROL_ENABLE; // Enable RTS

        // Enable software flow control (XON/XOFF)
        dcbSerialParams.fInX = TRUE;    // Enable XON/XOFF input flow control
        dcbSerialParams.fOutX = TRUE;   // Enable XON/XOFF output flow control         

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
                        if(p->t_delay > 0) {
                            Sleep(p->t_delay);
                        }
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
                    ret = spsr_invoke_cb(p->cb_evt_fn, p->cb_obj, readBuffer, bytesRead);
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
int spsr_module_init() {
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
        spllog(SPL_LOG_DEBUG, "spsr_module_init: DONE");

    } while (0);
    return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spsr_module_finish() {
    SPSERIAL_ROOT_TYPE* t = &spserial_root_node;
#ifndef UNIX_LINUX
    spsr_clear_all();
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
#endif
    do {
        if (!sem) {
            ret = SPSERIAL_SEM_NULL_ERROR;
            break;
        }
#ifndef UNIX_LINUX
        ReleaseSemaphore(sem, 1, 0);
#else

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
#ifndef UNIX_LINUX
int spsr_get_obj(char* portname, void** obj, int takeoff) {
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
                    if (prev) {
                        prev->next = node->next;
                        if (!prev->next) {
                            t->last_node = prev;
                        }
                    }
                    else {
                        t->init_node = t->init_node->next;
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
#else
#endif

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

#ifndef UNIX_LINUX
int spserial_create_thread(SP_SERIAL_THREAD_ROUTINE f, void* arg) {
    int ret = 0;
    DWORD dwThreadId = 0;
    HANDLE hThread = 0;
    hThread = CreateThread(NULL, 0, f, arg, 0, &dwThreadId);
    if (!hThread) {
        ret = SPSERIAL_THREAD_W32_CREATE;
        spllog(SPL_LOG_DEBUG, "CreateThread error: %d", (int)GetLastError());
    }
    return ret;
}
#else
#endif
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#ifndef UNIX_LINUX
int spserial_clear_node(SPSERIAL_ARR_LIST_LINED* node) {
    int ret = 0;
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
        SetEvent(node->item->hEvent);
        spserial_wait_sem(node->item->sem_off);

        SPSERIAL_CloseHandle(node->item->mtx_off);
        SPSERIAL_CloseHandle(node->item->sem_off);
        spserial_free(node->item->buff);

    } while (0);
    return ret;
}
#endif
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spsr_inst_write(char* portname, char*data, int sz) {
    int ret = 0;
    do {
#ifndef UNIX_LINUX
        SPSERIAL_ARR_LIST_LINED* node = 0;
        SP_SERIAL_INFO_ST* item = 0;
        void* p = 0;
        ret = spsr_get_obj(portname, &p, 0);
        if (!p) {
            ret = SPSERIAL_NOT_FOUND_IDD;
            spllog(SPL_LOG_ERROR, "Cannot find the object.");
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
#else
        SPSERIAL_ROOT_TYPE* t = &spserial_root_node;
        spserial_mutex_lock(t->mutex);
            ret = spsr_send_cmd(SPSR_CMD_WRITE, portname, data, sz);
        spserial_mutex_unlock(t->mutex);
        if(ret) {
            spllog(SPL_LOG_ERROR, "SEND command error ret: %d.", ret);
        }
#endif
    } while(0);
    return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#ifndef UNIX_LINUX
#else
#define SPSR_SIZE_CARTRIDGE         10
#define SPSR_SIZE_TRIGGER           2
#define SPSR_SIZE_MAX_EVENTS        10
#define SPSR_MSG_OFF        		"SPSR_MSG_OFF"
#define SPSR_MILLION        		1000000
    void* spsr_init_trigger_routine(void* obj) {
        spsr_init_trigger(obj);
        return 0;
    }

    void* spsr_init_cartridge_routine(void* obj) {
        SPSERIAL_ROOT_TYPE* t = &spserial_root_node;
        int ret = 0;
        
        int sockfd = 0;
        
        int isoff = 0; 
        int flags = 0;
        //socklen_t len = 0;
        char buffer[SPSR_MAXLINE + 1];
        //const char* hello = "Hello from server";
        struct sockaddr_in cartridge_addr, client_addr;
        int k  = 0;
		ssize_t lenmsg = 0;
        socklen_t client_len = sizeof(client_addr);
        

#ifndef __SPSR_EPOLL__
    int n = 0;
    
    int mx_number = 0;
    struct pollfd fds[SPSR_SIZE_MAX_EVENTS];
    memset(&fds, 0, sizeof(fds));
    for(n = 0; n < SPSR_SIZE_MAX_EVENTS; ++n) {
        fds[n].fd = -1;
    }
    n = 0;
#else
    int epollfd = 0;;
    int i = 0;
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
                char chk_delay = 0;
				/*
				spllog(SPL_LOG_DEBUG, "spserial_wait_sem------------------------");
				spserial_wait_sem(t->sem);
				*/
				
				k = 0;
				spserial_mutex_lock(t->mutex);
				/*do {*/
					isoff = t->spsr_off;
				/*} while (0);*/
				spserial_mutex_unlock(t->mutex);
				if (isoff) {
					break;
				}
				/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
				
#ifndef __SPSR_EPOLL__

				fds[0].fd = sockfd;  
				fds[0].events = POLLIN;  
                mx_number = 1;
				while(1) {
					if (isoff) {
						break;
					}
                    chk_delay = 0;
					ret = poll(fds, mx_number, 60 * 1000);
                    spllog(SPL_LOG_DEBUG, "poll,  mx_number: %d", mx_number);
					if(ret == -1) {
						continue;
					}
					if(ret == 0) {
						continue;
					}
					for(k = 0; k < mx_number; ++k) {
                        if(fds[k].fd < 0) {
                            continue;
                        }
						if (!(fds[k].revents & POLLIN)) {
							continue;
						}

						if(k == 0) {
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
                                    if(errno != 11) {
									    spllog(SPL_LOG_ERROR, "mach recvfrom, lenmsg: %d, errno: %d, text: %s.",
										    (int)lenmsg, errno, strerror(errno));
                                    }
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
                                spserial_malloc(SPSERIAL_BUFFER_SIZE, p, char);
								spserial_mutex_lock(t->mutex);
								    /*SPSERIAL_BUFFER_SIZE*/
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

								spserial_mutex_unlock(t->mutex);	

                                ret = spserial_fetch_commands(fds, &mx_number, p, lp);
								spllog(0, "lppppppppppppppppppppppppppp: %d", lp);
                
								spserial_free(p);
							}							
							continue;
						}
						if (fds[k].fd >= 0) {      
                            ret = spsr_read_fd(fds[k].fd, buffer, SPSR_MAXLINE + 1, &chk_delay);
						}                        
					}
					
				}
           #else
                /* Start epoll */
				spllog(SPL_LOG_DEBUG, "epoll_create------------------------");
                epollfd = epoll_create1(0);
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
					//spllog(SPL_LOG_DEBUG, "epoll_wait------------------------");
                    chk_delay = 0;
                    int nfds = epoll_wait(epollfd, events, SPSR_SIZE_MAX_EVENTS, -1);
					//spllog(SPL_LOG_DEBUG, "epoll_wait------------------------, nfds: %d", nfds);
                    for (i = 0; i < nfds; i++) 
                    {
						//spllog(SPL_LOG_DEBUG, "(data.fd, sockfd)------------------------(%d, %d)", events[i].data.fd, sockfd);
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
                                    if(errno != 11) {
									    spllog(SPL_LOG_ERROR, "epoll_ctl, lenmsg: %d, errno: %d, text: %s.",
										    (int)lenmsg, errno, strerror(errno));
                                    }
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
								    		if(lp) {
								    			if(lp > SPSERIAL_BUFFER_SIZE) {
								    				p = realloc(p, lp);
								    			}
								    			if(!p) {
								    				break;
								    			}
								    			memcpy(p, t->cmd_buff->data, lp);
								    			t->cmd_buff->pl = 0;
								    		}
								    	}
								    } while (0);
                                    
								spserial_mutex_unlock(t->mutex);	

								ret = spserial_fetch_commands(epollfd, p, lp);

								spserial_free(p);
							}
							continue;
                        } 
						if (events[i].data.fd >= 0) {
                            spllog(0, "======================================================before read");
                            ret = spsr_read_fd(events[i].data.fd, buffer, SPSR_MAXLINE + 1, &chk_delay);
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
             spsr_clear_all();
        /*} while (0);*/
        spserial_mutex_unlock(t->mutex);
        spsr_clear_hash();
		if(sockfd > 0) {
			ret = close(sockfd);
			if (ret) {
				spllog(SPL_LOG_ERROR, "close: ret: %d, errno: %d, text: %s.", ret, errno, strerror(errno));
				ret = PSERIAL_CLOSE_SOCK;
            }
            else {
                spllog(SPL_LOG_DEBUG, "close socket done: %d", sockfd);
            }
		}
#ifndef __SPSR_EPOLL__

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
        //int n = 0;
        int isoff = 0;
        int flags = 0;
        socklen_t len = 0;
        //char buffer[SPSR_MAXLINE];
        //const char* hello = "Hello from server";
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
                spllog(SPL_LOG_DEBUG, "----------close socket done: %d.", sockfd);
            }
			/* Clean linked list. TODO 2.*/
            spserial_rel_sem(t->sem_spsr);
        } 
        while (0);
        return 0;
    }
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

#ifndef __SPSR_EPOLL__
int spserial_fetch_commands(void *mp, int *prange, char* info,int n)
#else
int spserial_fetch_commands(int epollfd, char* info,int n) 
#endif
{
	int ret = 0;
	SPSERIAL_ROOT_TYPE* t = &spserial_root_node;
	SPSERIAL_ARR_LIST_LINED * temp = 0, *prev = 0;
	SP_SERIAL_GENERIC_ST* item = 0;;
	SP_SERIAL_INFO_ST *input = 0;
	int fd = 0;

#ifndef __SPSR_EPOLL__
    int i = 0;
    struct pollfd *fds = (struct pollfd*)mp;
#else
	struct epoll_event event = {0};
    int rerr = 0;
#endif   
    int step = 0;
	spllog(0, "-------------------------------------------------------------------enterfetch command, n: %d", n);
	do 
    {
		for(step = 0; step < n;) 
        {
			item = (SP_SERIAL_GENERIC_ST*) (info + step);
            step += item->total;
			spllog(0, "-------------------------------------------------------------------CMD: %d", item->type);
			if(item->type == SPSR_CMD_ADD) {
                spserial_mutex_lock(t->mutex);
                do {                   
				    temp = t->init_node;
				    spllog(0, "-------------------------------------------------------------------CMD_ADD");
				    while(temp) {
				    	if(temp->item->handle > -1) {
				    		temp = temp->next;
				    		continue;
				    	}
				    	input = temp->item;
                        fd = -1;
                        ret = spsr_open_fd(input->port_name, input->baudrate, &fd);
                        if(ret) {
                            break;
                        }
                    #ifndef __SPSR_EPOLL__
                        spllog(0, "*prange: %d -------------> DONE.", *prange);
                        for(i = 0; i < (*prange + 1); ++i) {
                            spllog(0, "*prange: %d -------------> fds[%d].fd: %d .", *prange, i, fds[i].fd);
                            if(fds[i].fd < 0) {
                                fds[i].fd = fd;
                                fds[i].events = POLLIN;  
                                (*prange)++;
                                spllog(0, "Add to poll list, index: %d, fd: %d, range: %d", 
                                    i, fd, (*prange));
                                break;
                            }
                        }
                        spllog(0, "*prange: %d -------------> DONE.", *prange);
                    #else
                        memset(&event, 0, sizeof(event));
                        event.events = EPOLLIN | EPOLLET ;
                        event.data.fd = fd;
                        rerr = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
                        if (rerr == -1) {
                            spllog(SPL_LOG_ERROR, "epoll_ctl error, fd: %d, errno: %d, text: %s.", fd, errno, strerror(errno));
                            ret = PSERIAL_UNIX_EPOLL_CTL;
                            break;
                        }                
                    #endif  
                        do {
                            SPSR_HASH_FD_NAME *hashobj = 0, *hashitem = 0;
                            int hashid = 0;
                            spserial_malloc(sizeof(SPSR_HASH_FD_NAME), hashobj, SPSR_HASH_FD_NAME);
                            if(!hashobj) {
                                break;
                            }
                            hashobj->fd = fd;
                            memcpy(hashobj->port_name, temp->item->port_name, strlen(temp->item->port_name));
                            hashobj->cb_evt_fn = temp->item->cb_evt_fn;
                            hashobj->cb_obj = temp->item->cb_obj; 
                            hashobj->t_delay = temp->item-> t_delay;                      
                            hashid = SPSR_HASH_FD(fd);
                            hashitem = (SPSR_HASH_FD_NAME *)spsr_hash_fd_arr[hashid];
                            if(!hashitem) {
                                spsr_hash_fd_arr[hashid] = (void*) hashobj;
                            } else {
                                SPSR_HASH_FD_NAME *temp = 0;
                                temp = hashobj;
                                while(!temp->next) {
                                    temp = temp->next;
                                }
                                temp->next = hashitem;
                            }

                        } while(0);               

                        temp->item->handle = fd;
				    	temp = temp->next;
				    }
                } while(0);
                spserial_mutex_unlock(t->mutex);
                continue;
			}
            if(item->type == SPSR_CMD_REM) {
                char *portname = 0;
                portname = item->data;
                spserial_mutex_lock(t->mutex);
                do {
                    temp = t->init_node;
                    spllog(0, "----------------SPSR_CMD_REM-------------------pl: %d, portname: %s, total: %d, initnode: 0x%p", 
                        item->pl, portname, item->total, temp);                
                    while(temp) 
                    {
                        spllog(0, "portname: %s", temp->item->port_name);
                        if( strcmp(temp->item->port_name, portname) == 0) {
                            if(temp->item->handle >= 0) {
                                int fd = temp->item->handle;
                                int errr = 0;
                                spllog(0, ">>>>>>>>>>>>>>--handle: %d", fd);
                                /* Remove fd out of epoll*/
                            #ifndef __SPSR_EPOLL__
                               for(i = 1; i < *prange; ++i) {
                                    if(fds[i].fd == fd) {
                                        int j = 0;
                                        for(j = i; j < (*prange -1); ++j) {
                                            fds[j].fd = fds[j+1].fd;
                                        }
                                        fds[(*prange -1)].fd = -1;
                                        (*prange)--;
                                        spllog(SPL_LOG_DEBUG, "EPOLL_CTL_DEL, fd: %d DONE", fd); 
                                        break;
                                    }
                                }                           
                            #else
                                errr = epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
                                if(errr == -1) {
                                    spllog(SPL_LOG_ERROR, "epoll_ctl error, fd: %d, errno: %d, text: %s.", fd, errno, strerror(errno)); 
                                } else {
                                    spllog(SPL_LOG_DEBUG, "EPOLL_CTL_DEL, fd: %d DONE", fd); 
                                }                        
                            #endif                           
                                /* Close handle*/
                                do {
                                    SPSR_HASH_FD_NAME *hashobj = 0, *temp = 0, *prev = 0;
                                    int hashid = SPSR_HASH_FD(fd);
                                    hashobj = (SPSR_HASH_FD_NAME *) spsr_hash_fd_arr[hashid];
                                    if(!hashobj) {
                                        spllog(SPL_LOG_ERROR, "Cannot find object."); 
                                        break;
                                    }
                                    temp = hashobj;
                                    while(temp) {
                                        if(temp->fd == fd) {
                                            if(prev) {
                                                prev->next = temp->next;
                                            } else {
                                                spsr_hash_fd_arr[hashid] = temp->next;
                                            }
                                            spllog(SPL_LOG_DEBUG, "--------------Clear from spsr_hash_fd_arr, hashid:%d, fd: %d.", hashid, fd); 
                                            spserial_free(temp);
                                            break;
                                        }
                                        prev = temp;
                                        temp = temp->next;
                                    }
                                } while(0);
                                /* Close handle*/
                                errr = close(fd);
                                if(errr) {
                                    spllog(SPL_LOG_ERROR, "close error, fd: %d, errno: %d, text: %s.", fd, errno, strerror(errno)); 
                                } else {
                                    spllog(SPL_LOG_DEBUG, "close, fd: %d, DONE", fd); 
                                }
                                /* Remove out of root list*/
                                if(t->count < 2) {
                                    t->init_node = 0;
                                    t->last_node = 0;
                                }
                                else {
                                    if(prev){
                                        prev->next = temp->next;
                                        if(!prev->next) {
                                            t->last_node = prev;
                                        }
                                    } else {
                                        t->init_node = temp->next;
                                    }
                                }
                                spserial_free(temp->item);
                                spserial_free(temp);
                                t->count--;
                                spllog(SPL_LOG_DEBUG, "t->count: %d", t->count); 
                            }
                            break;
                        }
                        prev = temp;
                        temp = temp->next;
                    } 
                } while(0);
                spserial_mutex_unlock(t->mutex);
                continue;
            }
            if(item->type == SPSR_CMD_WRITE) {
                char *portname = 0;
                fd = -1;
                portname = item->data;
                spserial_mutex_lock(t->mutex);
                do {
                    temp = t->init_node;                
                    spllog(0, "----------------SPSR_CMD_WRITE-------------------pl: %d, portname: %s, total: %d, initnode: 0x%p", 
                        item->pl, portname, item->total, temp);   
                    while(temp) {
                        spllog(0, "portname: %s", temp->item->port_name);
                        if( strcmp(temp->item->port_name, portname) == 0) {
                            if(temp->item->handle >= 0) {
                                fd = temp->item->handle;                    
                            }
                            break;
                        }
                        temp = temp->next;
                    }  
                } while(0);
                spserial_mutex_unlock(t->mutex);

                if(fd >= 0) {
                    int nwrote = 0, wlen = 0;;         
                    char *p = 0;
                    p = item->data + item->pc;
                    wlen = item->pl - item->pc;
                    if (tcflush(fd, TCIOFLUSH) == -1) {
                        spllog(SPL_LOG_ERROR, "Error flushing the serial port buffer");
                    } else {
                        spllog(SPL_LOG_DEBUG, "tcdrain DONE,");
                    }                               

                    nwrote = write(fd, p, wlen);
                    if(nwrote < 0) {
                        spllog(SPL_LOG_ERROR, "write error, fd: %d, errno: %d, text: %s.", fd, errno, strerror(errno)); 
                    } else {
                        spllog(SPL_LOG_DEBUG, "write DONE, fd: %d, nwrote: %d, p: %s, wlen: %d.", 
                            fd, nwrote, p, wlen); 
                    }

                    if (tcdrain(fd) == -1) {
                        spllog(SPL_LOG_ERROR, "Error flushing the serial port buffer");

                    } else {
                        spllog(SPL_LOG_DEBUG, "tcdrain DONE,");
                    }
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
int spsr_send_cmd(int cmd, char *portname, void* data, int datasz) {
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
            /*char *portname = (char *)data;*/
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
        if (cmd == SPSR_CMD_WRITE) {
            /*char *portname = (char *)data;*/
            int lport = 0;
            int len = strlen(portname);

            SP_SERIAL_GENERIC_ST *obj = 0;
            lport = (len + 1) + datasz;            
            nsize = sizeof(SP_SERIAL_GENERIC_ST) + lport;
            spllog(0, "SPSR_CMD_WRITE, nsize: %d, portname: %s", nsize, portname);
            if (t->cmd_buff->range > t->cmd_buff->pl + nsize) {
                obj = (SP_SERIAL_GENERIC_ST *) (t->cmd_buff->data + t->cmd_buff->pl);
                memset(obj, 0, nsize);
                obj->total = nsize;
                obj->type = cmd;
                obj->range = lport;
                memcpy(obj->data, portname, len);
                obj->data[len] = 0;
                obj->pc = len + 1;
                memcpy(obj->data + obj->pc, (char*) data, datasz);
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
#endif

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spserial_verify_info(SP_SERIAL_INPUT_ST* p ) {
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
            GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED , 0);

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
        item->t_delay = p->t_delay;
        node->item = item;
        //if (output) {
        //    *output = item;
        //}
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
        t->count++;
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
int spsr_clear_all() {
    int ret = 0;
    SPSERIAL_ROOT_TYPE* t = &spserial_root_node;
#ifndef UNIX_LINUX
    int count = 0;
    do  {
        char port[64];
        memset(port, 0, 64);
        spserial_mutex_lock(t->mutex);
            count = t->count; 
            if(t->init_node) {
                memcpy(port, t->init_node->item->port_name, strlen(t->init_node->item->port_name));
            } else {
                count = 0;
            }
        spserial_mutex_unlock(t->mutex);
        if(port[0]) {
            ret = spsr_inst_close(port);
            if(ret) {
                spllog(SPL_LOG_ERROR, "spsr_inst_close: ret: %d, port: %s.", ret, port);
            }
        }
    } while(count);
#else

    SPSERIAL_ARR_LIST_LINED* tnode = 0, *temp = 0;
    temp = t->init_node; 
    while (temp) {
        tnode = temp;
        temp = temp->next;
        if(tnode->item) {
            if(tnode->item->handle >= 0) {
                int fd = tnode->item->handle;
                ret = close(fd);
                if(ret) {
                    spllog(SPL_LOG_ERROR, "close: ret: %d, errno: %d, text: %s.", ret, errno, strerror(errno));
                } else {
                    spllog(SPL_LOG_DEBUG, "close fd: %d.", fd);
                }
            }
            spserial_free(tnode->item);
        }
        spserial_free(tnode);
    }
    t->init_node = t->last_node = 0;
    spserial_free(t->cmd_buff);
#endif        
    return ret;
}


#ifndef UNIX_LINUX
#else
//int spsr_hash_port(char *port, int len) {
//    int ret = 0;
//    int *p = 0;
//    if(len >= sizeof(int)) {
//        p = (int*) port + (len - sizeof(int));
//    }
//    else {
//        p = (int*) port;
//    }
//    ret = (*p) % SPSR_MAX_NUMBER_OF_PORT;
//    return ret;
//}
int spsr_clear_hash() {
    int ret = 0;
    int i = 0;
    SPSR_HASH_FD_NAME *tmp = 0, *obj = 0;
    for(i = 0; i < SPSR_MAX_NUMBER_OF_PORT; ++i){
        obj = (SPSR_HASH_FD_NAME *)spsr_hash_fd_arr[i];
        if(!obj) {
            continue;
        }
        while(obj) {
            tmp = obj;
            obj = obj->next;
            spllog(0, "fd: %d, name: %s", tmp->fd, tmp->port_name);
            spserial_free(tmp);
        }
        spsr_hash_fd_arr[i] = 0;
    }
    return ret;
}
int spsr_open_fd(char *port_name, int baudrate, int *outfd) {
    int ret = 0;
    int fd = -1;
    struct termios options = {0};
    int rerr = 0;
    do {
 	    //fd = open(input->port_name, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY | O_SYNC);
        fd = open(port_name, O_RDWR | O_NOCTTY | O_NONBLOCK  | O_SYNC);
        //fd = open(input->port_name, O_RDWR | O_NOCTTY  | O_SYNC);
        if (fd == -1) {
            spllog(SPL_LOG_ERROR, "open port error, fd: %d, errno: %d, text: %s.", fd, errno, strerror(errno));
            ret = PSERIAL_UNIX_OPEN_PORT;
            break;
        }		
        spllog(0, "fd: %d, portname: %s", fd, port_name);
        memset(&options, 0, sizeof(options));
        rerr = tcgetattr(fd, &options);
        if ( rerr < 0) {
            spllog(SPL_LOG_ERROR, "tcgetattr error, fd: %d, errno: %d, text: %s.", fd, errno, strerror(errno));
            ret = PSERIAL_UNIX_GET_ATTR;
            break;
        }

        spllog(0, "fd: %d, portname: %s, rate: %d", fd, port_name, baudrate);
        cfsetispeed(&options, baudrate);
        cfsetospeed(&options, baudrate);			

        options.c_cflag &= ~PARENB;    
        options.c_cflag &= ~CSTOPB;    
        options.c_cflag &= ~CSIZE;
        options.c_cflag |= CS8;        
        //options.c_cflag &= ~CRTSCTS;   
        options.c_cflag |= CRTSCTS; //// Enable RTS/CTS hardware flow control
        options.c_iflag = IGNPAR;
        options.c_cflag |= CREAD | CLOCAL; 	
        options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); 
        //options.c_iflag &= ~(IXON | IXOFF | IXANY);  
        options.c_iflag |= (IXON | IXOFF | IXANY);    //// Enable XON/XOFF software flow control
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
        *outfd = fd;
    } while(0);
    return ret;
}
int spsr_read_fd(int fd, char *buffer, int n, char *chk_delay) {
    int ret = 0;
    int didread = 0;
    int comfd = fd;   
    struct timespec nap_time = {0};
    spllog(0, "------------>>> fd: %d", fd);
    SPSR_HASH_FD_NAME *hashobj = 0, *temp = 0;
    //SP_SERIAL_GENERIC_ST* evt = 0;
    int t_wait = 0;
    do {
        do {
            //int nnnn = 0;
            int hasdid = SPSR_HASH_FD(comfd);
            
            hashobj = (SPSR_HASH_FD_NAME *) spsr_hash_fd_arr[hasdid];
            if(!hashobj) {
                spllog(SPL_LOG_ERROR, "Cannot find obj in reading.");
                ret = PSERIAL_HASH_NOTFOUND;
                break;
            }
            temp = hashobj;
            while(temp)  {
                if(temp->fd == comfd) {
                    t_wait = temp->t_delay;
                    break;
                }
                temp = temp->next;
            }

            /*-+-+ -+-+ -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
            memset(buffer, 0, n);
            if(!chk_delay[0]) {
                nap_time.tv_nsec = t_wait * SPSR_MILLION;
                nanosleep(&nap_time, 0);       
                chk_delay[0] = 1;
            }                     
            //while(1) {
                didread = (int)read(comfd, buffer, n -1);
                if(didread < 1) {
                    spllog(SPL_LOG_ERROR, "read error, fd: %d, errno: %d, text: %s.", fd, errno, strerror(errno));
                    break;
                }
            //}
            buffer[didread] = 0;

            spllog(0, "------------>>> data read didread: %d: %s, fd: %d, temp->t_delay: %d", 
                didread, buffer, fd, t_wait);        
  
            if(!temp) {
                ret = PSERIAL_HASH_NOTFOUND;
                spllog(SPL_LOG_ERROR, "Didsee Cannot find obj in reading.");                
                break;
            }  
            if(!temp->cb_evt_fn) {
                spllog(SPL_LOG_DEBUG, "cb_evt_fn, SPSERIAL_MEM_NULL.");
                break;
            }

            ret = spsr_invoke_cb(temp->cb_evt_fn, temp->cb_obj, buffer, didread);

        } while(0);

    } while(0); 
    return ret;
}
#endif
int spsr_invoke_cb(SPSERIAL_module_cb fn_cb, void *obj, char *data, int len) {
	int ret = 0;
    SP_SERIAL_GENERIC_ST* evt = 0;
    int n = 0;
	do {
		n = 1 + sizeof(SP_SERIAL_GENERIC_ST) + len + sizeof(void*);
        spserial_malloc(n, evt, SP_SERIAL_GENERIC_ST);
        if (!evt) {
            spllog(SPL_LOG_ERROR, "spserial_malloc, SPSERIAL_MEM_NULL.");
            ret = SPSERIAL_MEM_NULL;
            break;
        }
        evt->total = n;
        evt->type = SPSERIAL_EVENT_READ_BUF;

        evt->pc = sizeof(void*);
        if (sizeof(void*) == sizeof(unsigned int)) {
            unsigned int* pt = (unsigned int*) evt->data;
            *pt = (unsigned int)obj;
            spllog(SPL_LOG_DEBUG, "Try this case-------------------.");
        }
        else  if (sizeof(void*) == sizeof(unsigned long long int)) {
            unsigned long long int* pt = (unsigned long long int*) evt->data;
            spllog(SPL_LOG_DEBUG, "Try this case-------------------.");
            *pt = (unsigned long long int)obj;
        }
        memcpy(evt->data + evt->pc, data, len);
        evt->pl = evt->pc + len;
        fn_cb(evt);
        spserial_free(evt);
	} while(0);
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#ifndef UNIX_LINUX
#else
#endif

#ifndef __SPSR_EPOLL__
#else
#endif
//TODO: clean hash array