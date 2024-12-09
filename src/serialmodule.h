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

#ifdef __cplusplus
}
#endif

DLL_API_SERIAL_MODULE int 
	serial_module_init(void *);
DLL_API_SERIAL_MODULE int
	serial_module_close(void*);
#endif