#ifndef ___SIMPLE_SERIAL_MODULE__
#define ___SIMPLE_SERIAL_MODULE__
#include <stdlib.h>
#include <simplelog.h>

#define spserial_malloc(__nn__, __obj__, __type__) { (__obj__) = (__type__*) malloc(__nn__); if(__obj__) \
	{spllog(0, "Malloc: 0x%p.", (__obj__)); memset((__obj__), 0, (__nn__));} \
	else {spllog(0, "Malloc: error.");}} 

#define spserial_free(__obj__)   { if(__obj__) { spllog(0, "Free: 0x%p.", (__obj__)); free(__obj__); } }

#ifdef __cplusplus
extern "C" {
#endif

/*
#ifndef UNIX_LINUX
	#define  UNIX_LINUX
#endif
*/

#ifndef  UNIX_LINUX
	#ifndef __SIMPLE_STATIC_SERIAL_MODULE__
		#ifdef EXPORT_DLL_API_SERIAL_MODULE
			#define DLL_API_SERIAL_MODULE		__declspec(dllexport)
		#else
			#define DLL_API_SERIAL_MODULE		__declspec(dllimport)
		#endif
	#else
		#define DLL_API_SERIAL_MODULE
	#endif
#else
	#define DLL_API_SERIAL_MODULE
	#ifndef __SPSR_EPOLL__
		#define SPSR_MAINKEY				"main"
		#define SPSR_MAINKEY_MACH			"main_mach"
	#endif
#endif /*! UNIX_LINUX */ 

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

#define SPSERIAL_PORT_LEN						32
#define SPSERIAL_KEY_LEN						(SPSERIAL_PORT_LEN * 2)
#ifndef LLU
	#define LLU				unsigned long long
#endif
	typedef enum {
		SPSR_CMD_STORAGE_INPUT_INFO_ARR,
		SPSR_CMD_ADD,
		SPSR_CMD_REM,
		SPSR_CMD_WRITE,



		SPSR_CMD_PEAK
	} SPSR_CMD_TYPE;

	typedef int (*SPSERIAL_module_cb)(void*);

	typedef enum {
		SPSERIAL_PORT_OK,
		SPSERIAL_PORT_INFO_NULL,
		SPSERIAL_PORT_INPUT_NULL,
		SPSERIAL_IDD_NULL,
		SPSERIAL_OUTPUT_NULL,
		SPSERIAL_PORT_OPEN,
		SPSERIAL_PORT_OPEN_UNIX,
		SPSERIAL_PORT_CLOSE_UNIX,
		SPSERIAL_PORT_COMMSTATE,
		SPSERIAL_PORT_GETCOMMSTATE,
		SPSERIAL_PORT_SETCOMMSTATE,
		SPSERIAL_PORT_CREATEEVENT,
		SPSERIAL_PORT_SETCOMMTIMEOUTS,
		SPSERIAL_PORT_SPSERIAL_MUTEX_CREATE,
		SPSERIAL_PORT_SPSERIAL_SEM_CREATE,
		SPSERIAL_PORT_BAUDRATE_ERROR,
		SPSERIAL_PORT_NAME_ERROR,
		SPSERIAL_MTX_CREATE,
		SPSERIAL_SEM_CREATE,
		SPSERIAL_GEN_IDD,
		SPSERIAL_MEM_NULL,
		SPSERIAL_MUTEX_NULL_ERROR,
		SPSERIAL_SEM_NULL_ERROR,
		SPSERIAL_SEM_POST_ERROR,
		PSERIAL_SEM_UNLINK,
		PSERIAL_SEM_CLOSE,
		SPSERIAL_INPUT_NULL_ERROR,
		SPSERIAL_THREAD_W32_CREATE,
		SPSERIAL_NOT_FOUND_IDD,
		SPSERIAL_REALLOC_ERROR,
		SPSERIAL_MALLOC_ERROR,
		SPSERIAL_INFO_NULL,
		SPSERIAL_PARAM_NULL,
		SPSERIAL_ITEM_NOT_FOUND,
		PSERIAL_CREATE_THREAD_ERROR,
		PSERIAL_SHUTDOWN_SOCK,
		PSERIAL_CLOSE_SOCK,
		PSERIAL_CREATE_SOCK,
		PSERIAL_FCNTL_SOCK,
		PSERIAL_BIND_SOCK,
		PSERIAL_EPOLL_CREATE,
		PSERIAL_EPOLL_CTL,
		PSERIAL_BUFF_EXCEED,
		PSERIAL_UNIX_OPEN_PORT,
		PSERIAL_UNIX_GET_ATTR,
		PSERIAL_UNIX_SET_ATTR,
		PSERIAL_UNIX_EPOLL_CTL,
		PSERIAL_PORTNAME_EXISTED,
		PSERIAL_HASH_NOTFOUND,
		



		SPSERIAL_PORT_PEAK,
	} SERIAL_PORT_ERR;

	typedef enum {
		SPSERIAL_EVENT_READ_BUF,
	} SPSERIAL_MODULE_EVENT;

	typedef struct __SP_SERIAL_GENERIC_ST__ {
		int total;
		int range;
		int pl;
		int pc;
		int type;
		char data[0];
	} SP_SERIAL_GENERIC_ST;
	
	
	typedef struct __SP_SERIAL_INPUT_ST__ {
		int
			baudrate;
		char
			port_name[SPSERIAL_PORT_LEN];
		SPSERIAL_module_cb
			cb_evt_fn;
		void* 
			cb_obj;

		int 
			t_delay;

	} SP_SERIAL_INPUT_ST;
	
	
	typedef struct __SP_SERIAL_INFO_ST__ {
		int
			t_delay;
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
#ifdef UNIX_LINUX
		//void* sem_trigger;    /*It need to wait for UNIX_LINUX.*/
#endif 
		SPSERIAL_module_cb
			cb_evt_fn;
		void* cb_obj;
		SP_SERIAL_GENERIC_ST*
			buff;

	} SP_SERIAL_INFO_ST;

	
	
	
	typedef struct __SPSERIAL_ARR_LIST_LINED__ {
		SP_SERIAL_INFO_ST* item;
		/* struct __SPSERIAL_ARR_LIST_LINED__* prev; */
		struct __SPSERIAL_ARR_LIST_LINED__* next;
	} SPSERIAL_ARR_LIST_LINED;

	
	
	
	typedef struct __SPSERIAL_ROOT_TYPE__ {
		//int n;
		int count;
		void* mutex;
		void* sem;
#ifndef UNIX_LINUX
#else
		int
			spsr_off;			/* Check off.*/
		void* 
			sem_spsr;			/* Check off.*/
		SP_SERIAL_GENERIC_ST*
			cmd_buff;			/* Command list .*/
	#ifndef __SPSR_EPOLL__
		char sem_key[SPSERIAL_KEY_LEN]; /*It need to wait for UNIX_LINUX --->>> Mach.*/
	#endif			
#endif 
		SPSERIAL_ARR_LIST_LINED* init_node;
		SPSERIAL_ARR_LIST_LINED* last_node;
	}SPSERIAL_ROOT_TYPE;

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
	spsr_inst_open(SP_SERIAL_INPUT_ST* input);

/*Close the COM port after using. Thread-safe.*/
DLL_API_SERIAL_MODULE int
	spsr_inst_close(char* portname);

/*Close the COM port after using. Thread-safe.*/
DLL_API_SERIAL_MODULE int
	spsr_inst_write(char* portname, char*data, int sz);

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#ifdef __cplusplus
}
#endif

/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#endif
