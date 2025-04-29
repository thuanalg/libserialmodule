project(serial_module_project)
set(SOURCE_LIB_MODULE_FILES src/serialmodule.c)
set(SOURCE_TEST_MODULR_FILES tests/console/main.c)
set(SOURCE_SIMPLELOG_LIB_FILES src/simplelog.c)	


include_directories(./include)
add_library(simplelog SHARED ${SOURCE_SIMPLELOG_LIB_FILES})
if(DEFINED UNIX_LINUX)
	target_compile_definitions(simplelog PUBLIC UNIX_LINUX=1)
	if(DEFINED MACOSX)
		target_compile_definitions(simplelog PUBLIC __MACH__=1)
	endif()	
	set_target_properties(simplelog PROPERTIES  
		VERSION 1.0.6 
		SOVERSION 1 )	
else()
	target_compile_definitions(simplelog PUBLIC EXPORT_DLL_API_SIMPLE_LOG=1)
	target_compile_definitions(simplelog PUBLIC _CRT_SECURE_NO_WARNINGS=1)
endif()

include_directories(./include)
add_library(serialmodule SHARED ${SOURCE_LIB_MODULE_FILES})
if(DEFINED UNIX_LINUX)
	target_compile_definitions(serialmodule PUBLIC UNIX_LINUX=1)
	if(DEFINED MACOSX)
		target_compile_definitions(simplelog PUBLIC __MACH__=1)
	else()
		target_compile_definitions(simplelog PUBLIC __SPSR_EPOLL__=1)	
	endif()		
	set_target_properties(serialmodule PROPERTIES  
		VERSION 1.0.0 
		SOVERSION 1 
	)
else()
	target_compile_definitions(serialmodule PUBLIC EXPORT_DLL_API_SERIAL_MODULE=1)
	target_compile_definitions(serialmodule PUBLIC _CRT_SECURE_NO_WARNINGS=1)
endif()




include_directories(./include)
add_executable(test_serial_module ${SOURCE_TEST_MODULR_FILES})
if(DEFINED UNIX_LINUX)
	target_compile_definitions(test_serial_module PUBLIC UNIX_LINUX=1)
else()
	target_compile_definitions(test_serial_module PUBLIC _CRT_SECURE_NO_WARNINGS=1)
endif()

if(DEFINED UNIX_LINUX)
	install(TARGETS simplelog LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
	install(FILES include/simplelog.h DESTINATION include)
	install(FILES src/simplelog.cfg DESTINATION include)

	install(TARGETS serialmodule LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
	install(FILES include/serialmodule.h DESTINATION include)

endif()

target_link_libraries(test_serial_module 
	simplelog
	serialmodule
)

target_link_libraries(serialmodule 
	simplelog
)
