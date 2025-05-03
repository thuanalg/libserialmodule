/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
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
* Decription:
*		The (only) main header file to export 5 APIs: [spsr_module_init, spsr_module_finish, spsr_inst_open,
spsr_inst_close, spsr_inst_write].
*/
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#ifndef ___SIMPLE_SERIAL_MODULE__
#define ___SIMPLE_SERIAL_MODULE__ 
#include <stdlib.h>
#include <simplelog.h>

#if 0
#ifndef UNIX_LINUX
#define UNIX_LINUX                
#endif
#endif

#if 0
#ifndef __SPSR_EPOLL__
#define __SPSR_EPOLL__            
#endif
#endif

#if 1
#ifndef __ADD_SIMPLE_LOG__
#define __ADD_SIMPLE_LOG__        
#endif
#endif

#ifdef __ADD_SIMPLE_LOG__
#include <simplelog.h>
#else
#define spllog                    spsr_console_log
#endif

#ifndef __SP_FILLE__
#define __SP_FILLE__(__p__)                                                                                                 \
	do {                                                                                                                \
		__p__ = strrchr(__FILE__, '/');                                                                             \
		if (__p__) {                                                                                                \
			++__p__;                                                                                            \
			break;                                                                                              \
		}                                                                                                           \
		__p__ = strrchr(__FILE__, '\\');                                                                            \
		if (__p__) {                                                                                                \
			++__p__;                                                                                            \
			break;                                                                                              \
		}                                                                                                           \
		__p__ = __FILE__;                                                                                           \
	} while (0);
#endif

