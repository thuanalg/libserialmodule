# Add main.cpp file of the project root directory as a source file
# Add direct macro into cmake : https://stackoverflow.com/questions/8564337/how-can-i-define-a-c-preprocessor-macro-through-the-command-line-with-cmake
# cmake -DUNIX_LINUX=1 -D__MACH__=1
project(serial_module_project)
set(SOURCE_LIB_MODULE_FILES serialmodule.c serialmodule.h)
set(SOURCE_TEST_MODULR_FILES main.c)
set(SOURCE_SIMPLELOG_LIB_FILES simplelog/simplelog.c 
	simplelog/simplelog.h )	

# Add executable target with source files listed in SOURCE_FILES variable

#add_compile_definitions(EXPORT_DLL_API_SIMPLE_LOG simple_signal_slot_lib)
#add_compile_definitions(_CRT_SECURE_NO_WARNINGS simple_signal_slot_lib)
#include_directories(./ ./implement)
add_library(serial_module_lib SHARED ${SOURCE_LIB_MODULE_FILES})
target_compile_definitions(serial_module_lib PRIVATE EXPORT_DLL_API_SERIAL_MODULE=1)
target_compile_definitions(serial_module_lib PUBLIC _CRT_SECURE_NO_WARNINGS=1)
if(DEFINED UNIX_LINUX)
	target_compile_definitions(serial_module_lib PUBLIC UNIX_LINUX=1)
endif()

add_library(simpleloglib SHARED ${SOURCE_SIMPLELOG_LIB_FILES})
target_compile_definitions(simpleloglib PRIVATE EXPORT_DLL_API_SIMPLE_LOG=1)
target_compile_definitions(simpleloglib PUBLIC _CRT_SECURE_NO_WARNINGS=1)
if(DEFINED UNIX_LINUX)
	target_compile_definitions(simpleloglib PUBLIC UNIX_LINUX=1)
endif()


include_directories(./simplelog)
add_executable(test_serial_module ${SOURCE_TEST_MODULR_FILES})
target_compile_definitions(test_serial_module PUBLIC _CRT_SECURE_NO_WARNINGS=1)

include_directories(./simplelog)
target_link_libraries(test_serial_module 
	simpleloglib
	serial_module_lib
)

include_directories(./simplelog)
target_link_libraries(serial_module_lib 
	simpleloglib
)