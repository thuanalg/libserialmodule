/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
/* Email:
*		<nguyenthaithuanalg@gmail.com> - Nguyễn Thái Thuận
* Mobile:
*		<+084.799.324.179>
* Skype:
*		<nguyenthaithuanalg>
* Date:
*		<2025-Mar-01>
* The lasted modified date:
		<2025-Mar-27>
		<2025-Apr-29>
		<2025-Apr-30>
		<2025-May-01>
		<2025-May-02>
		<2025-May-03>
		<2025-May-06>
* Decription:
*		The (only) main header file to export 
		5 APIs: [spsr_module_init, spsr_module_finish, spsr_inst_open,
spsr_inst_close, spsr_inst_write].
*/
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

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

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#ifndef UNIX_LINUX
#define SPSR_CloseHandle(__o0bj__)                                                                                          \
	{                                                                                                                   \
		void *__serialpp__ = (__o0bj__);                                                                            \
		if (__serialpp__) {                                                                                         \
			;                                                                                                   \
			int bl = CloseHandle((__serialpp__));                                                               \
			;                                                                                                   \
			if (!bl) {                                                                                          \
				;                                                                                           \
				spllog(SPL_LOG_ERROR, "CloseHandle error: %lu", GetLastError());                            \
				;                                                                                           \
			};                                                                                                  \
			spllog(0, "SPSR_CloseHandle 0x%p -->> %s", __serialpp__, (bl ? "DONE" : "ERROR"));                  \
			;                                                                                                   \
			(__o0bj__) = 0;                                                                                     \
			;                                                                                                   \
		}                                                                                                           \
	}
#else

#define SPSR_PORT_TRIGGER          (10024 + 1)
#define SPSR_PORT_CARTRIDGE        (SPSR_PORT_TRIGGER + 10)
#endif

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

#define SPSR_MAX_AB(__a__, __b__) ((__a__) > (__b__)) ? (__a__) : (__b__)
#define SPSR_STEP_MEM              2048
#define SPSR_BUFFER_SIZE           2048
#define SPSR_DATA_RANGE            2048
#define SPSR_CMD_BUFF              2048

#define SPSR_EVT_CB_CART_LEN       (SPSR_BUFFER_SIZE + sizeof(void*) + sizeof(SPSR_GENERIC_ST) + 1)
#define SPSR_CMD_BUFF_LEN          (SPSR_CMD_BUFF + sizeof(SPSR_GENERIC_ST) + 1)

#define SPSR_EVT_CB_PORT_LEN       (SPSR_PORT_LEN + sizeof(void*) + sizeof(SPSR_GENERIC_ST) + 1)

#ifndef UNIX_LINUX

#define SPSR_THREAD_ROUTINE   LPTHREAD_START_ROUTINE

static int
spsr_clear_node(SPSR_ARR_LIST_LINED *);
static int
spsr_module_isoff(SPSR_INFO_ST *obj);
static int
spsr_module_openport(void *);
static DWORD WINAPI
spsr_thread_operating_routine(LPVOID lpParam);
static int
spsr_create_thread(SPSR_THREAD_ROUTINE f, void *arg);
static int
spsr_get_obj(char *portname, void **obj, int takeoff);
static int
spsr_win32_write(
	SPSR_INFO_ST *p, 
	SPSR_GENERIC_ST *buf, 
	DWORD *bytesWrite, 
	OVERLAPPED *olReadWrite,
    SPSR_GENERIC_ST *evt_cb_buff);
static int
spsr_win32_read(SPSR_INFO_ST *p, 
	DWORD *bytesWrite, OVERLAPPED *olReadWrite,
    SPSR_GENERIC_ST *evt_cb_buf);
#else

typedef void *(*SPSR_THREAD_ROUTINE)(void *);
static int spsr_init_trigger(void *);

static int spsr_px_write(
	SPSR_GENERIC_ST *item, 
	SPSR_GENERIC_ST *evt);

#ifndef __SPSR_EPOLL__
int spsr_px_rem(
	SPSR_GENERIC_ST *item, 
	SPSR_GENERIC_ST *evt, int *prange, struct pollfd *fds);
#else

int spsr_px_rem(
	SPSR_GENERIC_ST *item, 
	SPSR_GENERIC_ST *evt, int epollfd);
#endif

#ifndef __SPSR_EPOLL__
int spsr_px_add(
	SPSR_GENERIC_ST *item, 
	SPSR_GENERIC_ST *evt, int *prange, struct pollfd *fds);
#else

int spsr_px_add(
	SPSR_GENERIC_ST *item, 
	SPSR_GENERIC_ST *evt, int epollfd);
#endif	

int spsr_px_hash_add(
	SPSR_ARR_LIST_LINED *temp,
	SPSR_GENERIC_ST *evt) ;

#ifndef __SPSR_EPOLL__
#define SPSR_LOG_UNIX__SHARED_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define SPSR_LOG_UNIX_CREATE_MODE  (O_CREAT | O_RDWR | O_EXCL)
#define SPSR_LOG_UNIX_OPEN_MODE    (O_RDWR | O_EXCL)
#define SPSR_LOG_UNIX_PROT_FLAGS   (PROT_READ | PROT_WRITE | PROT_EXEC)

#define SPSR_SENDSK_FLAG           0

static int spsr_fetch_commands(void *, 
	int *, char *, int n, 
	SPSR_GENERIC_ST *evt);

static int spsr_ctrl_sock(void *fds, 
	int *mx_number, int sockfd, 
	SPSR_GENERIC_ST *evt, int *chk_off, 
	SPSR_GENERIC_ST **pcart_buff);

static int spsr_fmt_name(
	char *input, char *output, int);
#else
#define SPSR_SENDSK_FLAG           MSG_CONFIRM
static int spsr_fetch_commands(
	int, char *, 
	int n, 
	SPSR_GENERIC_ST *evt);
static int spsr_ctrl_sock(
	int epollfd, 
	int sockfd, 
	SPSR_GENERIC_ST *evt, int *chk_off, 
	SPSR_GENERIC_ST **pcart_buff);

#endif

static void *
spsr_init_trigger_routine(void *);
static void *
spsr_init_cartridge_routine(void *);
static int
spsr_send_cmd(int cmd, 
	char *portname, 
	void *data, int lendata);

#define SPSR_MAX_NUMBER_OF_PORT    10

static void *spsr_hash_fd_arr[SPSR_MAX_NUMBER_OF_PORT];

/* static void *spsr_hash_name_arr[SPSR_MAX_NUMBER_OF_PORT]; */

typedef struct __SPSR_HASH_FD_NAME__ {
	int fd;
	char port_name[SPSR_PORT_LEN];
	SPSR_module_cb cb_evt_fn;
	void *cb_obj;
	struct __SPSR_HASH_FD_NAME__ *next;
	int t_delay;
} SPSR_HASH_FD_NAME;

#define SPSR_HASH_FD(__fd__) (__fd__ % SPSR_MAX_NUMBER_OF_PORT)

/* static int spsr_hash_port(char* port, int len); */

static int spsr_clear_hash();
static int spsr_open_fd(
	char *port_name, int brate, int *);

static int spsr_read_fd(int fd, 
	SPSR_GENERIC_ST *pevt, 
	char *chk_delay);

#endif

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

static SPSR_ROOT_TYPE spsr_root_node;

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
static int spsr_resize_obj(
	int sz, SPSR_GENERIC_ST **obj);
static int spsr_clear_all();
static int spsr_verify_info(
	SPSR_INPUT_ST *obj);
static int spsr_is_existed(char *port, int *);

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
/* Group of sync tool. */
static void *spsr_mutex_create();
static int spsr_mutex_delete(void *);
static void *spsr_sem_create(char *);
static int spsr_sem_delete(void *, char *);
static int spsr_mutex_lock(void *obj);
static int spsr_mutex_unlock(void *obj);
static int spsr_rel_sem(void *sem);
static int spsr_wait_sem(void *sem);

static int spsr_invoke_cb(int evttype, 
	SPSR_module_cb fn_cb, 
	void *obj_cb, 
	SPSR_GENERIC_ST *evt, int lendata);
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

int spsr_inst_open(
	SPSR_INPUT_ST *p)
{
	int ret = 0;
	SPSR_ROOT_TYPE *t = &spsr_root_node;

	do {

		spsr_mutex_lock(t->mutex);
		/*do {*/
			ret = spsr_verify_info(p);
		/*} while (0); */
		spsr_mutex_unlock(t->mutex);

	} while (0);

	if (ret) {
		spllog(SPL_LOG_ERROR, 
			"spsr_verify_info: %d.", ret);
	}
	return ret;
}

int spsr_inst_close(char *portname)
{
	int ret = 0;
	spllog(0, "Delete port: %s.", portname);
#ifndef UNIX_LINUX
	void *p = 0;
	SPSR_ARR_LIST_LINED *node = 0;
	do {
		ret = spsr_get_obj(portname, &p, 1);
		if (p) {
			spllog(0, "Delete port: %s.", portname);
			node = (SPSR_ARR_LIST_LINED *)p;
			spsr_clear_node(node);

			spsr_free(node->item);
			spsr_free(node);
		}
	} while (0);
#else
	SPSR_ROOT_TYPE *t = &spsr_root_node;
	int isExisted = 0;
	spsr_mutex_lock(t->mutex);
	do {
		ret = spsr_is_existed(portname, &isExisted);
		if(ret) {
			break;
		}
		if(!isExisted) {
			spllog(SPL_LOG_ERROR, 
				"port %s not exsied.", 
				portname);
			ret = SPSR_PORTNAME_NONEXISTED;
			break;
		}
		ret = spsr_send_cmd(SPSR_CMD_REM, portname, 0, 0);
	} while (0);
	spsr_mutex_unlock(t->mutex);
#endif
	return ret;
}