#ifndef __UNIX_LINUX_CPP11_AND_NEWERS__
#define spsr_console_log(__lv__, ___fmttt___, ...)                                                                          \
	{                                                                                                                   \
		;                                                                                                           \
		const char *pfn = 0;                                                                                        \
		__SP_FILLE__(pfn);                                                                                          \
		;                                                                                                           \
		fprintf(stdout,                                                                                             \
		    "[%d - %s - %s] [%s:%s:%d] "___fmttt___                                                                 \
		    "\n",                                                                                                   \
		    (__lv__), __DATE__, __TIME__, (char *)pfn, (char *)__FUNCTION__, (int)__LINE__, ##__VA_ARGS__);         \
	}
#else
#define spsr_console_log(__lv__, ___fmttt___, ...)                                                                          \
	{                                                                                                                   \
		;                                                                                                           \
		std::string __c11fmt__ = "[%d - %s - %s] [%s:%s:%d] ";                                                      \
		__c11fmt__ += ___fmttt___;                                                                                  \
		__c11fmt__ += "\n";                                                                                         \
		const char *pfn = 0;                                                                                        \
		__FILLE__(pfn);                                                                                             \
		;                                                                                                           \
		fprintf(stdout, __c11fmt__.c_str(), (__lv__), __DATE__, __TIME__, (char *)pfn, (char *)__FUNCTION__,        \
		    (int)__LINE__, ##__VA_ARGS__);                                                                          \
	}
#endif

#define spsr_malloc(__nn__, __obj__, __type__)                                                                          \
	{                                                                                                                   \
		(__obj__) = (__type__ *)malloc(__nn__);                                                                     \
		if (__obj__) {                                                                                              \
			spllog(0, "[MEM] Malloc: 0x%p.", (__obj__));                                                              \
			memset((__obj__), 0, (__nn__));                                                                     \
		} else {                                                                                                    \
			spllog(0, "Malloc: error.");                                                                        \
		}                                                                                                           \
	}

#define spsr_free(__obj__)                                                                                              \
	{                                                                                                                   \
		if (__obj__) {                                                                                              \
			spllog(0, "[MEM] Free: 0x%p.", (__obj__));                                                                \
			free(__obj__);                                                                                      \
			(__obj__) = 0;                                                                                      \
		}                                                                                                           \
	}

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UNIX_LINUX
#ifndef __SIMPLE_STATIC_SERIAL_MODULE__
#ifdef EXPORT_DLL_API_SERIAL_MODULE
#define DLL_API_SERIAL_MODULE     __declspec(dllexport)
#else
#define DLL_API_SERIAL_MODULE     __declspec(dllimport)
#endif
#else
#define DLL_API_SERIAL_MODULE     
#endif
#else
#define DLL_API_SERIAL_MODULE     
#ifndef __SPSR_EPOLL__
#define SPSR_MAINKEY              "main"
#define SPSR_MAINKEY_MACH         "main_mach"
#endif
#endif /*! UNIX_LINUX */

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

#define SPSR_PORT_LEN             32
#define SPSR_KEY_LEN              (SPSR_PORT_LEN)
#ifndef SPSR_LLU
#define SPSR_LLU                  unsigned long long
#endif
#ifndef SPSR_UNIT
#define SPSR_UNIT                 unsigned int
#endif

typedef enum {
	SPSR_CMD_STORAGE_INPUT_INFO_ARR,
	SPSR_CMD_ADD,
	SPSR_CMD_REM,
	SPSR_CMD_WRITE,

	SPSR_CMD_PEAK
} SPSR_CMD_TYPE;

typedef int (*SPSR_module_cb)(void *);



typedef enum {
	SPSR_PORT_OK,
	SPSR_PORT_INFO_NULL,
	SPSR_PORT_INPUT_NULL,
	SPSR_IDD_NULL,
	SPSR_OUTPUT_NULL,
	SPSR_PORT_OPEN,
	SPSR_PORT_OPEN_UNIX,
	SPSR_PORT_CLOSE_UNIX,
	SPSR_PORT_COMMSTATE,
	SPSR_PORT_GETCOMMSTATE,
	SPSR_PORT_SETCOMMSTATE,
	SPSR_PORT_CREATEEVENT,
	SPSR_PORT_SETCOMMTIMEOUTS,
	SPSR_PORT_SPSR_MUTEX_CREATE,
	SPSR_PORT_SPSR_SEM_CREATE,
	SPSR_PORT_BAUDRATE_ERROR,
	SPSR_PORT_NAME_ERROR,
	SPSR_MTX_CREATE,
	SPSR_SEM_CREATE,
	SPSR_GEN_IDD,
	SPSR_MEM_NULL,
	SPSR_MUTEX_NULL_ERROR,
	SPSR_SEM_NULL_ERROR,
	SPSR_SEM_POST_ERROR,
	SPSR_SEM_UNLINK,
	SPSR_SEM_CLOSE,
	SPSR_INPUT_NULL_ERROR,
	SPSR_THREAD_W32_CREATE,
	SPSR_NOT_FOUND_IDD,
	SPSR_REALLOC_ERROR,
	SPSR_MALLOC_ERROR,
	SPSR_INFO_NULL,
	SPSR_PARAM_NULL,
	SPSR_ITEM_NOT_FOUND,
	SPSR_CREATE_THREAD_ERROR,
	SPSR_SHUTDOWN_SOCK,
	SPSR_CLOSE_SOCK,
	SPSR_CREATE_SOCK,
	SPSR_FCNTL_SOCK,
	SPSR_BIND_SOCK,
	SPSR_EPOLL_CREATE,
	SPSR_EPOLL_CTL,
	SPSR_BUFF_EXCEED,
	SPSR_UNIX_OPEN_PORT,
	SPSR_UNIX_GET_ATTR,
	SPSR_UNIX_SET_ATTR,
	SPSR_UNIX_EPOLL_CTL,
	SPSR_PORTNAME_EXISTED,
	SPSR_HASH_NOTFOUND,
	SPSR_CALLBACK_NULL,
	SPSR_PX_ITEM_NULL,
	SPSR_PX_CB_NULL,
	SPSR_PX_POLLFD_NULL,
	SPSR_PX_PRANGE_NULL,
	SPSR_PX_EPOLL_DEL,
	SPSR_PX_POLL_NOT_FOUND,
	SPSR_PX_MALINFO_FD,
	SPSR_PX_MAL_HASH_FD,
	SPSR_HASH_NOT_FOUND,
	SPSR_REM_NOT_FOUND,
	SPSR_WIN32_OBJ_NULL,
	SPSR_WIN32_BUF_NULL,
	SPSR_WIN32_BWRITE_NULL,
	SPSR_WIN32_OVERLAP_NULL,
	SPSR_WIN32_EVTCB_NULL,
	SPSR_WIN32_NOT_PENDING,
	SPSR_WIN32_OVERLAP_ERR,
	SPSR_PORT_NULL,
	SPSR_PORT_EMPTY,
	SPSR_OBJ_NULL,
	SPSR_PORTNAME_NONEXISTED,

	SPSR_PORT_PEAK,
} SPSR_PORT_ERR;

typedef enum {
	SPSR_EVENT_READ_BUF,
	SPSR_EVENT_WRITE_OK,
	SPSR_EVENT_WRITE_ERROR,
	SPSR_EVENT_OPEN_DEVICE_OK,
	SPSR_EVENT_OPEN_DEVICE_ERROR,
	SPSR_EVENT_CLOSE_DEVICE_OK,
	SPSR_EVENT_CLOSE_DEVICE_ERROR,

	SPSR_EVENT_PEAK,
} SPSR_MODULE_EVENT;

typedef struct __SP_SERIAL_GENERIC_ST__ {
	int total;
	int range;
	int pl;
	int pc;
	int type;
	char data[0];
} SP_SERIAL_GENERIC_ST;

typedef struct __SP_SERIAL_INPUT_ST__ {
	int baudrate;
	char port_name[SPSR_PORT_LEN];
	SPSR_module_cb cb_evt_fn;
	void *cb_obj;

	int t_delay;

} SP_SERIAL_INPUT_ST;

typedef struct __SP_SERIAL_INFO_ST__ {
	int t_delay;
	char isoff;
	char is_retry;
	int baudrate;
	char port_name[SPSR_PORT_LEN];

#ifndef UNIX_LINUX
	void *hEvent;

	void *
#else
	int
#endif
	    handle;

	void *mtx_off;
	void *sem_off; /*It need to wait for completing.*/
#ifdef UNIX_LINUX
#endif
	SPSR_module_cb cb_evt_fn;
	void *cb_obj;
	SP_SERIAL_GENERIC_ST *buff;

} SP_SERIAL_INFO_ST;

typedef struct __SPSR_ARR_LIST_LINED__ {
	SP_SERIAL_INFO_ST *item;
	/* struct __SPSR_ARR_LIST_LINED__* prev; */
	struct __SPSR_ARR_LIST_LINED__ *next;
} SPSR_ARR_LIST_LINED;

typedef struct __SPSR_ROOT_TYPE__ {
	// int n;
	int count;
	void *mutex;
	void *sem;
	int spsr_off; /* Check off.*/
#ifndef UNIX_LINUX
#else

	void *sem_spsr; /* Check off.*/
	SP_SERIAL_GENERIC_ST *cmd_buff; /* Command list .*/
#ifndef __SPSR_EPOLL__
	char sem_key[SPSR_KEY_LEN]; /*It need to wait for UNIX_LINUX --->>> Mach.*/
#endif
#endif
	SPSR_ARR_LIST_LINED *init_node;
	SPSR_ARR_LIST_LINED *last_node;
} SPSR_ROOT_TYPE;

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

/*Must be started before using. Thread-safe, but should be at a start of main function.*/
DLL_API_SERIAL_MODULE int
spsr_module_init();

/*Should be invoked to complete using. Thread-safe, but should be at an end of main function.*/
DLL_API_SERIAL_MODULE int
spsr_module_finish();

/*Open a COM port for using. Thread-safe.*/
/*SP_SERIAL_INPUT_ST: port_name, baudrate, callback, ...*/
DLL_API_SERIAL_MODULE int
spsr_inst_open(SP_SERIAL_INPUT_ST *input);

/*Close the COM port after using. Thread-safe.*/
DLL_API_SERIAL_MODULE int
spsr_inst_close(char *portname);

/*Close the COM port after using. Thread-safe.*/
DLL_API_SERIAL_MODULE int
spsr_inst_write(char *portname, char *data, int sz);

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#ifdef __cplusplus
}
#endif

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#endif
