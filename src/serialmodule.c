/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
/* Email:
*		<nguyenthaithuanalg@gmail.com> - Nguyễn Thái Thuận
* Mobile:
*		<+084.799.324.179>
* Whatsapp:
*		<+084.799.324.179>
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
		<2025-May-13>
		<2025-May-20>
		<2025-Jul-01>		
* Decription:
*		The (only) main header file to export
		5 APIs: [spsr_module_init, spsr_module_finish, spsr_inst_open,
spsr_inst_close, spsr_inst_write].
*/
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

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
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/ioctl.h>
#ifndef __SPSR_EPOLL__
#include <poll.h>
#else
#include <sys/epoll.h>
#endif
/* https://gist.github.com/reterVision/8300781 */
#endif

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#ifndef UNIX_LINUX
#define SPSR_CloseHandle(__o0bj__)                                             \
	{                                                                      \
		void *__serialpp__ = (__o0bj__);                               \
		if (__serialpp__) {                                            \
			;                                                      \
			int bl = CloseHandle((__serialpp__));                  \
			;                                                      \
			if (!bl) {                                             \
				;                                              \
				spsr_err(                                      \
				    "CloseHandle error: %lu", GetLastError()); \
				;                                              \
			};                                                     \
			spsr_all("SPSR_CloseHandle 0x%p -->> %s",              \
			    __serialpp__, (bl ? "DONE" : "ERROR"));            \
			;                                                      \
			(__o0bj__) = 0;                                        \
			;                                                      \
		}                                                              \
	}
#else