int spsr_module_openport(void *obj)
{
	int ret = 0;
	SPSR_INFO_ST *p = (SPSR_INFO_ST *)obj;

	/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
	
	do {
#ifndef UNIX_LINUX
		DCB dcb = {0};
		HANDLE hSerial = 0;
		COMSTAT comStat = {0};
		DWORD dwError = 0;
		BOOL fSuccess = FALSE;
		DCB dcbSerialParams = {0};
		COMMTIMEOUTS timeouts = {0};
		if (!p) {
			ret = SPSR_PORT_INFO_NULL;
			break;
		}

		/*Open the serial port with FILE_FLAG_OVERLAPPED for asynchronous operation*/
		hSerial =
		    CreateFile(p->port_name, 
				GENERIC_READ | GENERIC_WRITE, 
				0, 0, OPEN_EXISTING, 
				FILE_FLAG_OVERLAPPED, 0);

		if (hSerial == INVALID_HANDLE_VALUE) {
			DWORD dwError = GetLastError();
			spllog(SPL_LOG_ERROR, 
				"Open port errcode: %lu", dwError);
			ret = SPSR_PORT_OPEN;
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
			spllog(SPL_LOG_ERROR, 
				"GetCommState: %lu", dwError);
			ret = SPSR_PORT_GETCOMMSTATE;
			break;
		}
		dcbSerialParams.BaudRate = p->baudrate;
		dcbSerialParams.ByteSize = 8;
		dcbSerialParams.StopBits = ONESTOPBIT;
		dcbSerialParams.Parity = NOPARITY;
		/* dcbSerialParams.StopBits */

		/* Enable hardware flow control (RTS/CTS) */
		dcbSerialParams.fOutxCtsFlow = TRUE; 
		/* Enable CTS output flow control */
		/* dcbSerialParams.fCtsHandshake = TRUE; */
		dcbSerialParams.fOutxDsrFlow = FALSE; 
		/* Disable DSR output flow control */
		dcbSerialParams.fDsrSensitivity = FALSE; 
		/* DSR sensitivity disabled */
		dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE; 
		/* Enable DTR */
		dcbSerialParams.fRtsControl = RTS_CONTROL_ENABLE; 
		/* Enable RTS */

		/* Enable software flow control (XON/XOFF) */
		dcbSerialParams.fInX = TRUE; 
		/* Enable XON/XOFF input flow control */
		dcbSerialParams.fOutX = TRUE; 
		/* Enable XON/XOFF output flow control */

		if (!SetCommState(hSerial, &dcbSerialParams)) {
			DWORD dwError = GetLastError();
			spllog(SPL_LOG_ERROR, 
				"SetCommState: %lu", dwError);
			ret = SPSR_PORT_SETCOMMSTATE;
			break;
		}
		if(!p->hEvent) {
			p->hEvent = CreateEvent(0, TRUE, FALSE, 0);
		}
		if (!p->hEvent) {
			DWORD dwError = GetLastError();
			spllog(SPL_LOG_ERROR, 
				"CreateEvent: %lu", dwError);
			ret = SPSR_PORT_CREATEEVENT;
			break;
		}

		spllog(SPL_LOG_DEBUG, "Create hEvent: 0x%p.", p->hEvent);

		/* Set timeouts(e.g., read timeout of 500ms, write timeout of 500ms)
			//timeouts.ReadIntervalTimeout = 500;
			//timeouts.ReadTotalTimeoutConstant = 500;
			//timeouts.ReadTotalTimeoutMultiplier = 500;
			//timeouts.WriteTotalTimeoutConstant = 500;
			//timeouts.WriteTotalTimeoutMultiplier = 500;
		*/
		timeouts.ReadIntervalTimeout = 50;
		timeouts.ReadTotalTimeoutConstant = 50;
		timeouts.ReadTotalTimeoutMultiplier = 10;
		timeouts.WriteTotalTimeoutConstant = 50;
		timeouts.WriteTotalTimeoutMultiplier = 10;

		if (!SetCommTimeouts(hSerial, &timeouts)) {
			DWORD dwError = GetLastError();
			spllog(SPL_LOG_ERROR, 
				"SetCommTimeouts: %lu", dwError);
			ret = SPSR_PORT_SETCOMMTIMEOUTS;
			break;
		}
		if(!p->mtx_off) {
			p->mtx_off = spsr_mutex_create();
		}
		if (!p->mtx_off) {
			DWORD dwError = GetLastError();
			spllog(SPL_LOG_ERROR, 
				"spsr_mutex_create: %lu", dwError);
			ret = SPSR_PORT_SPSR_MUTEX_CREATE;
			break;
		}
		if(!p->sem_off) {
			p->sem_off = spsr_sem_create(0);
		}
		if (!p->sem_off) {
			DWORD dwError = GetLastError();
			spllog(SPL_LOG_ERROR, 
				"spsr_sem_create: %lu", dwError);
			ret = SPSR_PORT_SPSR_SEM_CREATE;
			break;
		}
#else
#endif
	} while (0);
	/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
	if (ret && p) {
#ifndef UNIX_LINUX
		if (p->handle) {
			SPSR_CloseHandle(p->handle);
		}
#else
#endif
	}
	/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
void *
spsr_sem_create(char *name_key)
{
	void *obj = 0;
	do {
#ifndef UNIX_LINUX
		obj = CreateSemaphoreA(0, 0, 1, 0);
		if(!obj) {
			DWORD dwError = GetLastError();
			spllog(SPL_LOG_ERROR, 
				"CreateSemaphoreA, %d",
				(int)dwError);
			break;
		}
#else
#ifndef __SPSR_EPOLL__
		int retry = 0;
		char name[SPSR_KEY_LEN * 2];

		spsr_fmt_name(name_key, name, SPSR_KEY_LEN * 2);
		do {
			obj = sem_open(
				name, 
				SPSR_LOG_UNIX_CREATE_MODE, 
				SPSR_LOG_UNIX__SHARED_MODE, 
				1);
			spllog(0, "sem_open ret: 0x%p", obj);
			if (obj == SEM_FAILED) {
				int err = 0;
				obj = 0;
				if (retry) {
					spllog(SPL_LOG_ERROR, 
						"mach sem_open, errno: "
						"%d, text: %s, name: %s.", 
						errno,
					    strerror(errno), 
						name);
					break;
				} else {
					spllog(SPL_LOG_ERROR, 
						"mach sem_open, errno: "
						"%d, text: %s, name: %s.", 
						errno,
					    strerror(errno), name);
				}
				err = sem_unlink(name);
				if (err) {
					spllog(SPL_LOG_ERROR, 
						"mach sem_unlink, errno: "
						"%d, text: %s, name: %s.", 
						errno,
					    strerror(errno), name);
					break;
				}
				retry++;
				continue;
			}
			break;
		} while (1);
#else

		/*https://linux.die.net/man/3/sem_init*/
		spsr_malloc(sizeof(sem_t), obj, void);
		if (!obj) {
			break;
		}
		memset(obj, 0, sizeof(sem_t));
		sem_init((sem_t *)obj, 0, 1);
#endif
#endif
	} while (0);
	spllog(0, "Create: 0x%p.", obj);
	return obj;
}

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

void *
spsr_mutex_create()
{
	void *obj = 0;
	do {
#ifndef UNIX_LINUX
		obj = CreateMutexA(0, 0, 0);
#else
		/*https://linux.die.net/man/3/pthread_mutex_init*/
		spsr_malloc(sizeof(pthread_mutex_t), obj, void);
		if (!obj) {
			break;
		}
		memset(obj, 0, sizeof(pthread_mutex_t));
		pthread_mutex_init((pthread_mutex_t *)obj, 0);
#endif
	} while (0);
	spllog(0, "Create: 0x%p.", obj);
	return obj;
}

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

int spsr_module_isoff(SPSR_INFO_ST *obj)
{
	int rs = 0;
	spsr_mutex_lock(obj->mtx_off);
		rs = obj->isoff;
	spsr_mutex_unlock(obj->mtx_off);
	return rs;
}



#ifndef UNIX_LINUX
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int
spsr_win32_read(
	SPSR_INFO_ST *p, 
	DWORD *pbytesRead, 
	OVERLAPPED *polReadWrite,
    SPSR_GENERIC_ST *ecb_buf)
{
	char *tbuffer = 0;
	int ret = 0;
	BOOL rs = FALSE;
	COMSTAT csta = {0};
	DWORD dwError = 0;
	DWORD readErr = 0;
	do {
		tbuffer = ecb_buf->data + sizeof(void *);
		rs = ReadFile(
			p->handle, tbuffer, 
			SPSR_BUFFER_SIZE, 
			pbytesRead, polReadWrite);

		spllog(SPL_LOG_DEBUG,
		    "olRead.InternalHigh: %d, "
		    "olRead.Internal: %d, rs : %s!!!",
		    (int)polReadWrite->InternalHigh, 
			(int)polReadWrite->Internal, 
			rs ? "true" : "false");

		if (rs) {
			spllog(SPL_LOG_INFO, "Read OK");
			break;
		}
		readErr = GetLastError();
		if (readErr != ERROR_IO_PENDING) {
			ret = SPSR_WIN32_NOT_PENDING;
			spllog(SPL_LOG_ERROR, "Read error readErr: %d", (int)readErr);
			break;
		}
		*pbytesRead = 0;
		if (p->t_delay > 0) {
			Sleep(p->t_delay);
		}
		WaitForSingleObject(p->hEvent, INFINITE);

		rs = GetOverlappedResult(p->handle, 
			polReadWrite, pbytesRead, 1);

		if (!rs) {
			spllog(SPL_LOG_ERROR, 
				"PurgeComm: %d", 
				(int)GetLastError());
			PurgeComm(p->handle, 
				PURGE_RXCLEAR | 
				PURGE_TXCLEAR);
			ret = SPSR_WIN32_OVERLAP_ERR;
			break;
		}
		spllog(SPL_LOG_DEBUG, 
			"bRead: %d", (int)*pbytesRead);
	} while (0);

	if (ret) {
		return ret;
	}

	memset(&csta, 0, sizeof(csta));
	ClearCommError(p->handle, &dwError, &csta);

	if (csta.cbInQue > 0) {
		spllog(SPL_LOG_ERROR, 
			"Read Com not finished!!!");
	} 
	else if (*pbytesRead > 0) 
	{
		tbuffer[*pbytesRead] = 0;
		spllog(0, "[tbuffer: %s]!", tbuffer);
		spsr_invoke_cb(
			SPSR_EVENT_READ_BUF, 
			p->cb_evt_fn,
			p->cb_obj, 
			ecb_buf, 
			*pbytesRead);
	}

	return ret;
}
    /*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int
spsr_win32_write(SPSR_INFO_ST *p, 
	SPSR_GENERIC_ST *buf, 
	DWORD *pbytesWrite, OVERLAPPED *polReadWrite,
    SPSR_GENERIC_ST *ecb_buf)
{
	int ret = 0;
	BOOL wrs = FALSE;
	char *tbuffer = 0;
	int evtcode = 0;
	int portlen = 0;
	DWORD wErr = 0;
	DWORD dwWaitResult = 0;
	int wroteRes = 0;
	BOOL rsOverlap = TRUE;

	do {
		if (!p) {
			ret = SPSR_WIN32_OBJ_NULL;
			spllog(SPL_LOG_ERROR, 
				"SPSR_WIN32_OBJ_NULL"); 			
			break;
		}
		if (!buf) {
			ret = SPSR_WIN32_BUF_NULL;
			spllog(SPL_LOG_ERROR, 
				"SPSR_WIN32_BUF_NULL"); 
			break;
		}
		if (buf->pl < 1) {
			ret = SPSR_WIN32_BUF_NULL;
			spllog(SPL_LOG_ERROR, 
				"SPSR_WIN32_BUF_NULL"); 			
			break;
		}
		if (!pbytesWrite) {
			ret = SPSR_WIN32_BWRITE_NULL;
			spllog(SPL_LOG_ERROR, 
				"SPSR_WIN32_BWRITE_NULL"); 			
			break;
		}
		if (!polReadWrite) {
			ret = SPSR_WIN32_OVERLAP_NULL;
			spllog(SPL_LOG_ERROR, 
				"SPSR_WIN32_OVERLAP_NULL");			
			break;
		}
		if (!ecb_buf) {
			ret = SPSR_WIN32_EVTCB_NULL;
			break;
		}
		tbuffer = ecb_buf->data + sizeof(void *);
		while (buf->pl > 0) {	
			*pbytesWrite = 0;
			memset(polReadWrite, 0, sizeof(OVERLAPPED));
			polReadWrite->hEvent = p->hEvent;

			wrs = WriteFile(
				p->handle, 
				buf->data, buf->pl, 
				pbytesWrite, polReadWrite);
			if (wrs) {
				if (buf->pl == (int)(*pbytesWrite)) {
					spllog(SPL_LOG_DEBUG, 
						"Write DONE, %d.", buf->pl);
					wroteRes = 1;
					buf->pl = 0;
				} else {
					spllog(SPL_LOG_ERROR, 
						"Write Error, %d.", buf->pl);
				}
				break;
			}

			wErr = GetLastError();
			spllog(SPL_LOG_DEBUG, 
				"WriteFile: %d", (int)wErr);
			if (wErr != ERROR_IO_PENDING) {
				spllog(SPL_LOG_ERROR, 
					"Write Error, %d.", buf->pl);
				break;
			}

			dwWaitResult = WaitForSingleObject(
				p->hEvent, INFINITE);
			if (dwWaitResult != WAIT_OBJECT_0) {
				spllog(SPL_LOG_ERROR, 
					"Write Error, WaitForSingleObject, %d.",
					buf->pl);
				break;
			}
			*pbytesWrite = 0;

			rsOverlap = GetOverlappedResult(p->handle, 
				polReadWrite, pbytesWrite, TRUE);

			if (!rsOverlap) {
				spllog(SPL_LOG_ERROR, 
					"Write Error code, %d.", buf->pl);
				break;
			}
			if (buf->pl != (int)(*pbytesWrite)) {
				spllog(SPL_LOG_ERROR, 
					"Write Error, %d.", buf->pl);
				break;
			}
			spllog(SPL_LOG_DEBUG, "Write DONE, %d.", buf->pl);
			wroteRes = 1;
			buf->pl = 0;
			break;
		}

		buf->pl = 0;

		evtcode = wroteRes ? 
			SPSR_EVENT_WRITE_OK : 
			SPSR_EVENT_WRITE_ERROR;

		portlen = (int)strlen(p->port_name);
		tbuffer[portlen] = 0;

		memcpy(tbuffer, 
			p->port_name, portlen);

		spsr_invoke_cb(evtcode, 
			p->cb_evt_fn, 
			p->cb_obj, 
			ecb_buf, portlen);

	} while (0);
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
DWORD WINAPI spsr_thread_operating_routine(
	LPVOID arg)
{
	SPSR_ARR_LIST_LINED *pp = (SPSR_ARR_LIST_LINED *)arg;
	SPSR_INFO_ST *p = pp->item;
	int isoff = 0;
	int ret = 0;
	SPSR_GENERIC_ST *buf = 0;
	DWORD bytesRead = 0;
	DWORD bytesWrite = 0;
	SPSR_GENERIC_ST *ecb_buf = 0;
	int portlen = 0;
	int evtcode = 0;
	char *tbuffer = 0;
	char evtbytes[SPSR_EVT_CB_CART_LEN] = {0};	
	int step = sizeof(SPSR_GENERIC_ST) + SPSR_STEP_MEM + 1;


	ecb_buf = (SPSR_GENERIC_ST *)evtbytes;
	ecb_buf->total = SPSR_EVT_CB_CART_LEN;
	ecb_buf->range = SPSR_BUFFER_SIZE;
	ecb_buf->pc = ecb_buf->pl = sizeof(void *);
	tbuffer = ecb_buf->data + sizeof(void *);

	spsr_resize_obj(step, &buf);

	while (1) {
		DWORD dwError = 0;
		int wrote = 0;
		DWORD dwEvtMask = 0, flags = 0;
		OVERLAPPED olReadWrite = {0};
		BOOL rs = FALSE;
		BOOL wrs = FALSE;
		int count = 0, cbInQue = 0;
		COMSTAT csta = {0};
		
		flags = EV_RXCHAR | EV_BREAK | EV_RXFLAG | EV_DSR;



		if (!buf) {
			spllog(SPL_LOG_ERROR, "buf NULL");
			break;
		}

		if (isoff) {
			spllog(0, "is OFF");
			break;
		}

		SPSR_CloseHandle(p->handle);

		isoff = spsr_module_isoff(p);		
		if (p->is_retry) {
			spllog(0, "retry");
		}			

		ret = spsr_module_openport(p);

		portlen = (int)strlen(p->port_name);
		do {
			int eventcb = 0;
			memcpy(tbuffer, 
				p->port_name, portlen);

			tbuffer[portlen] = 0;

			eventcb = ret ? 
				SPSR_EVENT_OPEN_DEVICE_ERROR : 
				SPSR_EVENT_OPEN_DEVICE_OK;

			spsr_invoke_cb(eventcb, 
				p->cb_evt_fn, 
				p->cb_obj, 
				ecb_buf, portlen);

		} while (0);

		if (ret) {
			Sleep(1000 * 2);
			spllog(SPL_LOG_ERROR, 
				"In r/w loop.");
			continue;;
		}

		while (1) {
			isoff = spsr_module_isoff(p);
			if (isoff) {
				spllog(0, "is OFF");
				break;
			}
			memset(&olReadWrite, 
				0, sizeof(olReadWrite));
			olReadWrite.hEvent = p->hEvent;
			wrs = TRUE;
			spsr_mutex_lock(p->mtx_off);
			do {

				if (!p->buff) {
					spllog(0, "No data.");
					break;
				}	

				if (p->buff->pl < 1) {
					spllog(0, "No data.");
					break;
				}

				spllog(0, 
					"(pl, range): (%d, %d)",
					p->buff->pl, buf->range);

				if(p->buff->pl > buf->range)
				{
					int rz = 0;
					rz = SPL_MAX_AB(
						p->buff->pl, 
						SPSR_STEP_MEM);
					ret = spsr_resize_obj(
						rz, &buf);
				}
				buf->pl = p->buff->pl;
				memcpy(buf->data, 
					p->buff->data, buf->pl);
				p->buff->pl = 0;

			} while (0);
			spsr_mutex_unlock(p->mtx_off);

			if (buf->pl > 0) {
				ret = spsr_win32_write(p, 
					buf, &bytesWrite, 
					&olReadWrite, ecb_buf);
				if(ret) {
					spllog(SPL_LOG_ERROR, 
						"spsr_win32_write");
					SPSR_CloseHandle(p->handle);
					break;
				}
			}

			rs = SetCommMask(p->handle, flags);
			dwEvtMask = flags;
			memset(&olReadWrite, 
				0, sizeof(olReadWrite));
			olReadWrite.hEvent = p->hEvent;
			rs = WaitCommEvent(
				p->handle, 
				&dwEvtMask, &olReadWrite);
			if (!rs) {
				DWORD dwRet = GetLastError();
				if (ERROR_IO_PENDING != dwRet) 
				{
					++count;
				}
				if (count > 3) {
					break;
				}
			} else {
				spllog(SPL_LOG_DEBUG, 
					"WaitCommEvent OK");
			}
			memset(&csta, 0, sizeof(csta));
			ClearCommError(p->handle, 
				&dwError, &csta);
			cbInQue = csta.cbInQue;
			if (!cbInQue) {
				BOOL rsOverlap = TRUE;
				WaitForSingleObject(
					p->hEvent, INFINITE);
				rsOverlap = GetOverlappedResult(
					p->handle, 
					&olReadWrite, 
					&bytesRead, TRUE);
				if (rsOverlap) {
					continue;
				} 
				PurgeComm(
					p->handle, 
					PURGE_RXCLEAR | 
					PURGE_TXCLEAR);
				
				continue;
			}

			bytesRead = 0;
			ret = spsr_win32_read(p, 
				&bytesRead, 
				&olReadWrite, ecb_buf);
			spllog(SPL_LOG_DEBUG, 
				" [[[ cbInQue: %d, bRead: %d ]]]", 
				cbInQue, bytesRead);
			bytesRead = 0;
		}

		p->is_retry = 1;
		if (isoff) {
			break;
		}
	}
	spsr_free(buf);
	SPSR_CloseHandle(p->hEvent);
	SPSR_CloseHandle(p->handle);
	spsr_rel_sem(p->sem_off);
	return 0;
}
#else

#endif
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int
spsr_module_init()
{
	int ret = 0;
	SPSR_ROOT_TYPE *t = &spsr_root_node;
#ifndef UNIX_LINUX
#else
	pthread_t idd = 0;
	int err = 0;
	int nsize = 0;
#ifndef __SPSR_EPOLL__
	snprintf(t->sem_key, 
		SPSR_KEY_LEN, 
		"/spsr_%llu", (LLU)getpid());
#endif
#endif
	do {
		t->mutex = spsr_mutex_create();
		if (!t->mutex) {
			ret = SPSR_MTX_CREATE;
			break;
		}
#ifndef UNIX_LINUX
		t->sem = spsr_sem_create(0);
#else
#ifndef __SPSR_EPOLL__
		t->sem = spsr_sem_create(SPSR_MAINKEY);
#else
		t->sem = spsr_sem_create(0);
#endif
#endif
		if (!t->sem) {
			ret = SPSR_SEM_CREATE;
			break;
		}
#ifndef UNIX_LINUX
#else
#ifndef __SPSR_EPOLL__

		t->sem_spsr = spsr_sem_create(SPSR_MAINKEY_MACH); /*Mach*/
#else
		t->sem_spsr = spsr_sem_create(0); /*Linux*/
#endif
		if (!t->sem_spsr) {
			ret = SPSR_SEM_CREATE;
			break;
		}

		err = pthread_create(&idd, 0, 
			spsr_init_trigger_routine, t);
		if (err) {
			ret = SPSR_CREATE_THREAD_ERROR;
			break;
		}

		idd = 0;
		err = pthread_create(&idd, 0, 
			spsr_init_cartridge_routine, t);
		if (err) {
			ret = SPSR_CREATE_THREAD_ERROR;
			break;
		}

		nsize = SPSR_BUFFER_SIZE + sizeof(int);
		spsr_malloc(nsize, 
			t->cmd_buff, SPSR_GENERIC_ST);
		if (!t->cmd_buff) {
			spllog(SPL_LOG_ERROR, 
				"spsr_malloc error");
			exit(1);
		}
		t->cmd_buff->total = nsize;
		t->cmd_buff->range = SPSR_BUFFER_SIZE;
#endif
		spllog(SPL_LOG_DEBUG, 
			"spsr_module_init: DONE");

	} while (0);
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int
spsr_module_finish()
{
	SPSR_ROOT_TYPE *t = &spsr_root_node;

	spsr_mutex_lock(t->mutex);
	/*do {*/
	t->spsr_off = 1;
	/*} while (0);*/
	spsr_mutex_unlock(t->mutex);

#ifndef UNIX_LINUX
	spsr_clear_all();
	spsr_sem_delete(t->sem, 0);
#else

	/*----------------------------------------*/
	while (1) {
		int is_off = 0;
		spsr_rel_sem(t->sem);
		/*t->sem_spsr*/
		spsr_wait_sem(t->sem_spsr);

		spsr_mutex_lock(t->mutex);
		/*do {*/
		is_off = t->spsr_off;
		/*} while (0);*/
		spsr_mutex_unlock(t->mutex);
		if (is_off > 2) {
#ifndef __SPSR_EPOLL__
			spsr_sem_delete(
				t->sem_spsr, SPSR_MAINKEY_MACH);
#else
			spsr_sem_delete(t->sem_spsr, 0);
#endif
			break;
		}
	}

#ifndef __SPSR_EPOLL__
	spsr_sem_delete(
		t->sem, SPSR_MAINKEY);
#else
	spsr_sem_delete(t->sem, 0);
#endif
#endif
	spsr_mutex_delete(t->mutex);

	return 0;
}

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spsr_mutex_lock(void *obj)
{
	int ret = 0;
#ifndef UNIX_LINUX
	DWORD err = 0;
#else
#endif
	do {
		if (!obj) {
			ret = SPSR_MUTEX_NULL_ERROR;
			break;
		}
#ifndef UNIX_LINUX
		err = WaitForSingleObject(obj, INFINITE);
		if (err != WAIT_OBJECT_0) {
			DWORD dwError = GetLastError();
			spllog(SPL_LOG_ERROR, 
				"WaitForSingleObject errcode: %lu", 
				dwError);			
			ret = SPSR_WIN32_LK_MTX;;
			break;
		}
#else
		ret = pthread_mutex_lock(
			(pthread_mutex_t *)obj);
		if (ret) {
			spllog(SPL_LOG_ERROR, 
				"pthread_mutex_lock: ret: %d, "
				"errno: %d, text: %s, obj: 0x%p.", 
				ret, errno,
			    strerror(errno), obj);
			ret = SPSR_PX_LK_MTX;
		}
#endif
	} while (0);
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spsr_mutex_unlock(void *obj)
{
	int ret = 0;
#ifndef UNIX_LINUX
	DWORD done = 0;
#else
#endif
	do {
		if (!obj) {
			ret = SPSR_MUTEX_NULL_ERROR;
			break;
		}
#ifndef UNIX_LINUX
		done = ReleaseMutex(obj);
		if (!done) {
			DWORD dwError = GetLastError();
			spllog(SPL_LOG_ERROR, 
				"ReleaseMutex errcode: %lu", 
				dwError);
			ret = SPSR_WIN32_RL_MTX;
			break;
		}
#else
		ret = pthread_mutex_unlock(
			(pthread_mutex_t *)obj);
		if (ret) {
			spllog(SPL_LOG_ERROR, 
				"pthread_mutex_unlock: ret: %d, "
				"errno: %d, text: %s.", 
				ret, errno,
			    strerror(errno));
			ret = SPSR_PX_RL_MTX;
		}
#endif
	} while (0);
	return ret;
}

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

int spsr_rel_sem(void *sem)
{
	int ret = 0;
#ifndef UNIX_LINUX
#else
	int err = 0;
#endif
	do {
		if (!sem) {
			ret = SPSR_SEM_NULL_ERROR;
			break;
		}
#ifndef UNIX_LINUX
		int iswell = 0;
		iswell = ReleaseSemaphore(sem, 1, 0);
		if (!iswell) {
			DWORD dwError = GetLastError();
			spllog(SPL_LOG_ERROR, 
				"ReleaseSemaphore errcode: %lu", 
				dwError);
			ret = SPSR_WIN32_RL_SEM;
		}		
#else

		err = sem_post((sem_t *)sem);
		if (err) {
			spllog(SPL_LOG_ERROR, "sem_post: err: %d, "
				"errno: %d, text: %s, sem: 0x%p.", 
				err, errno,
			    strerror(errno), sem);
			ret = SPSR_PX_RL_SEM;
		}
#endif
	} while (0);
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int
spsr_wait_sem(void *sem)
{
	int ret = 0;
#ifndef UNIX_LINUX
#else
	int err = 0;
#endif
	do {
		if (!sem) {
			ret = SPSR_SEM_NULL_ERROR;
			break;
		}
#ifndef UNIX_LINUX
		int iswell = 0;
		iswell = WaitForSingleObject(
			(HANDLE)sem, INFINITE);
		if (iswell != WAIT_OBJECT_0) {
			DWORD dwError = GetLastError();
			spllog(SPL_LOG_ERROR, 
				"WaitForSingleObject errcode: %lu", 
				dwError);
			ret = SPSR_WIN32_WAIT_SEM;
		}
#else
		err = sem_wait((sem_t *)sem);
		if (err) {
			spllog(SPL_LOG_ERROR, "sem_post: err: %d, "
				"errno: %d, text: %s, sem: 0x%p.", 
				err, errno, strerror(errno), sem);
			ret = SPSR_PX_WAIT_SEM;
		}		
#endif
	} while (0);
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#ifndef UNIX_LINUX
int spsr_get_obj(
	char *portname, 
	void **obj, int takeoff)
{
	int ret = 0;
	SPSR_ROOT_TYPE *t = &spsr_root_node;
	SPSR_ARR_LIST_LINED *node = 0, *prev = 0;
	;
	int found = 0;
	do {
		if (!obj) {
			break;
		}
		spsr_mutex_lock(t->mutex);
		node = t->init_node;
		while (node) {
			if (strcmp(portname, 
				node->item->port_name) == 0) 
			{
				*obj = node;
				found = 1;
				if(!takeoff) {
					break;
				}
				
				if (prev) {
					prev->next = node->next;
					if (!prev->next) {
						t->last_node = prev;
					}
				} else {
					t->init_node = t->init_node->next;
				}
				t->count--;
				if (t->count < 1) {
					t->init_node = 0;
					t->last_node = 0;
				}
				
				break;
			}
			prev = node;
			node = node->next;
		}
		spsr_mutex_unlock(t->mutex);
	} while (0);

	if (!found) {
		spllog(SPL_LOG_WARNING, 
			"Cannot find port: %s", portname);
		ret = SPSR_ITEM_NOT_FOUND;
	}

	return ret;
}
#else
#endif

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

#ifndef UNIX_LINUX
int spsr_create_thread(
	SPSR_THREAD_ROUTINE f, void *arg)
{
	int ret = 0;
	DWORD dwThreadId = 0;
	HANDLE hThread = 0;
	hThread = CreateThread(
		NULL, 0, f, arg, 0, &dwThreadId);
	if (!hThread) {
		ret = SPSR_THREAD_W32_CREATE;
		spllog(SPL_LOG_DEBUG, 
			"CreateThread error: %d", 
			(int)GetLastError());
	}
	return ret;
}
#else
#endif
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#ifndef UNIX_LINUX
int
spsr_clear_node(SPSR_ARR_LIST_LINED *node)
{
	int ret = 0;
	int isoff = 0;
	SPSR_ROOT_TYPE *t = &spsr_root_node;
	do {
		if (!node) {
			ret = SPSR_PARAM_NULL;
			break;
		}
		spsr_mutex_lock(node->item->mtx_off);
		/*do {*/
		node->item->isoff = 1;
		/*} while (0); */
		spsr_mutex_unlock(node->item->mtx_off);
		SetEvent(node->item->hEvent);
		spsr_wait_sem(node->item->sem_off);

		SPSR_CloseHandle(node->item->mtx_off);
		SPSR_CloseHandle(node->item->sem_off);
		spsr_mutex_lock(t->mutex);
		isoff = t->spsr_off;
		spsr_mutex_unlock(t->mutex);
		if (!isoff) {
			
			SPSR_module_cb cb_fn = 0;
			void *cb_obj = 0;
			int eventcb = 0;
			int l = 0;
			SPSR_GENERIC_ST *ecb_buf = 0;
			char *tbuffer = 0;
			char evtbytes[SPSR_EVT_CB_PORT_LEN] = {0};
			ecb_buf = (SPSR_GENERIC_ST *)evtbytes;
			ecb_buf->total = SPSR_EVT_CB_PORT_LEN;
			ecb_buf->range = SPSR_PORT_LEN;
			ecb_buf->pl = ecb_buf->pc = sizeof(void *);
			tbuffer = ecb_buf->data + ecb_buf->pc;			
			cb_fn = node->item->cb_evt_fn;
			cb_obj = node->item->cb_obj;

			eventcb = node->item->handle ? 
				SPSR_EVENT_CLOSE_DEVICE_ERROR : 
				SPSR_EVENT_CLOSE_DEVICE_OK;
			l = (int)strlen(node->item->port_name);

			memcpy(tbuffer, node->item->port_name, l);
			ret = spsr_invoke_cb(
				eventcb,
				cb_fn, 
				cb_obj, 
				ecb_buf, 
				l);
		}
		spsr_free(node->item->buff);

	} while (0);
	return ret;
}
#endif
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spsr_inst_write(
	char *portname, char *data, int sz)
{
	int ret = 0;
	
	do {
#ifndef UNIX_LINUX
		SPSR_ARR_LIST_LINED *node = 0;
		SPSR_INFO_ST *item = 0;
		void *p = 0;
		ret = spsr_get_obj(portname, &p, 0);
		if (!p) {
			ret = SPSR_NOT_FOUND_IDD;
			spllog(SPL_LOG_ERROR, 
				"Cannot find the object.");
			break;
		}
		node = (SPSR_ARR_LIST_LINED *)p;
		item = node->item;
		spsr_mutex_lock(item->mtx_off);
		do {
			int total = 0;
			char *tmp = 0;
			int range = 0;
			int pl = 0;

			if (!item->buff)
			{
				int step = 0;
				step = SPSR_MAX_AB(
					sz, SPSR_STEP_MEM);
				step += sizeof(SPSR_GENERIC_ST);
				ret = spsr_resize_obj(
					step, &(item->buff));

				if(ret) {
					spllog(SPL_LOG_ERROR, 
						"spsr_resize_obj.");					
					break;
				}
				tmp = item->buff->data;
				memcpy(tmp, data, sz);
				item->buff->pl = sz;

				break;
			}
			range = item->buff->range;
			pl = item->buff->pl;
			if (range > pl + sz) 
			{
				tmp = item->buff->data;
				tmp += item->buff->pl;				
				memcpy(tmp , data, sz);
				item->buff->pl += sz;
				break;
			} 

			total = item->buff->total;
			total += SPSR_MAX_AB(
				sz, SPSR_STEP_MEM);
			ret = spsr_resize_obj(
				total, &(item->buff));
			if(ret) {
				spllog(SPL_LOG_ERROR, 
					"spsr_resize_obj.");					
				break;
			}
			tmp = item->buff->data;
			tmp += item->buff->pl;
			memcpy(tmp, data, sz);
			item->buff->pl += sz;

			break;

		} 
		while (0);
		spsr_mutex_unlock(item->mtx_off);
		if(ret) {
			break;
		}
		SetEvent(item->hEvent);
#else
		SPSR_ROOT_TYPE *t = &spsr_root_node;
		int isExisted = 0;
		spsr_mutex_lock(t->mutex);
		do {
			ret = spsr_is_existed(
				portname, &isExisted);
			if(ret) {
				break;
			}
			if(!isExisted) {
				spllog(SPL_LOG_ERROR, 
					"port %s not exsied.", 
					portname);
				ret = SPSR_PORTNAME_NONEXISTED;
				break;
			}
			ret = spsr_send_cmd(
				SPSR_CMD_WRITE, portname, data, sz);
		} while(0);
		spsr_mutex_unlock(t->mutex);
		if (ret) {
			spllog(SPL_LOG_ERROR, 
				"SEND command error ret: %d.", ret);
		}
#endif
	} while (0);
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#ifndef UNIX_LINUX
#else
#define SPSR_SIZE_CARTRIDGE        10
#define SPSR_SIZE_TRIGGER          2
#define SPSR_SIZE_MAX_EVENTS       10
#define SPSR_MSG_OFF               "SPSR_MSG_OFF"
#define SPSR_MILLION               1000000
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
void *
spsr_init_trigger_routine(void *obj)
{
	spsr_init_trigger(obj);
	return 0;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

void *spsr_init_cartridge_routine(void *obj)
{
	SPSR_ROOT_TYPE *t = &spsr_root_node;
	int ret = 0;
	int sockfd = 0;
	int isoff = 0;
	int flags = 0;
	int err = 0;

	struct sockaddr_in cartridge_addr = {0};
	int k = 0;
	char chk_delay = 0;
	SPSR_GENERIC_ST *cart_buff = 0;

	SPSR_GENERIC_ST *ecb_buf = 0;
	char evtbytes[SPSR_EVT_CB_CART_LEN] = {0};
	ecb_buf = (SPSR_GENERIC_ST *)evtbytes;
	ecb_buf->total = SPSR_EVT_CB_CART_LEN;
	ecb_buf->range = SPSR_BUFFER_SIZE;
	ecb_buf->pl = ecb_buf->pc = sizeof(void *);

#ifndef __SPSR_EPOLL__
	int n = 0;

	int mx_number = 0;
	struct pollfd fds[SPSR_SIZE_MAX_EVENTS];
	memset(&fds, 0, sizeof(fds));
	for (n = 0; n < SPSR_SIZE_MAX_EVENTS; ++n) {
		fds[n].fd = -1;
	}
	n = 0;
#else
	int epollfd = 0;
	;
	int i = 0;
	struct epoll_event event, 
		events[SPSR_SIZE_MAX_EVENTS];
#endif
	spllog(SPL_LOG_DEBUG, "cartridge: ");
	/* Creating socket file descriptor */
	spsr_malloc(SPSR_CMD_BUFF_LEN, 
		cart_buff, SPSR_GENERIC_ST);
	do {
		if(!cart_buff) {
			ret = SPSR_MEM_NULL;
			spllog(SPL_LOG_ERROR, 
				"SPSR_MEM_NULL.");
			break;
		}
		cart_buff->total = SPSR_CMD_BUFF_LEN;
		cart_buff->range = SPSR_CMD_BUFF;

		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if (sockfd < 0) {

			spllog(SPL_LOG_ERROR, 
				"fcntl: ret: %d, errno: %d, "
				"text: %s.", sockfd, 
				errno, strerror(errno));

			ret = SPSR_CREATE_SOCK;
			break;
		}

		memset(&cartridge_addr, 0, 
			sizeof(cartridge_addr));

		/* Filling server information */

		cartridge_addr.sin_family = AF_INET;
		/*
		    cartridge_addr.sin_addr.s_addr = INADDR_ANY
		*/
		cartridge_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		;
		cartridge_addr.sin_port = htons(SPSR_PORT_CARTRIDGE);

		/* Set socket to non - blocking mode */

		flags = fcntl(sockfd, F_GETFL, 0);
		if (flags == -1) {
			spllog(SPL_LOG_ERROR, "fcntl: ret: %d, "
				"errno: %d, text: %s.", 
				ret, errno, strerror(errno));
			ret = SPSR_FCNTL_SOCK;
			break;
		}
		err = fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
		if (err == -1) {

			spllog(SPL_LOG_ERROR, 
				"fcntl: err: %d, errno: %d, "
				"text: %s.", err, errno, 
				strerror(errno));

			ret = SPSR_FCNTL_SOCK;
			break;
		}

		/* Bind the socket with the server address */
		err = bind(sockfd, 
			(const struct sockaddr *)&cartridge_addr, 
			sizeof(cartridge_addr));
		if (err < 0) {
			spllog(SPL_LOG_ERROR, 
				"bind failed: err: %d, errno: %d, "
				"text: %s.", err, 
				errno, strerror(errno));
			ret = SPSR_BIND_SOCK;
			break;
		}

		while (1) {
			chk_delay = 0;
			k = 0;
			spsr_mutex_lock(t->mutex);
			/*do {*/
			isoff = t->spsr_off;
			/*} while (0);*/
			spsr_mutex_unlock(t->mutex);
			if (isoff) {
				break;
			}
			/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

#ifndef __SPSR_EPOLL__

			fds[0].fd = sockfd;
			fds[0].events = POLLIN;
			mx_number = 1;
			while (1) {
				if (isoff) {
					break;
				}
				chk_delay = 0;

				err = poll(fds, mx_number, 
					60 * 1000);

				spllog(SPL_LOG_DEBUG, 
					"poll,  mx_number: %d", 
					mx_number);

				if (err == -1) {
					continue;
				}
				if (err == 0) {
					continue;
				}
				for (k = 0; k < mx_number; ++k) {
					if (fds[k].fd < 0) {
						continue;
					}
					if (!(fds[k].revents & POLLIN)) {
						continue;
					}

					if (k == 0) {

						spsr_ctrl_sock(
							fds, 
							&mx_number, 
							sockfd, 
							ecb_buf, 
							&isoff, 
							&cart_buff);

						continue;
					}
					if (fds[k].fd >= 0) {

						ret = spsr_read_fd(
							fds[k].fd, 
							ecb_buf, 
							&chk_delay);
					}
				}
			}
#else
			/* Start epoll */
			epollfd = epoll_create1(0);
			if (epollfd < 0) {
				spllog(SPL_LOG_ERROR, 
					"epoll_create, epollfd: %d, "
					"errno: %d, text: %s.", 
					epollfd, errno,
				    strerror(errno));
				ret = SPSR_EPOLL_CREATE;
				break;
			}
			event.events = EPOLLIN | EPOLLET;
			event.data.fd = sockfd;

			err = epoll_ctl(epollfd, 
				EPOLL_CTL_ADD, sockfd, &event);
			if (err < 0) {
				spllog(
				    SPL_LOG_ERROR, 
					"epoll_ctl, err: %d, errno: %d, "
					"text: %s.", 
					err, errno, strerror(errno));
				ret = SPSR_EPOLL_CTL;
				break;
			}
			while (1) {
				int nfds = 0;
				if (isoff) {
					break;
				}
				chk_delay = 0;

				nfds = epoll_wait(
					epollfd, 
					events, 
					SPSR_SIZE_MAX_EVENTS, 
					-1);

				for (i = 0; i < nfds; i++) {
					if (events[i].data.fd == sockfd) {

						spsr_ctrl_sock(
							epollfd, 
							sockfd, 
							ecb_buf, 
							&isoff, &cart_buff);

						continue;
					}
					if (events[i].data.fd >= 0) {

						ret = spsr_read_fd(
							events[i].data.fd, 
							ecb_buf, 
							&chk_delay);

					}
					/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
				}
				/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
			}
#endif
			/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
		}
	} while (0);
	spsr_mutex_lock(t->mutex);
	/*do {*/
	t->spsr_off++;
	spsr_clear_all();
	/*} while (0);*/
	spsr_mutex_unlock(t->mutex);
	spsr_clear_hash();
	if (sockfd > 0) {
		err = close(sockfd);
		if (err) {
			spllog(SPL_LOG_ERROR, 
				"close: err: %d, "
				"errno: %d, text: %s.", 
				err, errno, strerror(errno));
			ret = SPSR_CLOSE_SOCK;
		} else {
			spllog(SPL_LOG_DEBUG, 
				"close socket done: %d", sockfd);
		}
	}
#ifndef __SPSR_EPOLL__

#else
	if (epollfd > -1) {
	}
#endif
	if(ret) {
		spllog(SPL_LOG_ERROR, 
			"ret: %d", ret);
	}
	spsr_free(cart_buff);
	spsr_rel_sem(t->sem_spsr);
	
	return 0;
}

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

int spsr_init_trigger(void *obj)
{
	SPSR_ROOT_TYPE *t = &spsr_root_node;
	int ret = 0;
	int sockfd = 0;
	int isoff = 0;
	int flags = 0;
	socklen_t len = 0;
	struct sockaddr_in trigger_addr, cartridge_addr;
	spllog(SPL_LOG_DEBUG, "trigger: ");
	char had_cmd = 0;
	do {
		/* Creating socket file descriptor */
		if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			spllog(SPL_LOG_DEBUG, 
				"fcntl: ret: %d, errno: %d, "
				"text: %s.", 
				sockfd, errno, strerror(errno));
			ret = SPSR_CREATE_SOCK;
			break;
		}

		memset(&trigger_addr, 0, sizeof(trigger_addr));
		memset(&cartridge_addr, 0, sizeof(cartridge_addr));

		/* Filling server information */
		trigger_addr.sin_family = AF_INET;
		trigger_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		;
		;
		trigger_addr.sin_port = htons(SPSR_PORT_TRIGGER);

		cartridge_addr.sin_family = AF_INET;
		cartridge_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		;
		cartridge_addr.sin_port = htons(SPSR_PORT_CARTRIDGE);

		/* Set socket to non-blocking mode */
		ret = fcntl(sockfd, F_GETFL, 0);
		if (ret == -1) {
			spllog(SPL_LOG_DEBUG, "fcntl: ret: %d, "
				"errno: %d, text: %s.", 
				ret, errno, strerror(errno));
			ret = SPSR_FCNTL_SOCK;
			break;
		}

		ret = fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
		if (ret == -1) {
			spllog(SPL_LOG_DEBUG, "fcntl: ret: %d, "
				"errno: %d, text: %s.", 
				ret, errno, strerror(errno));
			ret = SPSR_FCNTL_SOCK;
			break;
		}

		/* Bind the socket with the server address */
		ret = bind(sockfd, 
			(const struct sockaddr *)&trigger_addr, 
			sizeof(trigger_addr));
		if (ret < 0) {
			/* perror("bind failed"); */
			spllog(SPL_LOG_ERROR, 
				"bind failed: ret: %d, "
				"errno: %d, text: %s.", 
				ret, errno, strerror(errno));
			ret = SPSR_BIND_SOCK;
			break;
		}
		len = sizeof(trigger_addr);
		while (1) {
			spsr_wait_sem(t->sem);

			spsr_mutex_lock(t->mutex);
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
			spsr_mutex_unlock(t->mutex);

			if (isoff) {
				int kkk = sendto(sockfd, 
					(const char *)SPSR_MSG_OFF, 
					strlen(SPSR_MSG_OFF), SPSR_SENDSK_FLAG,
				    (const struct sockaddr *)&cartridge_addr, len);
				spllog(SPL_LOG_DEBUG, "sendto kkk: %d", kkk);
				break;
			}
			if (had_cmd) {
				sendto(sockfd, (const char *)"CMD", 
				strlen("CMD"), SPSR_SENDSK_FLAG,
				    (const struct sockaddr *)&cartridge_addr, 
					len);
			}
			had_cmd = 0;
			/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
		}

		ret = close(sockfd);
		if (ret) {
			spllog(SPL_LOG_ERROR, 
				"Close socket, ret: %d, "
				"errno: %d, text: %s.", 
				ret, errno, strerror(errno));
			ret = SPSR_CLOSE_SOCK;
		} else {
			spllog(SPL_LOG_DEBUG, 
				"Close socket DONE: %d.", sockfd);
		}
		/* Clean linked list. TODO 2.*/
		spsr_rel_sem(t->sem_spsr);
	} while (0);
	return 0;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

#ifndef __SPSR_EPOLL__
int spsr_fetch_commands(void *mp, 
	int *prange, 
	char *info, 
	int n, SPSR_GENERIC_ST *evt)
#else
int spsr_fetch_commands(int epollfd, 
	char *info, int n, SPSR_GENERIC_ST *evt)
#endif
{
	int ret = 0;
	SPSR_GENERIC_ST *item = 0;

	
	int fd = 0;

#ifndef __SPSR_EPOLL__
	struct pollfd *fds = (struct pollfd *)mp;
#else
#endif
	int step = 0;
	spllog(0, "Enter fetching command, n: %d", n);

	for (step = 0; step < n;) {
		item = (SPSR_GENERIC_ST *)(info + step);
		step += item->total;
		
		if (item->type == SPSR_CMD_ADD) 
		{
			spllog(0, "\t SPSR_CMD_ADD: %d", item->type);
		#ifndef __SPSR_EPOLL__
			ret = spsr_px_add(item, evt, prange, fds);
		#else
			ret = spsr_px_add(item, evt, epollfd);
		#endif	
			continue;
		}
		/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
		if (item->type == SPSR_CMD_REM) {
			spllog(0, "\t SPSR_CMD_REM: %d", item->type);
		#ifndef __SPSR_EPOLL__
				ret = spsr_px_rem(item, evt, prange, fds);
		#else
				ret = spsr_px_rem(item, evt, epollfd);
		#endif				
			continue;
		}
		/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
		if (item->type == SPSR_CMD_WRITE) {
			spllog(0, "\t SPSR_CMD_WRITE: %d", item->type);
			ret = spsr_px_write(item, evt);
			continue;
		}
		if (ret) {
			break;
		}
	}

	if (ret) {
		if (fd >= 0) {
			close(fd);
		}
	}

	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
/*SPSR_CMD_ADD*/
int spsr_px_hash_add( SPSR_ARR_LIST_LINED *temp, 
	SPSR_GENERIC_ST *evt) 
{
	int ret = 0;
	int fd = -1;
	do {
		SPSR_HASH_FD_NAME *hashobj = 0, *hashitem = 0;
		int hashid = 0;
		fd = temp->item->handle;
		spsr_malloc(
			sizeof(SPSR_HASH_FD_NAME), 
			hashobj, SPSR_HASH_FD_NAME);
		if (!hashobj) {
			ret = SPSR_MALLOC_ERROR;
			spllog(SPL_LOG_ERROR, "hashobj null");
			break;
		}
		hashobj->fd = fd;
		memcpy(hashobj->port_name, temp->item->port_name,
			strlen(temp->item->port_name));
		hashobj->cb_evt_fn = temp->item->cb_evt_fn;
		hashobj->cb_obj = temp->item->cb_obj;
		hashobj->t_delay = temp->item->t_delay;
		hashid = SPSR_HASH_FD(fd);
		hashitem = (SPSR_HASH_FD_NAME *)
			spsr_hash_fd_arr[hashid];
		if (!hashitem) {
			spsr_hash_fd_arr[hashid] = (void *)hashobj;
		} else {
			SPSR_HASH_FD_NAME *temp = 0;
			temp = hashobj;
			while (!temp->next) {
				temp = temp->next;
			}
			temp->next = hashitem;
		}

		if (hashobj->cb_evt_fn) {
			char *tbuff = evt->data + sizeof(void*);
			memcpy(tbuff, temp->item->port_name,
				strlen(temp->item->port_name));			
			ret = spsr_invoke_cb(
				SPSR_EVENT_OPEN_DEVICE_OK, 
				hashobj->cb_evt_fn,
				hashobj->cb_obj, 
				evt, strlen(hashobj->port_name));
		}
	} while (0);	
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
/*SPSR_CMD_ADD*/
#ifndef __SPSR_EPOLL__
int spsr_px_add(
	SPSR_GENERIC_ST *item, 
	SPSR_GENERIC_ST *evt, int *prange, struct pollfd *fds)
#else

int spsr_px_add(
	SPSR_GENERIC_ST *item, 
	SPSR_GENERIC_ST *evt, int epollfd)
#endif
{
	int ret = 0;
	SPSR_ROOT_TYPE *t = &spsr_root_node;
	SPSR_ARR_LIST_LINED *temp = 0;
	char tmp_port[SPSR_PORT_LEN + 1] = {0};
	int fd = -1;
	SPSR_INFO_ST *input = 0;
	int l = 0;
	#ifndef __SPSR_EPOLL__
	int i = 0;
#else
	struct epoll_event event = {0};
	int rerr = 0;
#endif
	spsr_mutex_lock(t->mutex);
	do 
	{
		temp = t->init_node;
		while (temp) {
			if (temp->item->handle > -1) {
				temp = temp->next;
				continue;
			}
			memset(tmp_port, 0, sizeof(tmp_port));
			input = temp->item;
			fd = -1;
			l = strlen(input->port_name);
			ret = spsr_open_fd( 
				input->port_name,  
				input->baudrate, &fd);

			if (ret) {
				if (input->cb_evt_fn) {
					ret = spsr_invoke_cb(
						SPSR_EVENT_OPEN_DEVICE_ERROR,
						input->cb_evt_fn, 
						input->cb_obj, evt, l);
				}
				break;
			}
			memcpy(tmp_port, input->port_name, l);
			temp->item->handle = fd;
#ifndef __SPSR_EPOLL__
			spllog(0, "Range of hashtable before adding, "
				"*prange: %d,  DONE.", *prange);
			if(!prange) {
				spllog(SPL_LOG_ERROR, "prange NULL");
				ret = SPSR_PX_PRANGE_NULL;
				break;
			}				
			for (i = 0; i < (*prange + 1); ++i) {
				spllog(0, "*prange: %d , "
					"fds[%d].fd: %d .", 
					*prange, i, fds[i].fd);
				if (fds[i].fd < 0) {
					fds[i].fd = fd;
					fds[i].events = POLLIN;
					(*prange)++;
					spllog(0, "Add to poll list, "
						"index: %d, fd: %d, range: %d",
						i, fd, (*prange));
					break;
				}
			}
			spllog(0, "Range of hashtable after adding, "
				"*prange: %d,  DONE.", *prange);
#else
			memset(&event, 0, sizeof(event));
			event.events = EPOLLIN | EPOLLET;
			event.data.fd = fd;
			rerr = epoll_ctl(epollfd, 
				EPOLL_CTL_ADD, fd, &event);
			if (rerr == -1) {
				spllog(SPL_LOG_ERROR,
					"epoll_ctl error, fd: %d, "
					"errno: %d, text: %s.", fd, errno,
					strerror(errno));
				ret = SPSR_UNIX_EPOLL_CTL;
				break;
			}
#endif
			ret = spsr_px_hash_add(temp, evt);

			
			temp = temp->next;
		}
	} 
	while (0);
	spsr_mutex_unlock(t->mutex);
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
/*SPSR_CMD_REM*/
static
#ifndef __SPSR_EPOLL__
int spsr_px_off_poll(int fd, 
	struct pollfd *fds, 
	int *prange )
#else
int spsr_px_off_poll(int fd, int epollfd)
#endif
{
	int ret = SPSR_PX_POLL_NOT_FOUND;
#ifndef __SPSR_EPOLL__
	int i = 0;
	spllog(0, "Did catch handle: %d", fd);
	if(!prange) {
		spllog(SPL_LOG_ERROR, "prange NULL");
		return ret;
	}
	for (i = 1; i < *prange; ++i) {
		if (fds[i].fd == fd) 
		{
			int j = 0;
			for (j = i; j < (*prange - 1); ++j) 
			{
				fds[j].fd = fds[j + 1].fd;
			}

			fds[(*prange - 1)].fd = -1;

			(*prange)--;
			spllog(SPL_LOG_DEBUG, 
				"EPOLL_CTL_DEL, " "fd: %d DONE", fd);
			ret = 0;
			break;
		}
	}
#else
	int errr = 0;
	spllog(0, "Did catch handle: %d", fd);
	errr = epoll_ctl( epollfd, EPOLL_CTL_DEL, fd, 0);
	if (errr == -1) {
		spllog(SPL_LOG_ERROR,
			"epoll_ctl error, fd: %d, "
			"errno: %d, text: %s.",
			fd, errno, strerror(errno));
		ret = SPSR_PX_EPOLL_DEL;
	} else {
		spllog(SPL_LOG_DEBUG,
			"EPOLL_CTL_DEL, "
			"fd: %d DONE",
			fd);
		ret = 0;
	}
#endif
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
static int spsr_px_off_hash(int fd) 
{
	int ret = 0;
	SPSR_HASH_FD_NAME *hashobj = 0;
	SPSR_HASH_FD_NAME *temp = 0;
	SPSR_HASH_FD_NAME *prev = 0;
	int hashid = 0;
	char found = 0;
	do {

		hashid = SPSR_HASH_FD(fd);
		hashobj = (SPSR_HASH_FD_NAME *) spsr_hash_fd_arr[hashid];
		if (!hashobj) {
			spllog( SPL_LOG_ERROR, 
				"SPSR_PX_MAL_HASH_FD Cannot find object.");
			ret = SPSR_PX_MAL_HASH_FD;
			break;
		}
		temp = hashobj;
		while (temp) {
			if(temp->fd != fd) {
				prev = temp;
				temp = temp->next;				
				continue;
			}
			/*Found fd.*/
			found = 1;
			break;
		}
		if(!found) {
			ret = SPSR_HASH_NOT_FOUND;
			break;
		}
		if (prev) {
			prev->next = temp->next;
		} else {
			spsr_hash_fd_arr[hashid] = temp->next;
		}
		spllog(SPL_LOG_DEBUG,
			"Clear from spsr_hash_fd_arr, "
			"hashid:%d, fd: %d.", hashid, fd);
		spsr_free(temp);
		break;		
	} while (0);	
	if(ret) {
		spllog(SPL_LOG_ERROR, 
			"SPSR_HASH_NOT_FOUND Cannot find object.");		
	}
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
/*SPSR_CMD_REM*/
#ifndef __SPSR_EPOLL__
int spsr_px_rem(
	SPSR_GENERIC_ST *item, 
	SPSR_GENERIC_ST *evt, 
	int *prange, 
	struct pollfd *fds)	
#else
int spsr_px_rem(
	SPSR_GENERIC_ST *item, 
	SPSR_GENERIC_ST *evt, int epollfd)	
#endif

{
	int ret = 0;
	char *portname = 0;
	SPSR_ROOT_TYPE *t = &spsr_root_node;
	int fd = -1;
	int l = 0;
	SPSR_ARR_LIST_LINED *temp = 0;
	SPSR_ARR_LIST_LINED *prev = 0;
	SPSR_module_cb callback_fn = 0;
	void *callback_obj = 0;
	int callback_evt = 0;
	int errr = 0;
	char found = 0;
	do {
		if(!item) {
			ret = SPSR_PX_ITEM_NULL;
			break;
		}
		if(!evt) {
			ret = SPSR_PX_CB_NULL;
			break;
		}	
	#ifndef __SPSR_EPOLL__
		if(!prange) {
			ret = SPSR_PX_PRANGE_NULL;
			break;
		}	
		if(!fds) {
			ret = SPSR_PX_POLLFD_NULL;
			break;
		}			
	#else
	#endif
		portname = item->data;
		l = strlen(portname);
		spllog(0, "port: %s", item->data);
		spsr_mutex_lock(t->mutex);

		do {
			temp = t->init_node;
			spllog(0,
				"SPSR_CMD_REM command, "
				"pl: %d, "
				"portname: %s, "
				"total: %d, "
				"initnode: 0x%p",
				item->pl, portname, item->total, temp);
			while (temp) 
			{
				spllog(0, "portname: %s", temp->item->port_name);
				if(strcmp(temp->item->port_name, portname)) {
					prev = temp;
					temp = temp->next;					
					continue;
				}
				found = 1;
				break;
			}
			if(!found) {
				ret = SPSR_REM_NOT_FOUND;
				spllog( SPL_LOG_ERROR, 
					"SPSR_REM_NOT_FOUND, port: %s.", portname);
				break;
			}
			fd = temp->item->handle;
			if(fd < 0) {
				ret = SPSR_PX_MALINFO_FD;
				break;
			}
			callback_fn = temp->item->cb_evt_fn;
			callback_obj = temp->item->cb_obj;
			/* Remove fd out of epoll*/
#ifndef __SPSR_EPOLL__
			ret = spsr_px_off_poll(fd, fds, prange);
#else
			ret = spsr_px_off_poll(fd, epollfd);
#endif
			if(ret) {
				spllog( SPL_LOG_ERROR, 
					"spsr_px_off_poll, ret: %d.", ret);
			}
			ret = spsr_px_off_hash(fd);
			/* Close handle*/
			errr = close(fd);
			if (errr) {
				callback_evt = 
					SPSR_EVENT_CLOSE_DEVICE_ERROR;
				spllog(SPL_LOG_ERROR,
					"close error, "
					"fd: %d, "
					"errno: %d, text: %s.",
					fd, errno, strerror(errno));
			} else {
				callback_evt = 
					SPSR_EVENT_CLOSE_DEVICE_OK;
				spllog(
					SPL_LOG_DEBUG, "close, fd: %d, DONE", fd);
			}
			/* Remove out of root list*/
			if (t->count < 2) {
				t->init_node = 0;
				t->last_node = 0;
			} else {
				if (prev) {
					prev->next = temp->next;
					if (!prev->next) {
						t->last_node = prev;
					}
				} else {
					t->init_node = temp->next;
				}
			}
			spsr_free(temp->item);
			spsr_free(temp);
			t->count--;
			spllog( SPL_LOG_DEBUG, "t->count: %d", t->count);
			break;			
		} while (0);
		spsr_mutex_unlock(t->mutex);

		if (!callback_fn) {
			break;
		}

		memcpy( evt->data + evt->pc,  portname, l);

		ret = spsr_invoke_cb( callback_evt,  
				callback_fn, callback_obj, evt, l);
				
	} while(0);
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
/* SPSR_CMD_WRITE */
int spsr_px_write(
	SPSR_GENERIC_ST *item, 
	SPSR_GENERIC_ST *evt) 
{
	int ret = 0;
	char *portname = 0;
	SPSR_ROOT_TYPE *t = &spsr_root_node;
	int fd = -1;
	SPSR_ARR_LIST_LINED *temp = 0;
	int kcmp = 0;
	int nwrote = 0, wlen = 0;
	char *p = 0;
	SPSR_HASH_FD_NAME *hashobj = 0;
	int hashid = 0;
	int wrote = 0;
	int evtenum = 0;
	int l = 0;
	do {
		if(!item) {
			ret = SPSR_PX_ITEM_NULL;
			spllog(SPL_LOG_ERROR, 
				"SPSR_PX_ITEM_NULL");				
			break;
		}
		if(!evt) {
			ret = SPSR_PX_CB_NULL;
			spllog(SPL_LOG_ERROR, 
				"SPSR_PX_CB_NULL");			
			break;
		}		
		portname = item->data;
		l = strlen(portname);
		spsr_mutex_lock(t->mutex);
		/*do { */
			temp = t->init_node;
			spllog(0, "SPSR_CMD_WRITE, pl: %d, "
				"portname: %s, total: %d, initnode: 0x%p",
				item->pl, portname, item->total, temp);
			while (temp) {
				spllog(0, 
					"portname: %s", 
					temp->item->port_name);
				kcmp = strcmp(
					temp->item->port_name, 
					portname) 
					? 0 : 1;
				if(!kcmp) {
					temp = temp->next;
					continue;
				}
				if (temp->item->handle < 0) {
					spllog(SPL_LOG_ERROR, 
						"temp->item->handle: %d",
						temp->item->handle);
					break;
				} 
				fd = temp->item->handle;
				break;
			}
		/*} while (0);*/
		spsr_mutex_unlock(t->mutex);

		if(fd < 0) {
			spllog(SPL_LOG_ERROR, 
				"SPSR_PX_FD_NOT_FOUND");
			ret = SPSR_PX_FD_NOT_FOUND;
			break;
		}	
		
		hashid = SPSR_HASH_FD(fd);

		p = item->data + item->pc;
		wlen = item->pl - item->pc;

		hashobj = (SPSR_HASH_FD_NAME *)
			spsr_hash_fd_arr[hashid];

		if (tcflush(fd, TCIOFLUSH) == -1) 
		{
			spllog(SPL_LOG_ERROR, 
				"Error flushing the serial port buffer");
			break;
		} 
		else {
			spllog(SPL_LOG_DEBUG, "tcdrain DONE,");
		}

		nwrote = write(fd, p, wlen);
		if (nwrote != wlen) {
			spllog(SPL_LOG_ERROR,
				"write error, fd: %d, "
				"errno: %d, text: %s.",
				fd, errno, strerror(errno));		
			break;
		}
		wrote = 1;
		spllog(SPL_LOG_DEBUG, 
			"write DONE, fd: %d, nwrote: %d, wlen: %d.",
			fd, nwrote, wlen);
		break;

	} while(0);	

	do {
		if(!hashobj) {
			break;
		}
		if(! (hashobj->cb_evt_fn)) {
			break;
		}	
		if(l < 1) {
			portname = (char*)"EMPTY";
			l = strlen(portname);
		}
		evtenum = wrote ? 
			SPSR_EVENT_WRITE_OK : 
			SPSR_EVENT_WRITE_ERROR;

		memcpy(evt->data + evt->pc, 
			portname, l);
		
		ret = spsr_invoke_cb(
			evtenum, 
			hashobj->cb_evt_fn,
			hashobj->cb_obj, evt, l);
	} while(0);

	if(!ret && fd >= 0) {
		if (tcdrain(fd) == -1) 
		{
			spllog(SPL_LOG_ERROR, 
				"Error flushing the serial port buffer");

		} else {
			spllog(SPL_LOG_DEBUG, "tcdrain DONE,");
		}		
	}

	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spsr_send_cmd(
	int cmd, 
	char *portname, void *data, int datasz)
{
	int ret = 0;
	int nsize = 0;
	int *pend = 0;
	SPSR_ROOT_TYPE *t = &spsr_root_node;
	SPSR_GENERIC_ST *obj = 0;
	do {
		if (cmd == SPSR_CMD_ADD) {
			
			nsize = sizeof(SPSR_GENERIC_ST);

			if (t->cmd_buff->range > 
				(t->cmd_buff->pl + sizeof(obj))) 
			{
				obj = (SPSR_GENERIC_ST *)
					(t->cmd_buff->data + t->cmd_buff->pl);
				memset(obj, 0, nsize);
				obj->total = nsize;
				obj->type = cmd;

				t->cmd_buff->pl += nsize;
				pend = (int *)(t->cmd_buff->data + t->cmd_buff->pl);
				*pend = 0;
				spllog(0, 
					"cmd type SPSR_CMD_ADD: %d, size: %d", 
					obj->type, obj->total);
			}
			break;
		}
		if (cmd == SPSR_CMD_REM) {
			int lport = 0;
			int len = strlen(portname);

			lport = len + 1;
			nsize = sizeof(SPSR_GENERIC_ST) + lport;
			spllog(0, 
				"SPSR_CMD_REM, nsize: %d, portname: %s", 
				nsize, portname);
			if (t->cmd_buff->range > t->cmd_buff->pl + nsize) {
				obj = (SPSR_GENERIC_ST *)
					(t->cmd_buff->data + t->cmd_buff->pl);
				memset(obj, 0, nsize);
				obj->total = nsize;
				obj->type = cmd;
				obj->range = lport;
				memcpy(obj->data, portname, lport);
				obj->data[len] = 0;
				obj->pl = lport;
				t->cmd_buff->pl += nsize;
				pend = (int *)(t->cmd_buff->data + t->cmd_buff->pl);
				*pend = 0;
			}
			break;
		}
		if (cmd == SPSR_CMD_WRITE) {
			int lport = 0;
			int range  = 0;
			int pl = 0;
			char *tmp = 0;
			int len = strlen(portname);

			
			lport = (len + 1) + datasz;
			nsize = sizeof(SPSR_GENERIC_ST) + lport;
			spllog(0, 
				"SPSR_CMD_WRITE, nsize: %d, "
				"portname: %s", nsize, portname);
			range = t->cmd_buff->range;
			pl = t->cmd_buff->pl;
			if (range < pl + nsize) 
			{
				int total = 0;
				int adding = 2 * nsize;

				total = t->cmd_buff->total;
				total += adding;
				ret = spsr_resize_obj(
					total, &t->cmd_buff);
				if(ret) {
					break;
				}
			}
			tmp = t->cmd_buff->data;
			tmp += t->cmd_buff->pl;
			obj = (SPSR_GENERIC_ST *)tmp;;
			memset(obj, 0, nsize);
			obj->total = nsize;
			obj->type = cmd;
			obj->range = lport;
			memcpy(obj->data, portname, len);
			obj->data[len] = 0;
			obj->pc = len + 1;
			memcpy(obj->data + obj->pc, 
				(char *)data, datasz);
			obj->pl = lport;
			t->cmd_buff->pl += nsize;

			tmp = t->cmd_buff->data;
			tmp += t->cmd_buff->pl;			
			pend = (int *)tmp;
			*pend = 0;

			break;
		}
	} while (0);
	spsr_rel_sem(t->sem);
	return ret;
}

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#endif

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spsr_is_existed(char *port, int *isExisted) {
	int ret = 0;
	SPSR_ROOT_TYPE *t = &spsr_root_node;
	SPSR_ARR_LIST_LINED *tmp = 0;
	char *p1 = 0;
	
	do {
		if(!isExisted) {
			ret = SPSR_OBJ_NULL;
			spllog(SPL_LOG_ERROR, 
				"SPSR_OBJ_NULL.");
			break;
		}		
		*isExisted = 0;
		if(!port) {
			ret = SPSR_PORT_NULL;
			spllog(SPSR_PORT_NULL, 
				"SPSR_OBJ_NULL.");			
			break;
		}
		if(!port[0]) {
			ret = SPSR_PORT_EMPTY;
			spllog(SPSR_PORT_EMPTY, 
				"SPSR_OBJ_NULL.");				
			break;
		}		
		if (!t->init_node) {
			break;
		}
				
		tmp = t->init_node;
		
		while (tmp) {
			p1 = tmp->item->port_name;
			if (strcmp(p1, port))  {
				tmp = tmp->next;
				continue;
			}
			*isExisted= 1;
			break;
		}
	} while(0);
	return ret;
}
int spsr_verify_info(SPSR_INPUT_ST *p)
{
	SPSR_ROOT_TYPE *t = &spsr_root_node;
	int ret = 0;
	SPSR_INFO_ST *item = 0;
	SPSR_ARR_LIST_LINED *node = 0;
	spllog(SPL_LOG_DEBUG, "spsr_verify_info:");
#ifndef UNIX_LINUX
	HANDLE hSerial = 0;
#else
	int fd = 0;
#endif
	do {
		if (!p) {
			ret = SPSR_PORT_INPUT_NULL;
			spllog(SPL_LOG_ERROR, 
				"SPSR_PORT_INPUT_NULL.");
			break;
		}
		if (p->baudrate < 1) {
			ret = SPSR_PORT_BAUDRATE_ERROR;
			spllog(SPL_LOG_ERROR, 
				"SPSR_PORT_BAUDRATE_ERROR.");
			break;
		}
		if (!p->port_name[0]) {
			spllog(SPL_LOG_DEBUG, 
				"port_name empty.");
			ret = SPSR_PORT_NAME_ERROR;
			break;
		}
		if (t->init_node) {
			SPSR_ARR_LIST_LINED *tmp = 0;
			char *p1 = 0;
			char *p2 = 0;
			tmp = t->init_node;
			p2 = p->port_name;
			while (tmp) {
				p1 = tmp->item->port_name;
				if (strcmp(p1, p2))  {
					tmp = tmp->next;
					continue;
				}
				spllog(SPL_LOG_ERROR, 
					"SPSR_PORTNAME_EXISTED: \"%s\".", 
					p->port_name);
				ret = SPSR_PORTNAME_EXISTED;
				break;
			}
		}
		if (ret) {
			break;
		}

#ifndef UNIX_LINUX
		/* Open the serial port with FILE_FLAG_OVERLAPPED for asynchronous operation */
		/* https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea */
		hSerial =
		    CreateFile(p->port_name, 
				GENERIC_READ | GENERIC_WRITE, 
				0, 0, 
				OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);

		if (hSerial == INVALID_HANDLE_VALUE) {
			DWORD dwError = GetLastError();
			spllog(SPL_LOG_ERROR, 
				"Open port errcode: %lu", dwError);
			ret = SPSR_PORT_OPEN;
			break;
		} else {
			spllog(SPL_LOG_DEBUG, 
				"Create hSerial: 0x%p.", hSerial);
			SPSR_CloseHandle(hSerial);
		}
#else
		fd = open(
			p->port_name, 
			O_RDWR | O_NOCTTY | O_NDELAY);
		if (fd == -1) {
			ret = SPSR_PORT_OPEN_UNIX;
			spllog(SPL_LOG_ERROR, "open port: ret: %d, "
				"errno: %d, text: %s.", 
				ret, errno, strerror(errno));
			break;
		}
		spllog(SPL_LOG_DEBUG, "open portname: %s, "
			"fd: %d.", p->port_name, fd);
		ret = close(fd);
		if (ret) {
			ret = SPSR_PORT_CLOSE_UNIX;
			spllog(SPL_LOG_ERROR, "close port fd: %d, "
				"ret: %d, errno: %d, text: %s.", 
				fd, ret, errno,
			    strerror(errno));
			break;
		}
#endif

		spsr_malloc(
			sizeof(SPSR_ARR_LIST_LINED), 
			node, SPSR_ARR_LIST_LINED);
		if (!node) {
			ret = SPSR_MEM_NULL;
			spllog(SPL_LOG_ERROR, "SPSR_MEM_NULL");
			break;
		}
		spsr_malloc(sizeof(SPSR_INFO_ST), 
			item, SPSR_INFO_ST);
		if (!item) {
			ret = SPSR_MEM_NULL;
			spllog(SPL_LOG_ERROR, "SPSR_MEM_NULL");
			break;
		}

		/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

		snprintf(item->port_name, 
			SPSR_PORT_LEN, "%s", p->port_name);
		item->baudrate = p->baudrate;
		item->cb_evt_fn = p->cb_evt_fn;
		item->cb_obj = p->cb_obj;
		item->t_delay = p->t_delay;
		node->item = item;

		/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

		if (!t->init_node) {
			t->init_node = node;
			t->last_node = node;
		} else {
			t->last_node->next = node;
			t->last_node = node;
		}
		t->count++;
#ifndef UNIX_LINUX
		ret = spsr_create_thread(
			spsr_thread_operating_routine, node);
#else
		item->handle = -1;
		spllog(SPL_LOG_DEBUG, 
			"Check t->init_node: 0x%p", t->init_node);
		spsr_send_cmd(SPSR_CMD_ADD, 0, 0, 0);
#endif

	} while (0);

	if (ret) {
		if (item) {
			spsr_free(item);
		}
		if (node) {
			spsr_free(node);
		}
	}

	return ret;
}

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

int
spsr_clear_all()
{
	int ret = 0;
	SPSR_ROOT_TYPE *t = &spsr_root_node;
#ifndef UNIX_LINUX
	int count = 0;
	char port[64];
	int l = 0;
	char *p = 0;	
	do {

		memset(port, 0, sizeof(port));
		spsr_mutex_lock(t->mutex);
		count = t->count;
		if (t->init_node) {
			p = t->init_node->item->port_name;
			l = (int)strlen(p);
			memcpy(port, p, l);
		} else {
			count = 0;
		}
		spsr_mutex_unlock(t->mutex);
		if(!port[0]) {
			continue;
		}
		ret = spsr_inst_close(port);
		if(!ret) {
			continue;
		}
		spllog(SPL_LOG_ERROR,
			"spsr_inst_close: ret: %d, port: %s.",
			ret, port);
	} while (count);
#else

	SPSR_ARR_LIST_LINED *tnode = 0, *temp = 0;
	temp = t->init_node;
	while (temp) {
		int fd = -1;
		tnode = temp;
		temp = temp->next;
		if(!tnode->item) {
			spsr_free(tnode);
			continue;
		}
		if(tnode->item->handle < 0) {
			spsr_free(tnode);
			continue;			
		}
		fd = tnode->item->handle;
		ret = close(fd);
		if (ret) {
			spllog(SPL_LOG_ERROR, 
				"close: ret: %d, errno: %d, text: %s.", 
				ret, errno, strerror(errno));
		} else {
			spllog( SPL_LOG_DEBUG,"closed fd: %d.", fd);
		}
		spsr_free(tnode->item);
		spsr_free(tnode);
	}
	t->init_node = t->last_node = 0;
	spsr_free(t->cmd_buff);
#endif
	return ret;
}

#ifndef UNIX_LINUX
#else
/*
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
*/

#ifndef __SPSR_EPOLL__
int
spsr_fmt_name(char *input, char *output, int l)
{
	SPSR_ROOT_TYPE *t = &spsr_root_node;
	int ret = 0;
	if (input) {
		snprintf(output, l, 
			"%s_%s", t->sem_key, input);
	} else {
		snprintf(output, l, 
			"%s_%s", t->sem_key, "");
	}
	return ret;
}
#else
#endif
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

int
spsr_clear_hash()
{
	int ret = 0;
	int i = 0;
	SPSR_HASH_FD_NAME *tmp = 0, *obj = 0;
	for (i = 0; i < SPSR_MAX_NUMBER_OF_PORT; ++i) {
		obj = (SPSR_HASH_FD_NAME *)spsr_hash_fd_arr[i];
		if (!obj) {
			continue;
		}
		while (obj) {
			tmp = obj;
			obj = obj->next;
			spllog(0, "fd: %d, name: %s", 
				tmp->fd, tmp->port_name);
			spsr_free(tmp);
		}
		spsr_hash_fd_arr[i] = 0;
	}
	return ret;
}

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

int spsr_open_fd(
	char *port_name, 
	int baudrate, int *outfd)
{
	int ret = 0;
	int fd = -1;
	struct termios options = {0};
	int rerr = 0;
	do {
		fd = open(port_name, 
			O_RDWR | O_NOCTTY | O_NONBLOCK | O_SYNC);
		if (fd == -1) {
			spllog(SPL_LOG_ERROR, 
				"open port error, fd: %d, "
				"errno: %d, text: %s.", 
				fd, errno, strerror(errno));
			ret = SPSR_UNIX_OPEN_PORT;
			break;
		}
		spllog(0, 
			"fd: %d, portname: %s", 
			fd, port_name);
		memset(&options, 0, sizeof(options));
		rerr = tcgetattr(fd, &options);
		if (rerr < 0) {
			spllog(SPL_LOG_ERROR, 
				"tcgetattr error, fd: %d, "
				"errno: %d, text: %s.", 
				fd, errno, strerror(errno));
			ret = SPSR_UNIX_GET_ATTR;
			break;
		}

		spllog(0, "fd: %d, "
			"portname: %s, rate: %d", 
			fd, port_name, baudrate);

		cfsetispeed(&options, baudrate);
		cfsetospeed(&options, baudrate);

		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		options.c_cflag &= ~CSIZE;
		options.c_cflag |= CS8;
		/* //options.c_cflag &= ~CRTSCTS;   */
#if 1
		options.c_cflag |= CRTSCTS; 
#endif
		/* Enable RTS/CTS hardware flow control */
		options.c_iflag = IGNPAR;
		options.c_cflag |= CREAD | CLOCAL;
		options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
		/* options.c_iflag &= ~(IXON | IXOFF | IXANY);  */
		options.c_iflag |= (IXON | IXOFF | IXANY); 
		/* Enable XON/XOFF software flow control */
		options.c_oflag &= ~OPOST;

		/*
		options.c_cc[VMIN]= 1;
		options.c_cc[VTIME]= 0;
		*/

		rerr = tcsetattr(fd, TCSANOW, &options);
		if (rerr < 0) {
			spllog(SPL_LOG_ERROR, 
				"tcsetattr error, fd: %d, "
				"errno: %d, text: %s.", 
				fd, errno, strerror(errno));
			ret = SPSR_UNIX_SET_ATTR;
			break;
		} else {
			spllog(0, "tcsetattr: DONE.")
		}
		*outfd = fd;
	} while (0);

	return ret;
}

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

int spsr_read_fd(
	int fd, 
	SPSR_GENERIC_ST *pevtcb, 
	char *chk_delay)
{
	int ret = 0;
	int didread = 0;
	int comfd = fd;
	struct timespec nap_time = {0};

	SPSR_HASH_FD_NAME *hashobj = 0, *temp = 0;
	int t_wait = 0;
	int range = 0;
	SPSR_GENERIC_ST *evtcb = 0;
	char *buffer = 0;
	do {
		if (!pevtcb) {
			ret = SPSR_MEM_NULL;
			spllog(SPL_LOG_ERROR, "pevtcb NULL");
			break;
		}
		evtcb = pevtcb;
		if (!evtcb) {
			ret = SPSR_MEM_NULL;
			spllog(SPL_LOG_ERROR, "evtcb NULL");
			break;
		}
		buffer = evtcb->data + evtcb->pc;
		do {
			int hasdid = SPSR_HASH_FD(comfd);

			hashobj = (SPSR_HASH_FD_NAME *)
				spsr_hash_fd_arr[hasdid];
			if (!hashobj) {
				spllog(SPL_LOG_ERROR, 
					"Cannot find obj in reading.");
				ret = SPSR_HASH_NOTFOUND;
				break;
			}
			temp = hashobj;
			while (temp) {
				if (temp->fd == comfd) {
					t_wait = temp->t_delay;
					break;
				}
				temp = temp->next;
			}

			/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
			if (!chk_delay[0]) {
				nap_time.tv_nsec = t_wait * SPSR_MILLION;
				nanosleep(&nap_time, 0);
				chk_delay[0] = 1;
			}
			/* while(1) { */
			range = evtcb->range;
			didread = (int)read(comfd, buffer, range);
			if (didread < 1) {
				spllog(
				    SPL_LOG_ERROR, 
					"read error, fd: %d, "
					"errno: %d, text: %s.", 
					fd, errno, strerror(errno));
				break;
			}
			/* } */
			buffer[didread] = 0;

			spllog(
			    0, "Didread: %d, data: \"%s\", "
				"fd: %d, temp->t_delay/timeout: %d", 
				didread, buffer, fd, t_wait);

			if (!temp) {
				ret = SPSR_HASH_NOTFOUND;
				spllog(SPL_LOG_ERROR, 
					"Didsee Cannot find obj in reading.");
				break;
			}
			if (!temp->cb_evt_fn) {
				spllog(SPL_LOG_DEBUG, 
					"cb_evt_fn, cb_evt_fn null.");
				break;
			}

			ret = spsr_invoke_cb(
				SPSR_EVENT_READ_BUF, 
				temp->cb_evt_fn, 
				temp->cb_obj, 
				evtcb, didread);

		} while (0);

	} while (0);
	return ret;
}

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

static int spsr_check_resz( 
	int lenp, 
	SPSR_GENERIC_ST **pcart_buff) 
{

	int ret = 0;

	if(lenp <= (*pcart_buff)->range) {
		return ret;
	}

	do {

		int add = 0;
		int total = 0;	

		add = lenp - (*pcart_buff)->range + 1;
		total = (*pcart_buff)->total + add;
		ret = spsr_resize_obj(total, pcart_buff);

	} while(0);

	return ret;
}

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

#ifndef __SPSR_EPOLL__
int spsr_ctrl_sock(
	void *fds, 
	int *mx_number, 
	int sockfd, 
	SPSR_GENERIC_ST *evt, 
	int *chk_off, SPSR_GENERIC_ST **pcart_buff)
#else
int
spsr_ctrl_sock(
	int epollfd, 
	int sockfd, 
	SPSR_GENERIC_ST *evt, 
	int *chk_off, SPSR_GENERIC_ST **pcart_buff)
#endif
{
	SPSR_ROOT_TYPE *t = &spsr_root_node;
	int ret = 0;
	char *p = 0;
	int lenp = 0;
	struct sockaddr_in client_addr;
	ssize_t lenmsg = 0;
	socklen_t client_len = 0;
	int isoff = 0;
	char *buffer = evt->data + evt->pc;
	do {
		if (!evt) {
			ret = SPSR_PARAM_NULL;
			break;
		}
		if (!pcart_buff) {
			ret = SPSR_PARAM_NULL;
			break;
		}		
		while (1) {
			memset(&client_addr, 0, sizeof(client_addr));
			client_len = sizeof(client_addr);
			lenmsg = (int)recvfrom(
				sockfd, 
				buffer, evt->range, 0, 
				(struct sockaddr *)&client_addr, 
				&client_len);

			if (lenmsg < 1) {
				/* 11: Have no data in Linux */
				/* 35: Have no data in Linux */
#ifndef __SPSR_EPOLL__
				if (errno == 35 || errno == 11) {
					break;
				}
#else
				if (errno == 35 || errno == 11) {
					break;
				}
#endif
				spllog(SPL_LOG_ERROR, 
					"mach recvfrom, "
					"lenmsg: %d, errno: %d, text: %s.", 
					(int)lenmsg, errno,
				    strerror(errno));
				break;
			}

			buffer[lenmsg] = 0;
			spllog(SPL_LOG_DEBUG, "buffer: %s", buffer);
			if (strcmp(buffer, SPSR_MSG_OFF) == 0) {
				spllog(SPL_LOG_DEBUG, SPSR_MSG_OFF);
				isoff = 1;
				break;
			}
			if (isoff) {
				break;
			}
			lenp = 0;

			p = 0;
			(*pcart_buff)->pl = 0;
			spsr_mutex_lock(t->mutex);
			do {
				if(!t->cmd_buff) {
					ret = SPSR_MEM_NULL;
					spllog(SPL_LOG_ERROR, 
						"SPSR_MEM_NULL");
					break;
				}

				lenp = t->cmd_buff->pl;	

				if(!lenp) {
					break;
				}

				spsr_check_resz(lenp, pcart_buff);

				p = (*pcart_buff)->data;
				memcpy(p, t->cmd_buff->data, lenp);
				(*pcart_buff)->pl = lenp;
				t->cmd_buff->pl = 0;

			} while (0);

			spsr_mutex_unlock(t->mutex);

			if(!p) {
				continue;
			}

#ifndef __SPSR_EPOLL__
			ret = spsr_fetch_commands(
				fds, mx_number, p, lenp, evt);
			spllog(0, "MACH POLL --->>> "
				"Size of buffer of command : %d", lenp);
#else
			ret = spsr_fetch_commands(epollfd, p, lenp, evt);
			spllog(0, "LINUX EPOLL --->>> "
				"Size of buffer of command : %d", lenp);
#endif

		}
	} while (0);

	if (isoff) {
		if (chk_off) {
			*chk_off = isoff;
		}
	}

	return ret;
}
#endif

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int
spsr_mutex_delete(void *mtx)
{
	int ret = 0;
	do {
#ifndef UNIX_LINUX
		SPSR_CloseHandle(mtx);
#else
		ret = pthread_mutex_destroy((pthread_mutex_t *)mtx);
		spllog(0, "Delete 0x%p", mtx);
		spsr_free(mtx);
#endif
	} while (0);
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spsr_sem_delete(void *sem, char *sem_name)
{
	int ret = 0;
	do {
#ifndef UNIX_LINUX
		SPSR_CloseHandle(sem);
#else
#ifndef __SPSR_EPOLL__
		char name[SPSR_KEY_LEN * 2];
		int err = 0;
		spsr_fmt_name(sem_name, 
			name, SPSR_KEY_LEN * 2);
		err = sem_close((sem_t *)sem);
		if (err == -1) {
			ret = SPSR_SEM_CLOSE;
			spllog(SPL_LOG_ERROR, 
				"mach sem_close, "
				"err: %d, errno: %d, text: %s.", 
				(int)err, 
				errno, 
				strerror(errno));
		}

		spllog(0, "Sem Delete 0x%p,  "
			"sem_name: %s, name: %s", 
			sem, sem_name, name);

		err = sem_unlink(name);
		if (err) {
			ret = SPSR_SEM_UNLINK;
			spllog(SPL_LOG_ERROR, 
				"mach sem_unlink, "
				"err: %d, errno: %d, text: %s.", 
				(int)err, errno,
			    strerror(errno));
		}
		/* spsr_free(sem); */
#else
		ret = sem_destroy((sem_t *)sem);
		spllog(0, "Delete 0x%p", sem);
		spsr_free(sem);
#endif
#endif
	} while (0);
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spsr_invoke_cb(
	int evttype, 
	SPSR_module_cb fn_cb, 
	void *obj_cb, 
	SPSR_GENERIC_ST *evt, 
	int lendata)
{
	int ret = 0;
#ifdef SPSR_SHOW_CONSOLE
	spllog(0, "Enter call callback.");
#endif
	do {
		if (!fn_cb) {
			ret = SPSR_CALLBACK_NULL;
			break;
		}
		evt->type = evttype;
		evt->pc = sizeof(void *);

		if (sizeof(void *) == sizeof(SPSR_UNIT)) 
		{
			SPSR_UNIT *pt = (SPSR_UNIT *)evt->data;
			*pt = (SPSR_UNIT)obj_cb;
			spllog(SPL_LOG_DEBUG, "With 32 bit.");
		} 
		else if (sizeof(void *) == sizeof(SPSR_LLU)) 
		{
			SPSR_LLU *pt = (SPSR_LLU *)evt->data;
			spllog(SPL_LOG_DEBUG, "With 64 bit.");
			*pt = (SPSR_LLU)obj_cb;
		}
		evt->pl = evt->pc + lendata;
		evt->data[evt->pl] = 0;
		fn_cb(evt);
		evt->pl = evt->pc;
	} while (0);
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int spsr_resize_obj(
	int sz, SPSR_GENERIC_ST **obj) 
{
	int ret = 0;
	SPSR_GENERIC_ST *p = 0;
	int range = 0;
	int total = 0;
	int delta = 0;

	do {
		if(sz < (2 * sizeof(SPSR_GENERIC_ST))) 
		{
			ret = SPSR_MINI_SIZE;
			spllog(SPL_LOG_ERROR, 
				"SPSR_MINI_SIZE");			
			break;
		}
		if(!obj) {
			ret = SPSR_OBJ_NULL;
			spllog(SPL_LOG_ERROR, 
				"SPSR_OBJ_NULL");
			break;
		}
		p = *obj;
		if(p) {
			total = p->total;
			range = p->range;
			p = (SPSR_GENERIC_ST*)
				realloc(p, (sz + 1));
		} else {
			spsr_malloc(
				(sz + 1), p, 
				SPSR_GENERIC_ST);
		}
		
		if(!p) {
			ret = SPSR_MEM_NULL;
			spllog(SPL_LOG_ERROR, 
				"SPSR_MEM_NULL");			
			break;
		}
		delta = sz - total;
		p->total += delta;

		p->range += range ? delta 
			: (sz - sizeof(SPSR_GENERIC_ST));

		*obj = p;
	}
	while(0);

	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#ifndef UNIX_LINUX
#else
#endif

#ifndef __SPSR_EPOLL__
#else
#endif
