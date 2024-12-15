#ifndef ___SIMPLE_SERIAL_MODULE__
#define ___SIMPLE_SERIAL_MODULE__

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

	typedef enum {
		SPSERIAL_PORT_OK,
		SPSERIAL_PORT_INFO_NULL,
		SPSERIAL_PORT_OPEN,
		SPSERIAL_PORT_COMMSTATE,
		SPSERIAL_PORT_GETCOMMSTATE,
		SPSERIAL_PORT_SETCOMMSTATE,
		SPSERIAL_PORT_CREATEEVENT,
		SPSERIAL_PORT_SETCOMMTIMEOUTS,
		SPSERIAL_PORT_SPSERIAL_MUTEX_CREATE,

		SPSERIAL_PORT_PEAK,
	} SERIAL_PORT_ERR;
	typedef struct __SP_SERIAL_INPUT_ST__ {
		int
			baudrate;
		char
			port_name[32];
	} SP_SERIAL_INPUT_ST;

DLL_API_SERIAL_MODULE int
	spserial_module_create(void*);

DLL_API_SERIAL_MODULE int
	spserial_module_setoff(int id);


#ifdef __cplusplus
}
#endif


#endif