#define SPSR_PORT_TRIGGER          (10024 + 1)
#define SPSR_PORT_CARTRIDGE        (SPSR_PORT_TRIGGER + 10)
#endif

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#define spsr_all(__fmt_____, ...)                                              \
	spllog(SPL_LOG_BASE, __fmt_____, ##__VA_ARGS__)
#define spsr_dbg(__fmt_____, ...)                                              \
	spllog(SPL_LOG_DEBUG, __fmt_____, ##__VA_ARGS__)
#define spsr_inf(__fmt_____, ...)                                              \
	spllog(SPL_LOG_INFO, __fmt_____, ##__VA_ARGS__)
#define spsr_wrn(__fmt_____, ...)                                              \
	spllog(SPL_LOG_WARNING, __fmt_____, ##__VA_ARGS__)
#define spsr_err(__fmt_____, ...)                                              \
	spllog(SPL_LOG_ERROR, __fmt_____, ##__VA_ARGS__)
#define spsr_ftl(__fmt_____, ...)                                              \
	spllog(SPL_LOG_FATAL, __fmt_____, ##__VA_ARGS__)
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#ifndef UNIX_LINUX
#define spsr_api_err(___api__)                                                 \
	{                                                                      \
		spsr_err("%s, dwError: %llu.", ___api__, (LLU)GetLastError()); \
	}
#else
#define spsr_api_err(___api__)                                                 \
	{                                                                      \
		spsr_err("%s, errno: %d: \"%s\".", ___api__, errno,            \
		    strerror(errno));                                          \
	}

#define SPSR_PXCLOSE(__xfd__, __xret__)                                        \
	do {                                                                   \
		if ((__xfd__) < 1) {                                           \
			spsr_err("Wrong pxfd: %d.", (__xfd__));                \
			break;                                                 \
		}                                                              \
		(__xret__) = close(__xfd__);                                   \
		if (__xret__) {                                                \
			spsr_api_err("close");                                 \
			break;                                                 \
		};                                                             \
		spsr_all("Close DONE, fd: %d", (__xfd__));                     \
	} while (0);
#endif
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

#define SPSR_MAX_AB(__a__, __b__) ((__a__) > (__b__)) ? (__a__) : (__b__)
#define SPSR_STEP_MEM              2048
#define SPSR_BUFFER_SIZE           2048
#define SPSR_DATA_RANGE            2048
#define SPSR_CMD_BUFF              2048

#define SPSR_EVT_CB_CART_LEN       \
	(SPSR_BUFFER_SIZE + sizeof(void *) + sizeof(SPSR_GENERIC_ST) + 1)
#define SPSR_CMD_BUFF_LEN          (SPSR_CMD_BUFF + sizeof(SPSR_GENERIC_ST) + 1)

#define SPSR_EVT_CB_PORT_LEN       \
	(SPSR_PORT_LEN + sizeof(void *) + sizeof(SPSR_GENERIC_ST) + 1)

#ifndef UNIX_LINUX

#define SPSR_THREAD_ROUTINE        LPTHREAD_START_ROUTINE

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
spsr_win32_write(SPSR_INFO_ST *p, SPSR_GENERIC_ST *buf, DWORD *bytesWrite,
    OVERLAPPED *olReadWrite, SPSR_GENERIC_ST *evt_cb_buff);
static int
spsr_win32_read(SPSR_INFO_ST *p, DWORD *bytesWrite, OVERLAPPED *olReadWrite,
    SPSR_GENERIC_ST *evt_cb_buf);
#else

typedef void *(*SPSR_THREAD_ROUTINE)(void *);
static int
spsr_init_trigger(void *);

static void 
set_rts_dtr(int fd, char rts_dtr);

static int
spsr_px_write(SPSR_GENERIC_ST *item, SPSR_GENERIC_ST *evt);

#ifndef __SPSR_EPOLL__
int
spsr_px_rem(SPSR_GENERIC_ST *item, SPSR_GENERIC_ST *evt, int *prange,
    struct pollfd *fds);
#else

int
spsr_px_rem(SPSR_GENERIC_ST *item, SPSR_GENERIC_ST *evt, int epollfd);
#endif

#ifndef __SPSR_EPOLL__
int
spsr_px_add(SPSR_GENERIC_ST *item, SPSR_GENERIC_ST *evt, int *prange,
    struct pollfd *fds);
#else

int
spsr_px_add(SPSR_GENERIC_ST *item, SPSR_GENERIC_ST *evt, int epollfd);
#endif

int
spsr_px_hash_add(SPSR_ARR_LIST_LINED *temp, SPSR_GENERIC_ST *evt);

#ifndef __SPSR_EPOLL__
#define SPSR_LOG_UNIX__SHARED_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define SPSR_LOG_UNIX_CREATE_MODE  (O_CREAT | O_RDWR | O_EXCL)
#define SPSR_LOG_UNIX_OPEN_MODE    (O_RDWR | O_EXCL)
#define SPSR_LOG_UNIX_PROT_FLAGS   (PROT_READ | PROT_WRITE | PROT_EXEC)

#define SPSR_SENDSK_FLAG           0

static int
spsr_fetch_commands(void *, int *, char *, int n, SPSR_GENERIC_ST *evt);

static int
spsr_ctrl_sock(void *fds, int *mx_number, int sockfd, SPSR_GENERIC_ST *evt,
    int *chk_off, SPSR_GENERIC_ST **pcart_buff);

static int
spsr_fmt_name(char *input, char *output, int);
#else
#define SPSR_SENDSK_FLAG           MSG_CONFIRM
static int
spsr_fetch_commands(int, char *, int n, SPSR_GENERIC_ST *evt);
static int
spsr_ctrl_sock(int epollfd, int sockfd, SPSR_GENERIC_ST *evt, int *chk_off,
    SPSR_GENERIC_ST **pcart_buff);

#endif

static void *
spsr_init_trigger_routine(void *);
static void *
spsr_init_cartridge_routine(void *);
static int
spsr_send_cmd(int cmd, char *portname, void *data, int lendata);

#define SPSR_MAX_NUMBER_OF_PORT    10

static void *spsr_hash_fd_arr[SPSR_MAX_NUMBER_OF_PORT];

/* static void *spsr_hash_name_arr[SPSR_MAX_NUMBER_OF_PORT]; */

typedef struct __SPSR_HASH_FD_NAME__ {
	int fd;
	int t_delay;
	char offDSR;
	char port_name[SPSR_PORT_LEN];
	SPSR_module_cb cb_evt_fn;
	void *cb_obj;
	struct __SPSR_HASH_FD_NAME__ *next;

} SPSR_HASH_FD_NAME;

#define SPSR_HASH_FD(__fd__) (__fd__ % SPSR_MAX_NUMBER_OF_PORT)

/* static int spsr_hash_port(char* port, int len); */

static int
spsr_clear_hash();
static int
spsr_open_fd(char *port_name, int brate, int *);

static int
spsr_px_read(int fd, SPSR_GENERIC_ST *pevt, char *chk_delay);

#endif

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

static SPSR_ROOT_TYPE spsr_root_node;

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
static int
spsr_resize_obj(int sz, SPSR_GENERIC_ST **obj);
static int
spsr_clear_all();
static int
spsr_verify_info(SPSR_INPUT_ST *obj);
static int
spsr_is_existed(char *port, int *);
static void
spsr_err_txt_init();
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
/* Group of sync tool. */
static void *
spsr_mutex_create();
static int
spsr_mutex_delete(void *);
static void *
spsr_sem_create(char *);
static int
spsr_sem_delete(void *, char *);
static int
spsr_mutex_lock(void *obj);
static int
spsr_mutex_unlock(void *obj);
static int
spsr_rel_sem(void *sem);
static int
spsr_wait_sem(void *sem);

static int
spsr_invoke_cb(int evttype, int err_code, SPSR_module_cb fn_cb, void *obj_cb,
    SPSR_GENERIC_ST *evt, int lendata);
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

int
spsr_inst_open(SPSR_INPUT_ST *p)
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
		spsr_err("spsr_verify_info: %d.", ret);
	}
	return ret;
}

int
spsr_inst_close(char *portname)
{
	int ret = 0;
	spsr_all("Delete port: %s.", portname);
#ifndef UNIX_LINUX
	void *p = 0;
	SPSR_ARR_LIST_LINED *node = 0;
	do {
		ret = spsr_get_obj(portname, &p, 1);
		if (p) {
			spsr_all("Delete port: %s.", portname);
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
		if (ret) {
			break;
		}
		if (!isExisted) {
			spsr_err("port %s not exsied.", portname);
			ret = SPSR_PORTNAME_NONEXISTED;
			break;
		}
		ret = spsr_send_cmd(SPSR_CMD_REM, portname, 0, 0);
	} while (0);
	spsr_mutex_unlock(t->mutex);
#endif
	return ret;
}

int
spsr_module_openport(void *obj)
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
		if (p->handle) {
			SPSR_CloseHandle(p->handle);
		}
		/*Open the serial port with FILE_FLAG_OVERLAPPED for
		 * asynchronous operation*/
		hSerial = CreateFile(p->port_name, GENERIC_READ | GENERIC_WRITE,
		    0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);

		if (hSerial == INVALID_HANDLE_VALUE) {
			/*
			DWORD dwError = GetLastError();
			spsr_err(
				"Open port errcode: %lu", dwError);
			*/
			spsr_api_err("CreateFile");
			ret = SPSR_PORT_OPEN;
			hSerial = 0;
			break;
		}
		p->handle = hSerial;
		spsr_all("Create hSerial: 0x%p.", hSerial);
		/*
		if (p->is_retry) {
			break;
		}
		*/
		/* Set up the serial port parameters(baud rate, etc.) */
		dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
		if (!GetCommState(hSerial, &dcbSerialParams)) {
			/*
			DWORD dwError = GetLastError();
			spsr_err(
				"GetCommState: %lu", dwError);
			*/
			spsr_api_err("GetCommState");
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
#if 1
		dcbSerialParams.fOutxDsrFlow = p->offDSR ? 0 : 1;
#else
		dcbSerialParams.fOutxDsrFlow = FALSE;
#endif
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
			/*
			DWORD dwError = GetLastError();
			spsr_err(
				"SetCommState: %lu", dwError);
			*/
			spsr_api_err("SetCommState");
			ret = SPSR_PORT_SETCOMMSTATE;
			break;
		}
		if (!p->hEvent) {
			p->hEvent = CreateEvent(0, TRUE, FALSE, 0);
		}
		if (!p->hEvent) {
			/*
			DWORD dwError = GetLastError();
			spsr_err(
				"CreateEvent: %lu", dwError);
			*/
			spsr_api_err("CreateEvent");
			ret = SPSR_PORT_CREATEEVENT;
			break;
		}

		spsr_dbg("Create hEvent: 0x%p.", p->hEvent);

		/* Set timeouts(e.g., read timeout of 500ms, write timeout of
		   500ms)
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
			/*
			DWORD dwError = GetLastError();
			spsr_err(
				"SetCommTimeouts: %lu", dwError);
			*/
			spsr_api_err("SetCommTimeouts");
			ret = SPSR_PORT_SETCOMMTIMEOUTS;
			break;
		}
		if (!p->mtx_off) {
			p->mtx_off = spsr_mutex_create();
		}
		if (!p->mtx_off) {
			DWORD dwError = GetLastError();
			spsr_err("spsr_mutex_create: %lu", dwError);
			ret = SPSR_PORT_SPSR_MUTEX_CREATE;
			break;
		}
		if (!p->sem_off) {
			p->sem_off = spsr_sem_create(0);
		}
		if (!p->sem_off) {
			DWORD dwError = GetLastError();
			spsr_err("spsr_sem_create: %lu", dwError);
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
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
void *
spsr_sem_create(char *name_key)
{
	void *obj = 0;
	do {
#ifndef UNIX_LINUX
		obj = CreateSemaphoreA(0, 0, 1, 0);
		if (!obj) {
			spsr_api_err("CreateSemaphoreA");
			break;
		}
#else
#ifndef __SPSR_EPOLL__
		int retry = 0;
		char name[SPSR_KEY_LEN * 2];

		spsr_fmt_name(name_key, name, SPSR_KEY_LEN * 2);
		do {
			obj = sem_open(name, SPSR_LOG_UNIX_CREATE_MODE,
			    SPSR_LOG_UNIX__SHARED_MODE, 1);
			spsr_all("sem_open ret:"
				 " 0x%p, name: %s",
			    obj, name);
			if (obj == SEM_FAILED) {
				int err = 0;
				obj = 0;
				if (retry) {
					spsr_api_err("sem_open");
					break;
				} else {
					spsr_api_err("sem_open");
				}
				err = sem_unlink(name);
				if (err) {
					spsr_api_err("sem_unlink");
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
			spsr_api_err("malloc");
			break;
		}
		memset(obj, 0, sizeof(sem_t));
		sem_init((sem_t *)obj, 0, 1);
#endif
#endif
	} while (0);
	spsr_all("Create: 0x%p.", obj);
	return obj;
}

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

void *
spsr_mutex_create()
{
	void *obj = 0;
	do {
#ifndef UNIX_LINUX
		obj = CreateMutexA(0, 0, 0);
		if (!obj) {
			spsr_api_err("CreateMutexA");
			break;
		}
#else
		/*https://linux.die.net/man/3/pthread_mutex_init*/
		spsr_malloc(sizeof(pthread_mutex_t), obj, void);
		if (!obj) {
			spsr_api_err("malloc");
			break;
		}
		memset(obj, 0, sizeof(pthread_mutex_t));
		pthread_mutex_init((pthread_mutex_t *)obj, 0);
#endif
	} while (0);
	spsr_all("Create: 0x%p.", obj);
	return obj;
}

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

int
spsr_module_isoff(SPSR_INFO_ST *obj)
{
	int rs = 0;
	spsr_mutex_lock(obj->mtx_off);
	rs = obj->isoff;
	spsr_mutex_unlock(obj->mtx_off);
	return rs;
}

#ifndef UNIX_LINUX
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int
spsr_win32_read(SPSR_INFO_ST *p, DWORD *pbytesRead, OVERLAPPED *polReadWrite,
    SPSR_GENERIC_ST *ecb_buf)
{
	char *tbuffer = 0;
	int ret = 0;
	BOOL rs = FALSE;
	COMSTAT csta = {0};
	DWORD dwError = 0;
	DWORD readErr = 0;
	int len = 0;
	do {
		tbuffer = ecb_buf->data + sizeof(void *);
		rs = ReadFile(p->handle, tbuffer, SPSR_BUFFER_SIZE, pbytesRead,
		    polReadWrite);

		spsr_dbg("olRead.InternalHigh: %d, "
			 "olRead.Internal: %d, rs : %s!!!",
		    (int)polReadWrite->InternalHigh,
		    (int)polReadWrite->Internal, rs ? "true" : "false");

		if (rs) {
			spsr_inf("Read OK");
			break;
		}
		readErr = GetLastError();
		if (readErr != ERROR_IO_PENDING) {
			if (ERROR_TOO_MANY_POSTS != readErr) {
				ret = SPSR_WIN32_NOT_PENDING;
				spsr_err(
				    "Read error readErr: %d", (int)readErr);
				break;
			}
			WaitForSingleObject(p->hEvent, INFINITE);
		}
		*pbytesRead = 0;
		if (p->t_delay > 0) {
			Sleep(p->t_delay);
		}
		WaitForSingleObject(p->hEvent, INFINITE);

		rs =
		    GetOverlappedResult(p->handle, polReadWrite, pbytesRead, 1);

		if (!rs) {
			spsr_err("PurgeComm: %d", (int)GetLastError());
			PurgeComm(p->handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
			ret = SPSR_WIN32_OVERLAP_ERR;
			break;
		}
		spsr_dbg("bRead: %d", (int)*pbytesRead);
		memset(&csta, 0, sizeof(csta));
		rs = ClearCommError(p->handle, &dwError, &csta);
		if (!rs) {
			spsr_api_err("ClearCommError");
			ret = SPSR_WIN32_CLEARCOMM;
			break;
		}
		if (csta.cbInQue > 0) {
			spsr_err("Read Com not finished!!!");
			ret = SPSR_WIN32_STILL_INQUE;
			break;
		} else if (*pbytesRead > 0) {
			len = *pbytesRead;
			tbuffer[*pbytesRead] = 0;
			spsr_all("[tbuffer: %s]!", tbuffer);
#if 0
			spsr_invoke_cb(SPSR_EVENT_READ_BUF, 0,
				p->cb_evt_fn, p->cb_obj,
			    ecb_buf, *pbytesRead);
#endif
		}
	} while (0);
	do {
		int evtcode = 0;
		int err = 0;
		if (!p) {
			break;
		}
		if (!ecb_buf) {
			break;
		}
		if (!pbytesRead) {
			break;
		}
		if (!p->cb_evt_fn) {
			break;
		}
		if (ret) {
			const char *text = 0;
			text = spsr_err_txt(ret);
			snprintf(tbuffer, ecb_buf->range, "%s|%s", text,
			    p->port_name);

			len = strlen(tbuffer);
		}
		evtcode = ret ? SPSR_EVENT_READ_ERROR : SPSR_EVENT_READ_BUF;
		err = spsr_invoke_cb(
		    evtcode, ret, p->cb_evt_fn, p->cb_obj, ecb_buf, len);
		spsr_all("spsr_invoke_cb spsr_win32_read");
	} while (0);
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
static int
spsr_win32_connected(HANDLE hd)
{
	int ret = 0;
	DWORD status = 0;
	BOOL bl = FALSE;
	do {
		bl = GetCommModemStatus(hd, &status);
		if (!bl) {
			spsr_api_err("GetCommModemStatus");
			break;
		}
		if (!(status & MS_DSR_ON)) {
			spsr_wrn("DSR is NOT set.");
			break;
		}
		spsr_dbg("DSR is set.");
		ret = 1;
	} while (0);
	return ret;
}

int
spsr_win32_write(SPSR_INFO_ST *p, SPSR_GENERIC_ST *buf, DWORD *pbytesWrite,
    OVERLAPPED *polReadWrite, SPSR_GENERIC_ST *ecb_buf)
{
	int ret = 0;
	BOOL wrs = FALSE;
	char *tbuffer = 0;
	int evtcode = SPSR_EVENT_WRITE_ERROR;
	int portlen = 0;
	DWORD wErr = 0;
	DWORD dwWaitResult = 0;
	int wroteRes = 0;
	BOOL rsOverlap = TRUE;
	int connected = 0;

	do {
		if (!p) {
			ret = SPSR_WIN32_OBJ_NULL;
			spsr_err("SPSR_WIN32_OBJ_NULL");
			break;
		}
		if (!buf) {
			ret = SPSR_WIN32_BUF_NULL;
			spsr_err("SPSR_WIN32_BUF_NULL");
			break;
		}
		if (buf->pl < 1) {
			ret = SPSR_WIN32_BUF_NULL;
			spsr_err("SPSR_WIN32_BUF_NULL");
			break;
		}
		if (!pbytesWrite) {
			ret = SPSR_WIN32_BWRITE_NULL;
			spsr_err("SPSR_WIN32_BWRITE_NULL");
			break;
		}
		if (!polReadWrite) {
			ret = SPSR_WIN32_OVERLAP_NULL;
			spsr_err("SPSR_WIN32_OVERLAP_NULL");
			break;
		}
		if (!ecb_buf) {
			ret = SPSR_WIN32_EVTCB_NULL;
			break;
		}
		tbuffer = ecb_buf->data + sizeof(void *);
		connected = p->offDSR ? 1 : spsr_win32_connected(p->handle);
#if 0
		connected = spsr_win32_connected(p->handle);
#endif
		while (buf->pl > 0) {
			if (!connected) {
				spsr_err("Remote unconnected! "
					 "Turn on offDSR for half-duplex!."
					 "Turn off offDSR for full-duplex!."
					 "Port [%s].",
				    p->port_name);
				ret = SPSR_WIN32_UNCONNECTED;
				break;
			}
			*pbytesWrite = 0;
			memset(polReadWrite, 0, sizeof(OVERLAPPED));
			polReadWrite->hEvent = p->hEvent;

			wrs = WriteFile(p->handle, buf->data, buf->pl,
			    pbytesWrite, polReadWrite);
			
			if (wrs) {
				if (buf->pl == (int)(*pbytesWrite)) {
					spsr_inf("Write DONE, %d, data: %s.", 
						buf->pl, buf->data);
					wroteRes = 1;
					buf->pl = 0;
				} else {
					spsr_api_err("WriteFile");
					ret = SPSR_WIN32_BYTEWRITE;
				}
				break;
			}

			wErr = GetLastError();
			spsr_dbg("WriteFile: %d", (int)wErr);
			if (wErr != ERROR_IO_PENDING) {
				spsr_err("!IO_PENDING, %d.", (int)wErr);
				ret = SPSR_WIN32_NOTPENDING;
				break;
			}

			dwWaitResult = WaitForSingleObject(p->hEvent, INFINITE);
			if (dwWaitResult != WAIT_OBJECT_0) {
				spsr_api_err("WaitForSingleObject.");
				ret = SPSR_WIN32_WOBJ;
				break;
			}
			*pbytesWrite = 0;

			rsOverlap = GetOverlappedResult(
			    p->handle, polReadWrite, pbytesWrite, TRUE);

			if (!rsOverlap) {
				spsr_api_err("GetOverlappedResult.");
				ret = SPSR_WIN32_GETOVERLAP;
				break;
			}
			if (buf->pl != (int)(*pbytesWrite)) {
				spsr_err("Write Error, %d<>%d.", buf->pl,
				    (int)(*pbytesWrite));
				ret = SPSR_WIN32_BYTEWRITE;
				break;
			}
			spsr_dbg("Write DONE, %d.", buf->pl);
			wroteRes = 1;
			buf->pl = 0;
			break;
		}

		buf->pl = 0;

		evtcode =
		    wroteRes ? SPSR_EVENT_WRITE_OK : SPSR_EVENT_WRITE_ERROR;

		portlen = (int)strlen(p->port_name);
		tbuffer[portlen] = 0;

		memcpy(tbuffer, p->port_name, portlen);
#if 0
		spsr_invoke_cb( evtcode, ret, 
			p->cb_evt_fn, p->cb_obj, ecb_buf, portlen);
#endif
	} while (0);
	do {
		if (!p) {
			break;
		}
		if (!ecb_buf) {
			break;
		}
		if (!p->cb_evt_fn) {
			break;
		}
		spsr_invoke_cb(
		    evtcode, ret, p->cb_evt_fn, p->cb_obj, ecb_buf, portlen);
	} while (0);
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
DWORD WINAPI
spsr_thread_operating_routine(LPVOID arg)
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
		ret = 0;

		flags = EV_RXCHAR | EV_BREAK | EV_RXFLAG | EV_DSR;

		if (!buf) {
			spsr_err("buf NULL");
			break;
		}

		if (isoff) {
			spsr_all("is OFF");
			break;
		}

		SPSR_CloseHandle(p->handle);

		isoff = spsr_module_isoff(p);
		if (p->is_retry) {
			spsr_all("retry");
		}
		if (!p->is_retry) {
			ret = spsr_module_openport(p);
		}
		portlen = (int)strlen(p->port_name);
		do {
			int eventcb = 0;
			if (p->is_retry) {
				break;
			}
			memcpy(tbuffer, p->port_name, portlen);

			tbuffer[portlen] = 0;

			eventcb = ret ? SPSR_EVENT_OPEN_DEVICE_ERROR
				      : SPSR_EVENT_OPEN_DEVICE_OK;
			spsr_invoke_cb(eventcb, ret, p->cb_evt_fn, p->cb_obj,
			    ecb_buf, portlen);
		} while (0);

		if (ret) {
			Sleep(1000 * 2);
			spsr_err("In r/w loop.");
			continue;
			;
		}

		while (1) {
			isoff = spsr_module_isoff(p);
			if (isoff) {
				spsr_all("is OFF");
				break;
			}
			memset(&olReadWrite, 0, sizeof(olReadWrite));
			olReadWrite.hEvent = p->hEvent;
			wrs = TRUE;
			spsr_mutex_lock(p->mtx_off);
			do {
				if (!p->buff) {
					spsr_all("No data.");
					break;
				}

				if (p->buff->pl < 1) {
					spsr_all("No data.");
					break;
				}

				spsr_all("(pl, range): (%d, %d)", p->buff->pl,
				    buf->range);

				if (p->buff->pl > buf->range) {
					int rz = 0;
					rz = SPL_MAX_AB(
					    p->buff->pl, SPSR_STEP_MEM);
					ret = spsr_resize_obj(rz, &buf);
				}
				buf->pl = p->buff->pl;
				memcpy(buf->data, p->buff->data, buf->pl);
				p->buff->pl = 0;

			} while (0);
			spsr_mutex_unlock(p->mtx_off);

			if (buf->pl > 0) {
				ret = spsr_win32_write(
				    p, buf, &bytesWrite, &olReadWrite, ecb_buf);
				if (ret) {
					spsr_err("spsr_win32_write");
					p->is_retry++;
					if (p->is_retry > 3) {
						break;
					}
					/*
					SPSR_CloseHandle(p->handle);
					break;
					*/
				}
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
			} else {
				spsr_dbg("WaitCommEvent OK");
			}
			memset(&csta, 0, sizeof(csta));
			ClearCommError(p->handle, &dwError, &csta);
			cbInQue = csta.cbInQue;
			if (!cbInQue) {
				BOOL rsOverlap = TRUE;
				WaitForSingleObject(p->hEvent, INFINITE);

				rsOverlap = GetOverlappedResult(
				    p->handle, &olReadWrite, &bytesRead, TRUE);
				if (rsOverlap) {
					continue;
				}
				PurgeComm(
				    p->handle, PURGE_RXCLEAR | PURGE_TXCLEAR);

				continue;
			}

			bytesRead = 0;
			ret = spsr_win32_read(
			    p, &bytesRead, &olReadWrite, ecb_buf);
			if (ret) {
				spsr_err("spsr_win32_read");
			}
			spsr_dbg(" [[[ cbInQue: %d, bRead: %d ]]]", cbInQue,
			    bytesRead);
			bytesRead = 0;
		}

		p->is_retry++;
		if (p->is_retry > 3) {
			p->is_retry = 0;
		}
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
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int
spsr_module_init()
{
	int ret = 0;
	SPSR_ROOT_TYPE *t = &spsr_root_node;
	spsr_err_txt_init();
#ifndef UNIX_LINUX
#else
	pthread_t idd = 0;
	int err = 0;
	int nsize = 0;
#ifndef __SPSR_EPOLL__
	snprintf(t->sem_key, SPSR_KEY_LEN, "/spsr_%llu", (LLU)getpid());
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

		err = pthread_create(&idd, 0, spsr_init_trigger_routine, t);
		if (err) {
			ret = SPSR_CREATE_THREAD_ERROR;
			break;
		}

		idd = 0;
		err = pthread_create(&idd, 0, spsr_init_cartridge_routine, t);
		if (err) {
			ret = SPSR_CREATE_THREAD_ERROR;
			break;
		}

		nsize = SPSR_BUFFER_SIZE + sizeof(int);
		spsr_malloc(nsize, t->cmd_buff, SPSR_GENERIC_ST);
		if (!t->cmd_buff) {
			spsr_err("spsr_malloc error");
			exit(1);
		}
		t->cmd_buff->total = nsize;
		t->cmd_buff->range = SPSR_BUFFER_SIZE;
#endif
		spsr_dbg("spsr_module_init: DONE");

	} while (0);
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
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
			spsr_sem_delete(t->sem_spsr, SPSR_MAINKEY_MACH);
#else
			spsr_sem_delete(t->sem_spsr, 0);
#endif
			break;
		}
	}

#ifndef __SPSR_EPOLL__
	spsr_sem_delete(t->sem, SPSR_MAINKEY);
#else
	spsr_sem_delete(t->sem, 0);
#endif
#endif
	spsr_mutex_delete(t->mutex);

	return 0;
}

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int
spsr_mutex_lock(void *obj)
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
			spsr_err("WaitForSingleObject errcode: %lu", dwError);
			ret = SPSR_WIN32_LK_MTX;
			;
			break;
		}
#else
		ret = pthread_mutex_lock((pthread_mutex_t *)obj);
		if (ret) {
			spsr_err("pthread_mutex_lock: ret: %d, "
				 "errno: %d, text: %s, obj: 0x%p.",
			    ret, errno, strerror(errno), obj);
			ret = SPSR_PX_LK_MTX;
		}
#endif
	} while (0);
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int
spsr_mutex_unlock(void *obj)
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
			spsr_err("ReleaseMutex errcode: %lu", dwError);
			ret = SPSR_WIN32_RL_MTX;
			break;
		}
#else
		ret = pthread_mutex_unlock((pthread_mutex_t *)obj);
		if (ret) {
			spsr_err("pthread_mutex_unlock: ret: %d, "
				 "errno: %d, text: %s.",
			    ret, errno, strerror(errno));
			ret = SPSR_PX_RL_MTX;
		}
#endif
	} while (0);
	return ret;
}

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

int
spsr_rel_sem(void *sem)
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
			spsr_err("ReleaseSemaphore errcode: %lu", dwError);
			ret = SPSR_WIN32_RL_SEM;
		}
#else

		err = sem_post((sem_t *)sem);
		if (err) {
			spsr_err("sem_post: err: %d, "
				 "errno: %d, text: %s, sem: 0x%p.",
			    err, errno, strerror(errno), sem);
			ret = SPSR_PX_RL_SEM;
		}
#endif
	} while (0);
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
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
		iswell = WaitForSingleObject((HANDLE)sem, INFINITE);
		if (iswell != WAIT_OBJECT_0) {
			DWORD dwError = GetLastError();
			spsr_err("WaitForSingleObject errcode: %lu", dwError);
			ret = SPSR_WIN32_WAIT_SEM;
		}
