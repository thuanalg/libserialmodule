#ifndef ___SIMPLE_SERIAL_MODULE__
#define ___SIMPLE_SERIAL_MODULE__
#include <simplelog.h>

#define spserial_malloc(__nn__, __obj__, __type__) { (__obj__) = (__type__*) malloc(__nn__); if(__obj__) \
	{spllog(0, "Malloc: 0x%p\n", (__obj__)); memset((__obj__), 0, (__nn__));} \
	else {spllog(SPL_LOG_ERROR, "Malloc: error.\n");}} 
#define spserial_free(__obj__)   { if(obj) { spllog(0, "Free: %x", (__obj__)); free(__obj__); } }

#ifdef __cplusplus
extern "C" {
#endif

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
#endif /*! UNIX_LINUX */ 

#ifndef LLU
	#define LLU				unsigned long long
#endif
	typedef int (*SPSERIAL_module_cb)(void*);
	typedef enum {
		SPSERIAL_PORT_OK,
		SPSERIAL_PORT_INFO_NULL,
		SPSERIAL_PORT_INPUT_NULL,
		SPSERIAL_PORT_OPEN,
		SPSERIAL_PORT_COMMSTATE,
		SPSERIAL_PORT_GETCOMMSTATE,
		SPSERIAL_PORT_SETCOMMSTATE,
		SPSERIAL_PORT_CREATEEVENT,
		SPSERIAL_PORT_SETCOMMTIMEOUTS,
		SPSERIAL_PORT_SPSERIAL_MUTEX_CREATE,
		SPSERIAL_PORT_BAUDRATE_ERROR,
		SPSERIAL_PORT_NAME_ERROR,

		SPSERIAL_PORT_PEAK,
	} SERIAL_PORT_ERR;

	typedef enum {
		SPSERIAL_EVENT_READ_BUF,
	} SPSERIAL_MODULE_EVENT;

	typedef struct __SP_SERIAL_GENERIC_ST__ {
		int total;
		int pl;
		int pc;
		int type;
		char data[0];
	} SP_SERIAL_GENERIC_ST;
	typedef struct __SP_SERIAL_INPUT_ST__ {
		int
			baudrate;
		char
			port_name[32];
		SPSERIAL_module_cb
			cb;
	} SP_SERIAL_INPUT_ST;

DLL_API_SERIAL_MODULE int
	spserial_module_create(void*);

DLL_API_SERIAL_MODULE int
	spserial_module_setoff(int id);


#ifdef __cplusplus
}
#endif


#endif