#else
		err = sem_wait((sem_t *)sem);
		if (err) {
			spsr_err("sem_post: err: %d, "
				 "errno: %d, text: %s, sem: 0x%p.",
			    err, errno, strerror(errno), sem);
			ret = SPSR_PX_WAIT_SEM;
		}
#endif
	} while (0);
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#ifndef UNIX_LINUX
int
spsr_get_obj(char *portname, void **obj, int takeoff)
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
			if (strcmp(portname, node->item->port_name) == 0) {
				*obj = node;
				found = 1;
				if (!takeoff) {
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
		spsr_wrn("Cannot find port: %s", portname);
		ret = SPSR_ITEM_NOT_FOUND;
	}

	return ret;
}
#else
#endif

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

#ifndef UNIX_LINUX
int
spsr_create_thread(SPSR_THREAD_ROUTINE f, void *arg)
{
	int ret = 0;
	DWORD dwThreadId = 0;
	HANDLE hThread = 0;
	hThread = CreateThread(NULL, 0, f, arg, 0, &dwThreadId);
	if (!hThread) {
		ret = SPSR_THREAD_W32_CREATE;
		spsr_dbg("CreateThread error: %d", (int)GetLastError());
	}
	return ret;
}
#else
#endif
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
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

			eventcb = node->item->handle
				      ? SPSR_EVENT_CLOSE_DEVICE_ERROR
				      : SPSR_EVENT_CLOSE_DEVICE_OK;
			l = (int)strlen(node->item->port_name);

			memcpy(tbuffer, node->item->port_name, l);
			if (eventcb) {
				ret = SPSR_WIN32_FD_CLOSED;
			}
			spsr_invoke_cb(eventcb, ret, cb_fn, cb_obj, ecb_buf, l);
		}
		spsr_free(node->item->buff);

	} while (0);
	return ret;
}
#endif
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int
spsr_inst_write(char *portname, char *data, int sz)
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
			spsr_err("Cannot find the object.");
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

			if (!item->buff) {
				int step = 0;
				step = SPSR_MAX_AB(sz, SPSR_STEP_MEM);
				step += sizeof(SPSR_GENERIC_ST);
				ret = spsr_resize_obj(step, &(item->buff));

				if (ret) {
					spsr_err("spsr_resize_obj.");
					break;
				}
				tmp = item->buff->data;
				memcpy(tmp, data, sz);
				item->buff->pl = sz;

				break;
			}
			range = item->buff->range;
			pl = item->buff->pl;
			if (range > pl + sz) {
				tmp = item->buff->data;
				tmp += item->buff->pl;
				memcpy(tmp, data, sz);
				item->buff->pl += sz;
				break;
			}

			total = item->buff->total;
			total += SPSR_MAX_AB(sz, SPSR_STEP_MEM);
			ret = spsr_resize_obj(total, &(item->buff));
			if (ret) {
				spsr_err("spsr_resize_obj.");
				break;
			}
			tmp = item->buff->data;
			tmp += item->buff->pl;
			memcpy(tmp, data, sz);
			item->buff->pl += sz;

			break;

		} while (0);
		spsr_mutex_unlock(item->mtx_off);
		if (ret) {
			break;
		}
		SetEvent(item->hEvent);
#else
		SPSR_ROOT_TYPE *t = &spsr_root_node;
		int isExisted = 0;
		spsr_mutex_lock(t->mutex);
		do {
			ret = spsr_is_existed(portname, &isExisted);
			if (ret) {
				break;
			}
			if (!isExisted) {
				spsr_err("port %s not exsied.", portname);
				ret = SPSR_PORTNAME_NONEXISTED;
				break;
			}
			ret = spsr_send_cmd(SPSR_CMD_WRITE, portname, data, sz);
		} while (0);
		spsr_mutex_unlock(t->mutex);
		if (ret) {
			spsr_err("SEND command error ret: %d.", ret);
		}
#endif
	} while (0);
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#ifndef UNIX_LINUX
#else
#define SPSR_SIZE_CARTRIDGE        10
#define SPSR_SIZE_TRIGGER          2
#define SPSR_SIZE_MAX_EVENTS       10
/*#define SPSR_MSG_OFF               "SPSR_MSG_OFF"*/
#define SPSR_MILLION               1000000
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
void *
spsr_init_trigger_routine(void *obj)
{
	spsr_init_trigger(obj);
	return 0;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

void *
spsr_init_cartridge_routine(void *obj)
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
	struct epoll_event event, events[SPSR_SIZE_MAX_EVENTS];
#endif
	spsr_dbg("cartridge: ");
	/* Creating socket file descriptor */
	spsr_malloc(SPSR_CMD_BUFF_LEN, cart_buff, SPSR_GENERIC_ST);
	do {
		if (!cart_buff) {
			ret = SPSR_MEM_NULL;
			spsr_err("SPSR_MEM_NULL.");
			break;
		}
		cart_buff->total = SPSR_CMD_BUFF_LEN;
		cart_buff->range = SPSR_CMD_BUFF;

		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if (sockfd < 0) {
			/*
			spsr_err("fcntl: ret: %d, errno: %d, "
				 "text: %s.",
			    sockfd, errno, strerror(errno));
			*/
			spsr_api_err("socket");
			ret = SPSR_CREATE_SOCK;
			break;
		}

		memset(&cartridge_addr, 0, sizeof(cartridge_addr));

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
			/*
			spsr_err("fcntl: ret: %d, "
				 "errno: %d, text: %s.",
			    ret, errno, strerror(errno));
			*/
			spsr_api_err("fcntl");
			ret = SPSR_FCNTL_SOCK;
			break;
		}
		err = fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
		if (err == -1) {
			/*
			spsr_err("fcntl: err: %d, errno: %d, "
				 "text: %s.",
			    err, errno, strerror(errno));
			*/
			spsr_api_err("fcntl");
			ret = SPSR_FCNTL_SOCK;
			break;
		}

		/* Bind the socket with the server address */
		err = bind(sockfd, (const struct sockaddr *)&cartridge_addr,
		    sizeof(cartridge_addr));
		if (err < 0) {
			spsr_err("bind failed: err: %d, errno: %d, "
				 "text: %s.",
			    err, errno, strerror(errno));
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
				spsr_all("isoff");
				break;
			}
			/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

#ifndef __SPSR_EPOLL__

			fds[0].fd = sockfd;
			fds[0].events = POLLIN;
			mx_number = 1;
			while (1) {
				if (isoff) {
					spsr_all("isoff");
					break;
				}
				chk_delay = 0;

				err = poll(fds, mx_number, 60 * 1000);

				spsr_dbg("poll,  mx_number: %d", mx_number);

				if (err == -1) {
					spsr_wrn("poll");
					continue;
				}
				if (err == 0) {
					spsr_wrn("poll");
					continue;
				}
				spsr_mutex_lock(t->mutex);
				/*do {*/
				isoff = t->spsr_off;
				/*} while (0);*/
				spsr_mutex_unlock(t->mutex);
				if (isoff) {
					spsr_all("isoff");
					break;
				}
				for (k = 0; k < mx_number; ++k) {
					if (fds[k].fd < 0) {
						continue;
					}
					if (!(fds[k].revents & POLLIN)) {
						continue;
					}

					if (k == 0) {
						spsr_ctrl_sock(fds, &mx_number,
						    sockfd, ecb_buf, &isoff,
						    &cart_buff);

						continue;
					}
					if (fds[k].fd >= 0) {
						spsr_px_read(fds[k].fd, ecb_buf,
						    &chk_delay);
					}
				}
			}
#else
			/* Start epoll */
			epollfd = epoll_create1(0);
			if (epollfd < 0) {
				spsr_err("epoll_create, epollfd: %d, "
					 "errno: %d, text: %s.",
				    epollfd, errno, strerror(errno));
				ret = SPSR_EPOLL_CREATE;
				break;
			}
			event.events = EPOLLIN | EPOLLET;
			event.data.fd = sockfd;

			err = epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event);
			if (err < 0) {
				spllog(SPL_LOG_ERROR,
				    "epoll_ctl, err: %d, errno: %d, "
				    "text: %s.",
				    err, errno, strerror(errno));
				ret = SPSR_EPOLL_CTL;
				break;
			}
			while (1) {
				int nfds = 0;
				if (isoff) {
					spsr_all("isoff");
					break;
				}
				chk_delay = 0;

				nfds = epoll_wait(
				    epollfd, events, SPSR_SIZE_MAX_EVENTS, -1);

				spsr_mutex_lock(t->mutex);
				/*do {*/
				isoff = t->spsr_off;
				/*} while (0);*/
				spsr_mutex_unlock(t->mutex);

				if (isoff) {
					spsr_all("isoff");
					break;
				}

				for (i = 0; i < nfds; i++) {
					if (events[i].data.fd == sockfd) {
						spsr_ctrl_sock(epollfd, sockfd,
						    ecb_buf, &isoff,
						    &cart_buff);

						continue;
					}
					if (events[i].data.fd >= 0) {
						spsr_px_read(events[i].data.fd,
						    ecb_buf, &chk_delay);
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
	spsr_clear_all();
	/*} while (0);*/
	spsr_mutex_unlock(t->mutex);
	spsr_clear_hash();
	if (sockfd > 0) {
		SPSR_PXCLOSE(sockfd, err);
		if (err) {
			ret = SPSR_CLOSE_SOCK;
		}
	}

	if (ret) {
		spsr_err("ret: %d", ret);
	}
	spsr_free(cart_buff);

	spsr_mutex_lock(t->mutex);
	/*do { */
	t->spsr_off++;
	spsr_rel_sem(t->sem_spsr);
	/*} while(0); */
	spsr_mutex_unlock(t->mutex);

	return 0;
}

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

int
spsr_init_trigger(void *obj)
{
	SPSR_ROOT_TYPE *t = &spsr_root_node;
	int ret = 0;
	int sockfd = 0;
	int isoff = 0;
	int flags = 0;
	socklen_t len = 0;
	struct sockaddr_in trigger_addr, cartridge_addr;
	spsr_dbg("trigger: ");
	char had_cmd = 0;
	do {
		/* Creating socket file descriptor */
		if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			spsr_dbg("fcntl: ret: %d, errno: %d, "
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
			spsr_dbg("fcntl: ret: %d, "
				 "errno: %d, text: %s.",
			    ret, errno, strerror(errno));
			ret = SPSR_FCNTL_SOCK;
			break;
		}

		ret = fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
		if (ret == -1) {
			spsr_dbg("fcntl: ret: %d, "
				 "errno: %d, text: %s.",
			    ret, errno, strerror(errno));
			ret = SPSR_FCNTL_SOCK;
			break;
		}

		/* Bind the socket with the server address */
		ret = bind(sockfd, (const struct sockaddr *)&trigger_addr,
		    sizeof(trigger_addr));
		if (ret < 0) {
			/* perror("bind failed"); */
			spsr_err("bind failed: ret: %d, "
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
			if (t->cmd_buff) {
				if (t->cmd_buff->pl > 0) {
					had_cmd = 1;
				}
			}
			/*} while (0);*/
			spsr_mutex_unlock(t->mutex);

			if (isoff) {
				char c = 1;
				int didsent = sendto(sockfd, &c, 1,
				    SPSR_SENDSK_FLAG,
				    (const struct sockaddr *)&cartridge_addr,
				    len);

				spsr_dbg("didsent : isoff, %d", didsent);
				break;
			}
			if (had_cmd) {
				char c = 0;
				int didsent = sendto(sockfd, &c, 1,
				    SPSR_SENDSK_FLAG,
				    (const struct sockaddr *)&cartridge_addr,
				    len);
				spsr_dbg("didsent: %d", didsent);
			}
			had_cmd = 0;
			/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
		}
		SPSR_PXCLOSE(sockfd, ret);
		if (ret) {
			ret = SPSR_CLOSE_SOCK;
		}

	} while (0);
	spsr_mutex_lock(t->mutex);
	/*do {*/
	t->spsr_off++;
	spsr_rel_sem(t->sem_spsr);
	/*} while(0); */
	spsr_mutex_unlock(t->mutex);
	return 0;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

#ifndef __SPSR_EPOLL__
int
spsr_fetch_commands(
    void *mp, int *prange, char *info, int n, SPSR_GENERIC_ST *evt)
#else
int
spsr_fetch_commands(int epollfd, char *info, int n, SPSR_GENERIC_ST *evt)
#endif
{
	int ret = 0;
	SPSR_GENERIC_ST *item = 0;

#ifndef __SPSR_EPOLL__
	struct pollfd *fds = (struct pollfd *)mp;
#else
#endif
	int step = 0;
	spsr_all("Enter fetching command, n: %d", n);

	for (step = 0; step < n;) {
		item = (SPSR_GENERIC_ST *)(info + step);
		step += item->total;

		if (item->type == SPSR_CMD_ADD) {
			spsr_all("\t SPSR_CMD_ADD: %d", item->type);
#ifndef __SPSR_EPOLL__
			ret = spsr_px_add(item, evt, prange, fds);
#else
			ret = spsr_px_add(item, evt, epollfd);
#endif
			continue;
		}
		/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
		if (item->type == SPSR_CMD_REM) {
			spsr_all("\t SPSR_CMD_REM: %d", item->type);
#ifndef __SPSR_EPOLL__
			ret = spsr_px_rem(item, evt, prange, fds);
#else
			ret = spsr_px_rem(item, evt, epollfd);
#endif
			continue;
		}
		/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
		if (item->type == SPSR_CMD_WRITE) {
			spsr_all("\t SPSR_CMD_WRITE: %d", item->type);
			ret = spsr_px_write(item, evt);
			continue;
		}
		if (ret) {
			break;
		}
	}

	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
/*SPSR_CMD_ADD*/
int
spsr_px_hash_add(SPSR_ARR_LIST_LINED *temp, SPSR_GENERIC_ST *evt)
{
	int ret = 0;
	int fd = -1;
	do {
		SPSR_HASH_FD_NAME *hashobj = 0, *hashitem = 0;
		int hashid = 0;
		fd = temp->item->handle;
		spsr_malloc(
		    sizeof(SPSR_HASH_FD_NAME), hashobj, SPSR_HASH_FD_NAME);
		if (!hashobj) {
			ret = SPSR_MALLOC_ERROR;
			spsr_err("hashobj null");
			break;
		}
		hashobj->fd = fd;
		memcpy(hashobj->port_name, temp->item->port_name,
		    strlen(temp->item->port_name));
		hashobj->cb_evt_fn = temp->item->cb_evt_fn;
		hashobj->cb_obj = temp->item->cb_obj;
		hashobj->t_delay = temp->item->t_delay;
		hashobj->offDSR = temp->item->offDSR;

		hashid = SPSR_HASH_FD(fd);
		hashitem = (SPSR_HASH_FD_NAME *)spsr_hash_fd_arr[hashid];
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
			char *tbuff = evt->data + sizeof(void *);
			memcpy(tbuff, temp->item->port_name,
			    strlen(temp->item->port_name));
			spsr_invoke_cb(SPSR_EVENT_OPEN_DEVICE_OK, 0,
			    hashobj->cb_evt_fn, hashobj->cb_obj, evt,
			    strlen(hashobj->port_name));
		}
	} while (0);
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
/*SPSR_CMD_ADD*/
#ifndef __SPSR_EPOLL__
int
spsr_px_add(SPSR_GENERIC_ST *item, SPSR_GENERIC_ST *evt, int *prange,
    struct pollfd *fds)
#else

int
spsr_px_add(SPSR_GENERIC_ST *item, SPSR_GENERIC_ST *evt, int epollfd)
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
	do {
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
			    input->port_name, input->baudrate, &fd);
			if(input->rts || input->dtr) {
				char rts_dtr = 0;
				rts_dtr |= (!!input->rts) ? 0x01 : 0x00;
				rts_dtr |= (!!input->dtr) ? 0x02 : 0x00;
				set_rts_dtr(fd, rts_dtr);
			}
			if (ret) {
				if (input->cb_evt_fn) {
					spsr_invoke_cb(
					    SPSR_EVENT_OPEN_DEVICE_ERROR, ret,
					    input->cb_evt_fn, input->cb_obj,
					    evt, l);
				}
				break;
			}
			memcpy(tmp_port, input->port_name, l);
			temp->item->handle = fd;
#ifndef __SPSR_EPOLL__
			spsr_all("Range of hashtable before adding, "
				 "*prange: %d,  DONE.",
			    *prange);
			if (!prange) {
				spsr_err("prange NULL");
				ret = SPSR_PX_PRANGE_NULL;
				break;
			}
			for (i = 0; i < (*prange + 1); ++i) {
				spsr_all("*prange: %d , "
					 "fds[%d].fd: %d .",
				    *prange, i, fds[i].fd);
				if (fds[i].fd < 0) {
					fds[i].fd = fd;
					fds[i].events = POLLIN;
					(*prange)++;
					spsr_all("Add to poll list, "
						 "index: %d, fd: %d, range: %d",
					    i, fd, (*prange));
					break;
				}
			}
			spsr_all("Range of hashtable after adding, "
				 "*prange: %d,  DONE.",
			    *prange);
#else
			memset(&event, 0, sizeof(event));
			event.events = EPOLLIN | EPOLLET;
			event.data.fd = fd;
			rerr = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
			if (rerr == -1) {
				spsr_err("epoll_ctl error, fd: %d, "
					 "errno: %d, text: %s.",
				    fd, errno, strerror(errno));
				ret = SPSR_UNIX_EPOLL_CTL;
				break;
			}
#endif
			ret = spsr_px_hash_add(temp, evt);

			temp = temp->next;
		}
	} while (0);
	spsr_mutex_unlock(t->mutex);
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
/*SPSR_CMD_REM*/
static
#ifndef __SPSR_EPOLL__
    int
    spsr_px_off_poll(int fd, struct pollfd *fds, int *prange)
#else
    int
    spsr_px_off_poll(int fd, int epollfd)
#endif
{
	int ret = SPSR_PX_POLL_NOT_FOUND;
#ifndef __SPSR_EPOLL__
	int i = 0;
	spsr_all("Did catch handle: %d", fd);
	if (!prange) {
		spsr_err("prange NULL");
		return ret;
	}
	for (i = 1; i < *prange; ++i) {
		if (fds[i].fd == fd) {
			int j = 0;
			for (j = i; j < (*prange - 1); ++j) {
				fds[j].fd = fds[j + 1].fd;
			}

			fds[(*prange - 1)].fd = -1;

			(*prange)--;
			spsr_dbg("EPOLL_CTL_DEL, "
				 "fd: %d DONE",
			    fd);
			ret = 0;
			break;
		}
	}
#else
	int errr = 0;
	spsr_all("Did catch handle: %d", fd);
	errr = epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
	if (errr == -1) {
		spsr_err("epoll_ctl error, fd: %d, "
			 "errno: %d, text: %s.",
		    fd, errno, strerror(errno));
		ret = SPSR_PX_EPOLL_DEL;
	} else {
		spsr_dbg("EPOLL_CTL_DEL, "
			 "fd: %d DONE",
		    fd);
		ret = 0;
	}
#endif
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
static int
spsr_px_off_hash(int fd)
{
	int ret = 0;
	SPSR_HASH_FD_NAME *hashobj = 0;
	SPSR_HASH_FD_NAME *temp = 0;
	SPSR_HASH_FD_NAME *prev = 0;
	int hashid = 0;
	char found = 0;
	do {
		hashid = SPSR_HASH_FD(fd);
		hashobj = (SPSR_HASH_FD_NAME *)spsr_hash_fd_arr[hashid];
		if (!hashobj) {
			spllog(SPL_LOG_ERROR,
			    "SPSR_PX_MAL_HASH_FD Cannot find object.");
			ret = SPSR_PX_MAL_HASH_FD;
			break;
		}
		temp = hashobj;
		while (temp) {
			if (temp->fd != fd) {
				prev = temp;
				temp = temp->next;
				continue;
			}
			/*Found fd.*/
			found = 1;
			break;
		}
		if (!found) {
			ret = SPSR_HASH_NOT_FOUND;
			break;
		}
		if (prev) {
			prev->next = temp->next;
		} else {
			spsr_hash_fd_arr[hashid] = temp->next;
		}
		spsr_dbg("Clear from spsr_hash_fd_arr, "
			 "hashid:%d, fd: %d.",
		    hashid, fd);
		spsr_free(temp);
		break;
	} while (0);
	if (ret) {
		spsr_err("SPSR_HASH_NOT_FOUND Cannot find object.");
	}
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
/*SPSR_CMD_REM*/
#ifndef __SPSR_EPOLL__
int
spsr_px_rem(SPSR_GENERIC_ST *item, SPSR_GENERIC_ST *evt, int *prange,
    struct pollfd *fds)
#else
int
spsr_px_rem(SPSR_GENERIC_ST *item, SPSR_GENERIC_ST *evt, int epollfd)
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
		if (!item) {
			ret = SPSR_PX_ITEM_NULL;
			break;
		}
		if (!evt) {
			ret = SPSR_PX_CB_NULL;
			break;
		}
#ifndef __SPSR_EPOLL__
		if (!prange) {
			ret = SPSR_PX_PRANGE_NULL;
			break;
		}
		if (!fds) {
			ret = SPSR_PX_POLLFD_NULL;
			break;
		}
#else
#endif
		portname = item->data;
		l = strlen(portname);
		spsr_all("port: %s", item->data);
		spsr_mutex_lock(t->mutex);

		do {
			temp = t->init_node;
			spsr_all("SPSR_CMD_REM command, "
				 "pl: %d, "
				 "portname: %s, "
				 "total: %d, "
				 "initnode: 0x%p",
			    item->pl, portname, item->total, temp);
			while (temp) {
				spsr_all("portname: %s", temp->item->port_name);
				if (strcmp(temp->item->port_name, portname)) {
					prev = temp;
					temp = temp->next;
					continue;
				}
				found = 1;
				break;
			}
			if (!found) {
				ret = SPSR_REM_NOT_FOUND;
				spsr_err(
				    "SPSR_REM_NOT_FOUND, port: %s.", portname);
				break;
			}
			fd = temp->item->handle;
			if (fd < 0) {
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
			if (ret) {
				spsr_err("spsr_px_off_poll, ret: %d.", ret);
			}
			ret = spsr_px_off_hash(fd);
			/* Close handle*/

			SPSR_PXCLOSE(fd, errr);

			if (errr) {
				callback_evt = SPSR_EVENT_CLOSE_DEVICE_ERROR;
				ret = SPSR_PX_FD_CLOSED;
			} else {
				callback_evt = SPSR_EVENT_CLOSE_DEVICE_OK;
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
			spsr_dbg("t->count: %d", t->count);
			break;
		} while (0);
		spsr_mutex_unlock(t->mutex);

		if (!callback_fn) {
			break;
		}

		memcpy(evt->data + evt->pc, portname, l);

		ret = spsr_invoke_cb(
		    callback_evt, ret, callback_fn, callback_obj, evt, l);

	} while (0);
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
/* SPSR_CMD_WRITE */

static int
spsr_remote_connected(int fd)
{
	int status = 0;
	if (ioctl(fd, TIOCMGET, &status) == -1) {
		spsr_wrn("ioctl");
		return 0; /*Assume not connected*/
	}
	/*Check if DSR (remote ready) or CTS (clear to send) is on*/
	if (status & TIOCM_DSR) {
		spsr_all("Remote side is READY (DSR set)\n");
		return 1;
	} else {
		spsr_wrn("Remote side NOT ready (DSR not set)\n");
	}
	return 0;
}

int
spsr_px_write(SPSR_GENERIC_ST *item, SPSR_GENERIC_ST *evt)
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
	int connected = 0;
	do {
		if (!item) {
			ret = SPSR_PX_ITEM_NULL;
			spsr_err("SPSR_PX_ITEM_NULL");
			break;
		}
		if (!evt) {
			ret = SPSR_PX_CB_NULL;
			spsr_err("SPSR_PX_CB_NULL");
			break;
		}
		portname = item->data;
		l = strlen(portname);
		spsr_mutex_lock(t->mutex);
		/*do { */
		temp = t->init_node;
		spsr_all("SPSR_CMD_WRITE, pl: %d, "
			 "portname: %s, total: %d, initnode: 0x%p",
		    item->pl, portname, item->total, temp);
		while (temp) {
			spsr_all("portname: %s", temp->item->port_name);
			kcmp = strcmp(temp->item->port_name, portname) ? 0 : 1;
			if (!kcmp) {
				temp = temp->next;
				continue;
			}
			if (temp->item->handle < 0) {
				spsr_err("temp->item->handle: %d",
				    temp->item->handle);
				break;
			}
			fd = temp->item->handle;
			break;
		}
		/*} while (0);*/
		spsr_mutex_unlock(t->mutex);

		if (fd < 0) {
			spsr_err("SPSR_PX_FD_NOT_FOUND");
			ret = SPSR_PX_FD_NOT_FOUND;
			break;
		}

		hashid = SPSR_HASH_FD(fd);

		p = item->data + item->pc;
		wlen = item->pl - item->pc;

		hashobj = (SPSR_HASH_FD_NAME *)spsr_hash_fd_arr[hashid];

		connected = (hashobj->offDSR) ? 1 : spsr_remote_connected(fd);

		if (!connected) {
			spsr_err("Remote unconnected! "
				 "Turn on offDSR for half-duplex! "
				 "Turn off offDSR for full-duplex!"
				 "Port: %s.",
			    hashobj->port_name);
			ret = SPSR_PX_UNCONNECTED;
			break;
		}
		if (tcflush(fd, TCIOFLUSH) == -1) {
			spsr_api_err("tcflush");
			break;
		} else {
			spsr_dbg("tcflush DONE,");
		}

		nwrote = write(fd, p, wlen);
		if (nwrote != wlen) {
			spsr_err("write error, fd: %d, "
				 "errno: %d, text: %s.",
			    fd, errno, strerror(errno));
			break;
		}
		wrote = 1;
		spsr_inf("write DONE, fd: %d, nwrote: %d, wlen: %d, data: %s.", fd,
		    nwrote, wlen, p);
		break;

	} while (0);

	do {
		if (!hashobj) {
			break;
		}
		if (!(hashobj->cb_evt_fn)) {
			break;
		}
		if (l < 1) {
			portname = (char *)"EMPTY";
			l = strlen(portname);
		}
		evtenum = wrote ? SPSR_EVENT_WRITE_OK : SPSR_EVENT_WRITE_ERROR;

		memcpy(evt->data + evt->pc, portname, l);

		spsr_invoke_cb(
		    evtenum, ret, hashobj->cb_evt_fn, hashobj->cb_obj, evt, l);
	} while (0);

	if (wrote) {
		if (tcdrain(fd) == -1) {
			spsr_api_err("tcdrain");
		} else {
			spsr_dbg("tcdrain DONE,");
		}
	}

	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int
spsr_send_cmd(int cmd, char *portname, void *data, int datasz)
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
			    (t->cmd_buff->pl + sizeof(obj))) {
				obj = (SPSR_GENERIC_ST *)(t->cmd_buff->data +
							  t->cmd_buff->pl);
				memset(obj, 0, nsize);
				obj->total = nsize;
				obj->type = cmd;

				t->cmd_buff->pl += nsize;
				pend = (int *)(t->cmd_buff->data +
					       t->cmd_buff->pl);
				*pend = 0;
				spsr_all("cmd type SPSR_CMD_ADD: %d, size: %d",
				    obj->type, obj->total);
			}
			break;
		}
		if (cmd == SPSR_CMD_REM) {
			int lport = 0;
			int len = strlen(portname);

			lport = len + 1;
			nsize = sizeof(SPSR_GENERIC_ST) + lport;
			spsr_all("SPSR_CMD_REM, nsize: %d, portname: %s", nsize,
			    portname);
			if (t->cmd_buff->range > t->cmd_buff->pl + nsize) {
				obj = (SPSR_GENERIC_ST *)(t->cmd_buff->data +
							  t->cmd_buff->pl);
				memset(obj, 0, nsize);
				obj->total = nsize;
				obj->type = cmd;
				obj->range = lport;
				memcpy(obj->data, portname, lport);
				obj->data[len] = 0;
				obj->pl = lport;
				t->cmd_buff->pl += nsize;
				pend = (int *)(t->cmd_buff->data +
					       t->cmd_buff->pl);
				*pend = 0;
			}
			break;
		}
		if (cmd == SPSR_CMD_WRITE) {
			int lport = 0;
			int range = 0;
			int pl = 0;
			char *tmp = 0;
			int len = strlen(portname);

			lport = (len + 1) + datasz;
			nsize = sizeof(SPSR_GENERIC_ST) + lport;
			spsr_all("SPSR_CMD_WRITE, nsize: %d, "
				 "portname: %s",
			    nsize, portname);
			range = t->cmd_buff->range;
			pl = t->cmd_buff->pl;
			if (range < pl + nsize) {
				int total = 0;
				int adding = 2 * nsize;

				total = t->cmd_buff->total;
				total += adding;
				ret = spsr_resize_obj(total, &t->cmd_buff);
				if (ret) {
					break;
				}
			}
			tmp = t->cmd_buff->data;
			tmp += t->cmd_buff->pl;
			obj = (SPSR_GENERIC_ST *)tmp;
			;
			memset(obj, 0, nsize);
			obj->total = nsize;
			obj->type = cmd;
			obj->range = lport;
			memcpy(obj->data, portname, len);
			obj->data[len] = 0;
			obj->pc = len + 1;
			memcpy(obj->data + obj->pc, (char *)data, datasz);
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

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#endif

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int
spsr_is_existed(char *port, int *isExisted)
{
	int ret = 0;
	SPSR_ROOT_TYPE *t = &spsr_root_node;
	SPSR_ARR_LIST_LINED *tmp = 0;
	char *p1 = 0;

	do {
		if (!isExisted) {
			ret = SPSR_OBJ_NULL;
			spsr_err("SPSR_OBJ_NULL.");
			break;
		}
		*isExisted = 0;
		if (!port) {
			ret = SPSR_PORT_NULL;
			spllog(SPSR_PORT_NULL, "SPSR_OBJ_NULL.");
			break;
		}
		if (!port[0]) {
			ret = SPSR_PORT_EMPTY;
			spllog(SPSR_PORT_EMPTY, "SPSR_OBJ_NULL.");
			break;
		}
		if (!t->init_node) {
			break;
		}

		tmp = t->init_node;

		while (tmp) {
			p1 = tmp->item->port_name;
			if (strcmp(p1, port)) {
				tmp = tmp->next;
				continue;
			}
			*isExisted = 1;
			break;
		}
	} while (0);
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int
spsr_verify_info(SPSR_INPUT_ST *p)
{
	SPSR_ROOT_TYPE *t = &spsr_root_node;
	int ret = 0;
	SPSR_INFO_ST *item = 0;
	SPSR_ARR_LIST_LINED *node = 0;
	spsr_dbg("spsr_verify_info:");
#ifndef UNIX_LINUX
	HANDLE hSerial = 0;
#else
	int fd = 0;
#endif
	do {
		if (!p) {
			ret = SPSR_PORT_INPUT_NULL;
			spsr_err("SPSR_PORT_INPUT_NULL.");
			break;
		}
		if (p->baudrate < 1) {
			ret = SPSR_PORT_BAUDRATE_ERROR;
			spsr_err("SPSR_PORT_BAUDRATE_ERROR.");
			break;
		}
		if (!p->port_name[0]) {
			spsr_dbg("port_name empty.");
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
				if (strcmp(p1, p2)) {
					tmp = tmp->next;
					continue;
				}
				spsr_err("SPSR_PORTNAME_EXISTED: \"%s\".",
				    p->port_name);
				ret = SPSR_PORTNAME_EXISTED;
				break;
			}
		}
		if (ret) {
			break;
		}

#ifndef UNIX_LINUX
		/* Open the serial port with FILE_FLAG_OVERLAPPED for
		 * asynchronous operation */
		/* https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea
		 */
		hSerial = CreateFile(p->port_name, GENERIC_READ | GENERIC_WRITE,
		    0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);

		if (hSerial == INVALID_HANDLE_VALUE) {
			DWORD dwError = GetLastError();
			spsr_err("Open port errcode: %lu", dwError);
			ret = SPSR_PORT_OPEN;
			break;
		} else {
			spsr_dbg("Create hSerial: 0x%p.", hSerial);
			SPSR_CloseHandle(hSerial);
		}
#else
		fd = open(p->port_name, O_RDWR | O_NOCTTY | O_NDELAY);
		if (fd == -1) {
			ret = SPSR_PORT_OPEN_UNIX;
			spsr_err("open port: ret: %d, "
				 "errno: %d, text: %s.",
			    ret, errno, strerror(errno));
			break;
		}
		spsr_dbg("open portname: %s, "
			 "fd: %d.",
		    p->port_name, fd);
		SPSR_PXCLOSE(fd, ret);
		if (ret) {
			ret = SPSR_PORT_CLOSE_UNIX;
			spsr_err("close port fd: %d, "
				 "ret: %d, errno: %d, text: %s.",
			    fd, ret, errno, strerror(errno));
			break;
		}
#endif

		spsr_malloc(
		    sizeof(SPSR_ARR_LIST_LINED), node, SPSR_ARR_LIST_LINED);
		if (!node) {
			ret = SPSR_MEM_NULL;
			spsr_err("SPSR_MEM_NULL");
			break;
		}
		spsr_malloc(sizeof(SPSR_INFO_ST), item, SPSR_INFO_ST);
		if (!item) {
			ret = SPSR_MEM_NULL;
			spsr_err("SPSR_MEM_NULL");
			break;
		}

		/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

		snprintf(item->port_name, SPSR_PORT_LEN, "%s", p->port_name);
		item->baudrate = p->baudrate;
		item->offDSR = p->offDSR;
		item->cb_evt_fn = p->cb_evt_fn;
		item->cb_obj = p->cb_obj;
		item->t_delay = p->t_delay;

		item->rts = p->rts;
		item->dtr = p->dtr;

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
		ret = spsr_create_thread(spsr_thread_operating_routine, node);
#else
		item->handle = -1;
		spsr_dbg("Check t->init_node: 0x%p", t->init_node);
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

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

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
		if (!port[0]) {
			continue;
		}
		ret = spsr_inst_close(port);
		if (!ret) {
			continue;
		}
		spsr_err("spsr_inst_close: ret: %d, port: %s.", ret, port);
	} while (count);
#else

	SPSR_ARR_LIST_LINED *tnode = 0, *temp = 0;
	temp = t->init_node;
	while (temp) {
		int fd = -1;
		tnode = temp;
		temp = temp->next;
		if (!tnode->item) {
			spsr_free(tnode);
			continue;
		}
		if (tnode->item->handle < 0) {
			spsr_free(tnode);
			continue;
		}
		fd = tnode->item->handle;
		SPSR_PXCLOSE(fd, ret);
		if (ret) {
			ret = SPSR_PX_CLOSE;
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
		snprintf(output, l, "%s_%s", t->sem_key, input);
	} else {
		snprintf(output, l, "%s_%s", t->sem_key, "");
	}
	return ret;
}
#else
#endif
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

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
			spsr_all("fd: %d, name: %s", tmp->fd, tmp->port_name);
			spsr_free(tmp);
		}
		spsr_hash_fd_arr[i] = 0;
	}
	return ret;
}

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
void set_rts_dtr(int fd, char rts_dtr)
{
    int status = 0;
    if (ioctl(fd, TIOCMGET, &status) == -1) {
        spsr_err("ioctl TIOCMGET");
        return;
    }
	if(0x01 & rts_dtr) {
    	status |= TIOCM_RTS; 
		spsr_all("RTS was found.\n");
	}
	if(0x02 & rts_dtr) {
    	status |= TIOCM_DTR;
		spsr_all("DTR was found.\n");
	}

    if (ioctl(fd, TIOCMSET, &status) == -1) {
        spsr_err("ioctl TIOCMSET");
        return;
    }

    spsr_all("RTS và DTR were on already.\n");
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int
spsr_open_fd(char *port_name, int baudrate, int *outfd)
{
	int ret = 0;
	int fd = -1;
	struct termios options = {0};
	int rerr = 0;
	do {
		fd = open(port_name, O_RDWR | O_NOCTTY | O_NONBLOCK | O_SYNC);
		if (fd == -1) {
			spsr_err("open port error, fd: %d, "
				 "errno: %d, text: %s.",
			    fd, errno, strerror(errno));
			ret = SPSR_UNIX_OPEN_PORT;
			break;
		}
		spsr_all("fd: %d, portname: %s", fd, port_name);
		memset(&options, 0, sizeof(options));
		rerr = tcgetattr(fd, &options);
		if (rerr < 0) {
			spsr_err("tcgetattr error, fd: %d, "
				 "errno: %d, text: %s.",
			    fd, errno, strerror(errno));

			ret = SPSR_UNIX_GET_ATTR;
			break;
		}

		spsr_all("fd: %d, "
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
			spsr_err("tcsetattr error, fd: %d, "
				 "errno: %d, text: %s.",
			    fd, errno, strerror(errno));
			ret = SPSR_UNIX_SET_ATTR;
			break;
		} else {
			spsr_all("tcsetattr: DONE.")
		}
		*outfd = fd;
	} while (0);

	return ret;
}

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

int
spsr_px_read(int fd, SPSR_GENERIC_ST *pevtcb, char *chk_delay)
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
	int len = 0;
	do {
		if (!pevtcb) {
			ret = SPSR_MEM_NULL;
			spsr_err("pevtcb NULL");
			break;
		}
		evtcb = pevtcb;
		if (!evtcb) {
			ret = SPSR_MEM_NULL;
			spsr_err("evtcb NULL");
			break;
		}
		buffer = evtcb->data + evtcb->pc;
		do {
			int hasdid = SPSR_HASH_FD(comfd);

			hashobj = (SPSR_HASH_FD_NAME *)spsr_hash_fd_arr[hasdid];
			if (!hashobj) {
				spsr_err("Cannot find obj in reading.");
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
				spsr_err("read error, fd: %d, "
					 "errno: %d, text: %s.",
				    fd, errno, strerror(errno));
				ret = SPSR_PX_READ;
				len = snprintf(buffer, range, "%s|%s",
				    temp ? temp->port_name : "",
				    spsr_err_txt(ret));
				break;
			}
			/* } */
			buffer[didread] = 0;
			spsr_all("Didread: %d, data: \"%s\", "
				 "fd: %d, temp->t_delay/timeout: %d",
			    didread, buffer, fd, t_wait);

			if (!temp) {
				ret = SPSR_HASH_NOTFOUND;
				spsr_err("Didsee Cannot find obj in reading.");
				break;
			}
			if (!temp->cb_evt_fn) {
				spsr_dbg("cb_evt_fn, cb_evt_fn null.");
				break;
			}
			len = didread;
#if 0
			spsr_invoke_cb(SPSR_EVENT_READ_BUF, 
				ret, temp->cb_evt_fn,
			    temp->cb_obj, evtcb, didread);
#endif
			break;
		} while (0);

	} while (0);

	do {
		int evtcode = 0;
		if (!temp) {
			break;
		}
		if (!temp->cb_evt_fn) {
			break;
		}
		if (!evtcb) {
			break;
		}
		evtcode = ret ? SPSR_EVENT_READ_ERROR : SPSR_EVENT_READ_BUF;

		spsr_invoke_cb(
		    evtcode, ret, temp->cb_evt_fn, temp->cb_obj, evtcb, len);

	} while (0);
	return ret;
}

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

static int
spsr_check_resz(int lenp, SPSR_GENERIC_ST **pcart_buff)
{
	int ret = 0;

	if (lenp <= (*pcart_buff)->range) {
		return ret;
	}

	do {
		int add = 0;
		int total = 0;

		add = lenp - (*pcart_buff)->range + 1;
		total = (*pcart_buff)->total + add;
		ret = spsr_resize_obj(total, pcart_buff);

	} while (0);

	return ret;
}

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

#ifndef __SPSR_EPOLL__
int
spsr_ctrl_sock(void *fds, int *mx_number, int sockfd, SPSR_GENERIC_ST *evt,
    int *chk_off, SPSR_GENERIC_ST **pcart_buff)
#else
int
spsr_ctrl_sock(int epollfd, int sockfd, SPSR_GENERIC_ST *evt, int *chk_off,
    SPSR_GENERIC_ST **pcart_buff)
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
			lenmsg = (int)recvfrom(sockfd, buffer, evt->range, 0,
			    (struct sockaddr *)&client_addr, &client_len);

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
				/*
				spsr_err("mach recvfrom, "
					 "lenmsg: %d, errno: %d, text: %s.",
				    (int)lenmsg, errno, strerror(errno));
				*/
				spsr_api_err("recvfrom");
				break;
			}
			buffer[lenmsg] = 0;
			if (lenmsg == 1) {
				isoff = (int)buffer[0];
			} else {
				int i = 0;
				for (; i < lenmsg; ++i) {
					if (buffer[i]) {
						isoff = 1;
						break;
					}
				}
			}
			/*
			buffer[lenmsg] = 0;
			spsr_dbg("buffer: %s", buffer);
			if (strcmp(buffer, SPSR_MSG_OFF) == 0) {
				spsr_dbg(SPSR_MSG_OFF);
				isoff = 1;
				break;
			}
			*/
			if (isoff) {
				break;
			}
			lenp = 0;

			p = 0;
			(*pcart_buff)->pl = 0;
			spsr_mutex_lock(t->mutex);
			do {
				if (!t->cmd_buff) {
					ret = SPSR_MEM_NULL;
					spsr_err("SPSR_MEM_NULL");
					break;
				}

				lenp = t->cmd_buff->pl;

				if (!lenp) {
					break;
				}

				spsr_check_resz(lenp, pcart_buff);

				p = (*pcart_buff)->data;
				memcpy(p, t->cmd_buff->data, lenp);
				(*pcart_buff)->pl = lenp;
				t->cmd_buff->pl = 0;

			} while (0);

			spsr_mutex_unlock(t->mutex);

			if (!p) {
				continue;
			}

#ifndef __SPSR_EPOLL__
			ret = spsr_fetch_commands(fds, mx_number, p, lenp, evt);
			spsr_all("MACH POLL --->>> "
				 "Size of buffer of command : %d",
			    lenp);
#else
			ret = spsr_fetch_commands(epollfd, p, lenp, evt);
			spsr_all("LINUX EPOLL --->>> "
				 "Size of buffer of command : %d",
			    lenp);
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

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int
spsr_mutex_delete(void *mtx)
{
	int ret = 0;
	do {
#ifndef UNIX_LINUX
		SPSR_CloseHandle(mtx);
#else
		ret = pthread_mutex_destroy((pthread_mutex_t *)mtx);
		spsr_all("Delete 0x%p", mtx);
		spsr_free(mtx);
#endif
	} while (0);
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int
spsr_sem_delete(void *sem, char *sem_name)
{
	int ret = 0;
	do {
#ifndef UNIX_LINUX
		SPSR_CloseHandle(sem);
#else
#ifndef __SPSR_EPOLL__
		char name[SPSR_KEY_LEN * 2];
		int err = 0;
		spsr_fmt_name(sem_name, name, SPSR_KEY_LEN * 2);
		err = sem_close((sem_t *)sem);
		if (err == -1) {
			ret = SPSR_SEM_CLOSE;
			spsr_api_err("sem_close");
		}

		spsr_all("Sem Delete 0x%p,  "
			 "sem_name: %s, name: %s",
		    sem, sem_name, name);

		err = sem_unlink(name);
		if (err) {
			ret = SPSR_SEM_UNLINK;
			spsr_api_err("sem_unlink");
		}
		/* spsr_free(sem); */
#else
		ret = sem_destroy((sem_t *)sem);
		spsr_all("Delete 0x%p", sem);
		spsr_free(sem);
#endif
#endif
	} while (0);
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int
spsr_invoke_cb(int evttype, int err_code, SPSR_module_cb fn_cb, void *obj_cb,
    SPSR_GENERIC_ST *evt, int lendata)
{
	int ret = 0;
#ifdef SPSR_SHOW_CONSOLE
	spsr_all("Enter call callback.");
#endif
	do {
		if (!fn_cb) {
			ret = SPSR_CALLBACK_NULL;
			break;
		}
		evt->err_code = err_code;
		evt->type = evttype;
		evt->pc = sizeof(void *);

		if (sizeof(void *) == sizeof(SPSR_UINT)) {
			SPSR_UINT *pt = (SPSR_UINT *)evt->data;
			*pt = (SPSR_UINT)obj_cb;
			spsr_dbg("With 32 bit.");
		} else if (sizeof(void *) == sizeof(SPSR_LLU)) {
			SPSR_LLU *pt = (SPSR_LLU *)evt->data;
			spsr_dbg("With 64 bit.");
			*pt = (SPSR_LLU)obj_cb;
		}
		evt->pl = evt->pc + lendata;
		evt->data[evt->pl] = 0;
		fn_cb(evt);
		evt->pl = evt->pc;
	} while (0);
	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
int
spsr_resize_obj(int sz, SPSR_GENERIC_ST **obj)
{
	int ret = 0;
	SPSR_GENERIC_ST *p = 0;
	int range = 0;
	int total = 0;
	int delta = 0;

	do {
		if (sz < (2 * sizeof(SPSR_GENERIC_ST))) {
			ret = SPSR_MINI_SIZE;
			spsr_err("SPSR_MINI_SIZE");
			break;
		}
		if (!obj) {
			ret = SPSR_OBJ_NULL;
			spsr_err("SPSR_OBJ_NULL");
			break;
		}
		p = *obj;
		if (p) {
			total = p->total;
			range = p->range;
			p = (SPSR_GENERIC_ST *)realloc(p, (sz + 1));
		} else {
			spsr_malloc((sz + 1), p, SPSR_GENERIC_ST);
		}

		if (!p) {
			ret = SPSR_MEM_NULL;
			spsr_err("SPSR_MEM_NULL");
			break;
		}
		delta = sz - total;
		p->total += delta;

		p->range += range ? delta : (sz - sizeof(SPSR_GENERIC_ST));

		*obj = p;
	} while (0);

	return ret;
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
static const char *__spsr_err_text__[SPSR_PORT_PEAK + 1];

void
spsr_err_txt_init()
{
	__spsr_err_text__[SPSR_PORT_OK] = "SPSR_PORT_OK";
	__spsr_err_text__[SPSR_PORT_INFO_NULL] = "SPSR_PORT_INFO_NULL";
	__spsr_err_text__[SPSR_PORT_INPUT_NULL] = "SPSR_PORT_INPUT_NULL";
	__spsr_err_text__[SPSR_IDD_NULL] = "SPSR_IDD_NULL";
	__spsr_err_text__[SPSR_OUTPUT_NULL] = "SPSR_OUTPUT_NULL";
	__spsr_err_text__[SPSR_PORT_OPEN] = "SPSR_PORT_OPEN";
	__spsr_err_text__[SPSR_PORT_OPEN_UNIX] = "SPSR_PORT_OPEN_UNIX";
	__spsr_err_text__[SPSR_PORT_CLOSE_UNIX] = "SPSR_PORT_CLOSE_UNIX";
	__spsr_err_text__[SPSR_PORT_COMMSTATE] = "SPSR_PORT_COMMSTATE";
	__spsr_err_text__[SPSR_PORT_GETCOMMSTATE] = "SPSR_PORT_GETCOMMSTATE";
	__spsr_err_text__[SPSR_PORT_SETCOMMSTATE] = "SPSR_PORT_SETCOMMSTATE";
	__spsr_err_text__[SPSR_PORT_CREATEEVENT] = "SPSR_PORT_CREATEEVENT";
	__spsr_err_text__[SPSR_PORT_SETCOMMTIMEOUTS] =
	    "SPSR_PORT_SETCOMMTIMEOUTS";
	__spsr_err_text__[SPSR_PORT_SPSR_MUTEX_CREATE] =
	    "SPSR_PORT_SPSR_MUTEX_CREATE";
	__spsr_err_text__[SPSR_PORT_SPSR_SEM_CREATE] =
	    "SPSR_PORT_SPSR_SEM_CREATE";
	__spsr_err_text__[SPSR_PORT_BAUDRATE_ERROR] =
	    "SPSR_PORT_BAUDRATE_ERROR";
	__spsr_err_text__[SPSR_PORT_NAME_ERROR] = "SPSR_PORT_NAME_ERROR";
	__spsr_err_text__[SPSR_MTX_CREATE] = "SPSR_MTX_CREATE";
	__spsr_err_text__[SPSR_SEM_CREATE] = "SPSR_SEM_CREATE";
	__spsr_err_text__[SPSR_GEN_IDD] = "SPSR_GEN_IDD";
	__spsr_err_text__[SPSR_MEM_NULL] = "SPSR_MEM_NULL";
	__spsr_err_text__[SPSR_MUTEX_NULL_ERROR] = "SPSR_MUTEX_NULL_ERROR";
	__spsr_err_text__[SPSR_SEM_NULL_ERROR] = "SPSR_SEM_NULL_ERROR";
	__spsr_err_text__[SPSR_SEM_POST_ERROR] = "SPSR_SEM_POST_ERROR";
	__spsr_err_text__[SPSR_SEM_UNLINK] = "SPSR_SEM_UNLINK";
	__spsr_err_text__[SPSR_SEM_CLOSE] = "SPSR_SEM_CLOSE";
	__spsr_err_text__[SPSR_INPUT_NULL_ERROR] = "SPSR_INPUT_NULL_ERROR";
	__spsr_err_text__[SPSR_THREAD_W32_CREATE] = "SPSR_THREAD_W32_CREATE";
	__spsr_err_text__[SPSR_NOT_FOUND_IDD] = "SPSR_NOT_FOUND_IDD";
	__spsr_err_text__[SPSR_REALLOC_ERROR] = "SPSR_REALLOC_ERROR";
	__spsr_err_text__[SPSR_MALLOC_ERROR] = "SPSR_MALLOC_ERROR";
	__spsr_err_text__[SPSR_INFO_NULL] = "SPSR_INFO_NULL";
	__spsr_err_text__[SPSR_PARAM_NULL] = "SPSR_PARAM_NULL";
	__spsr_err_text__[SPSR_ITEM_NOT_FOUND] = "SPSR_ITEM_NOT_FOUND";
	__spsr_err_text__[SPSR_CREATE_THREAD_ERROR] =
	    "SPSR_CREATE_THREAD_ERROR";
	__spsr_err_text__[SPSR_SHUTDOWN_SOCK] = "SPSR_SHUTDOWN_SOCK";
	__spsr_err_text__[SPSR_CLOSE_SOCK] = "SPSR_CLOSE_SOCK";
	__spsr_err_text__[SPSR_CREATE_SOCK] = "SPSR_CREATE_SOCK";
	__spsr_err_text__[SPSR_FCNTL_SOCK] = "SPSR_FCNTL_SOCK";
	__spsr_err_text__[SPSR_BIND_SOCK] = "SPSR_BIND_SOCK";
	__spsr_err_text__[SPSR_EPOLL_CREATE] = "SPSR_EPOLL_CREATE";
	__spsr_err_text__[SPSR_EPOLL_CTL] = "SPSR_EPOLL_CTL";
	__spsr_err_text__[SPSR_BUFF_EXCEED] = "SPSR_BUFF_EXCEED";
	__spsr_err_text__[SPSR_UNIX_OPEN_PORT] = "SPSR_UNIX_OPEN_PORT";
	__spsr_err_text__[SPSR_UNIX_GET_ATTR] = "SPSR_UNIX_GET_ATTR";
	__spsr_err_text__[SPSR_UNIX_SET_ATTR] = "SPSR_UNIX_SET_ATTR";
	__spsr_err_text__[SPSR_UNIX_EPOLL_CTL] = "SPSR_UNIX_EPOLL_CTL";
	__spsr_err_text__[SPSR_PORTNAME_EXISTED] = "SPSR_PORTNAME_EXISTED";
	__spsr_err_text__[SPSR_HASH_NOTFOUND] = "SPSR_HASH_NOTFOUND";
	__spsr_err_text__[SPSR_CALLBACK_NULL] = "SPSR_CALLBACK_NULL";
	__spsr_err_text__[SPSR_PX_ITEM_NULL] = "SPSR_PX_ITEM_NULL";
	__spsr_err_text__[SPSR_PX_CB_NULL] = "SPSR_PX_CB_NULL";
	__spsr_err_text__[SPSR_PX_POLLFD_NULL] = "SPSR_PX_POLLFD_NULL";
	__spsr_err_text__[SPSR_PX_PRANGE_NULL] = "SPSR_PX_PRANGE_NULL";
	__spsr_err_text__[SPSR_PX_EPOLL_DEL] = "SPSR_PX_EPOLL_DEL";
	__spsr_err_text__[SPSR_PX_POLL_NOT_FOUND] = "SPSR_PX_POLL_NOT_FOUND";
	__spsr_err_text__[SPSR_PX_MALINFO_FD] = "SPSR_PX_MALINFO_FD";
	__spsr_err_text__[SPSR_PX_MAL_HASH_FD] = "SPSR_PX_MAL_HASH_FD";
	__spsr_err_text__[SPSR_HASH_NOT_FOUND] = "SPSR_HASH_NOT_FOUND";
	__spsr_err_text__[SPSR_REM_NOT_FOUND] = "SPSR_REM_NOT_FOUND";
	__spsr_err_text__[SPSR_WIN32_OBJ_NULL] = "SPSR_WIN32_OBJ_NULL";
	__spsr_err_text__[SPSR_WIN32_BUF_NULL] = "SPSR_WIN32_BUF_NULL";
	__spsr_err_text__[SPSR_WIN32_BWRITE_NULL] = "SPSR_WIN32_BWRITE_NULL";
	__spsr_err_text__[SPSR_WIN32_OVERLAP_NULL] = "SPSR_WIN32_OVERLAP_NULL";
	__spsr_err_text__[SPSR_WIN32_EVTCB_NULL] = "SPSR_WIN32_EVTCB_NULL";
	__spsr_err_text__[SPSR_WIN32_NOT_PENDING] = "SPSR_WIN32_NOT_PENDING";
	__spsr_err_text__[SPSR_WIN32_OVERLAP_ERR] = "SPSR_WIN32_OVERLAP_ERR";
	__spsr_err_text__[SPSR_PORT_NULL] = "SPSR_PORT_NULL";
	__spsr_err_text__[SPSR_PORT_EMPTY] = "SPSR_PORT_EMPTY";
	__spsr_err_text__[SPSR_OBJ_NULL] = "SPSR_OBJ_NULL";
	__spsr_err_text__[SPSR_PORTNAME_NONEXISTED] =
	    "SPSR_PORTNAME_NONEXISTED";
	__spsr_err_text__[SPSR_PX_FD_NOT_FOUND] = "SPSR_PX_FD_NOT_FOUND";
	__spsr_err_text__[SPSR_WIN32_RL_MTX] = "SPSR_WIN32_RL_MTX";
	__spsr_err_text__[SPSR_WIN32_LK_MTX] = "SPSR_WIN32_LK_MTX";
	__spsr_err_text__[SPSR_PX_LK_MTX] = "SPSR_PX_LK_MTX";
	__spsr_err_text__[SPSR_PX_RL_MTX] = "SPSR_PX_RL_MTX";
	__spsr_err_text__[SPSR_PX_RL_SEM] = "SPSR_PX_RL_SEM";
	__spsr_err_text__[SPSR_PX_WAIT_SEM] = "SPSR_PX_WAIT_SEM";
	__spsr_err_text__[SPSR_WIN32_WAIT_SEM] = "SPSR_WIN32_WAIT_SEM";
	__spsr_err_text__[SPSR_WIN32_RL_SEM] = "SPSR_WIN32_RL_SEM";
	__spsr_err_text__[SPSR_MINI_SIZE] = "SPSR_MINI_SIZE";
	__spsr_err_text__[SPSR_PX_READ] = "SPSR_PX_READ";
	__spsr_err_text__[SPSR_PX_UNCONNECTED] = "SPSR_PX_UNCONNECTED";
	__spsr_err_text__[SPSR_WIN32_UNCONNECTED] = "SPSR_WIN32_UNCONNECTED";
	__spsr_err_text__[SPSR_PX_FD_CLOSED] = "SPSR_PX_FD_CLOSED";
	__spsr_err_text__[SPSR_WIN32_FD_CLOSED] = "SPSR_WIN32_FD_CLOSED";
	__spsr_err_text__[SPSR_WIN32_CLEARCOMM] = "SPSR_WIN32_CLEARCOMM";
	__spsr_err_text__[SPSR_WIN32_STILL_INQUE] = "SPSR_WIN32_STILL_INQUE";
	__spsr_err_text__[SPSR_PX_CLOSE] = "SPSR_PX_CLOSE";
	__spsr_err_text__[SPSR_WIN32_GETOVERLAP] = "SPSR_WIN32_GETOVERLAP";
	__spsr_err_text__[SPSR_WIN32_BYTEWRITE] = "SPSR_WIN32_BYTEWRITE";
	__spsr_err_text__[SPSR_WIN32_NOTPENDING] = "SPSR_WIN32_NOTPENDING";
	__spsr_err_text__[SPSR_WIN32_WOBJ] = "SPSR_WIN32_WOBJ";

	__spsr_err_text__[SPSR_PORT_PEAK] = "SPSR_PORT_PEAK";
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
const char *
spsr_err_txt(int i)
{
	if (i < 0) {
		return "UNKNOW - less than 0.";
	}
	if (i > SPSR_PORT_PEAK) {
		return "UNKNOW - greater than SPSR_PORT_PEAK.";
	}
	return __spsr_err_text__[i];
}
#ifndef UNIX_LINUX
#else
#endif

#ifndef __SPSR_EPOLL__
#else
#endif
