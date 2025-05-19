cmake_minimum_required(VERSION 3.15)
project(serial_module_project)

# === Sources ===
set(SOURCE_LIB_MODULE_FILES src/serialmodule.c)
set(SOURCE_TEST_MODULE_FILES tests/console/main.c)
set(SOURCE_MFC_MODULE_FILES 
	tests/testMFC/testSerialPort/testSerialPort/framework.h
	tests/testMFC/testSerialPort/testSerialPort/pch.cpp
	tests/testMFC/testSerialPort/testSerialPort/pch.h
	tests/testMFC/testSerialPort/testSerialPort/resource.h
	tests/testMFC/testSerialPort/testSerialPort/targetver.h
	tests/testMFC/testSerialPort/testSerialPort/testSerialPort.cpp
	tests/testMFC/testSerialPort/testSerialPort/testSerialPort.h
	tests/testMFC/testSerialPort/testSerialPort/testSerialPort.rc
	tests/testMFC/testSerialPort/testSerialPort/testSerialPortDlg.cpp
	tests/testMFC/testSerialPort/testSerialPort/testSerialPortDlg.h
	tests/testMFC/testSerialPort/testSerialPort/res/testSerialPort.ico
	tests/testMFC/testSerialPort/testSerialPort/res/testSerialPort.rc2
)

# === Include main headers ===
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# === Define shared library ===
add_library(serialmodule SHARED ${SOURCE_LIB_MODULE_FILES})

# === Platform-specific defines ===
if(DEFINED UNIX_LINUX)
	target_compile_definitions(serialmodule PUBLIC UNIX_LINUX=1)
	if(DEFINED MACOSX)
		target_compile_definitions(serialmodule PUBLIC __MACH__=1)
	else()
		target_compile_definitions(serialmodule PUBLIC __SPSR_EPOLL__=1)
	endif()
	set_target_properties(serialmodule PROPERTIES VERSION 1.0.7 SOVERSION 1)
else()
	target_compile_definitions(serialmodule PUBLIC EXPORT_DLL_API_SERIAL_MODULE=1)
	target_compile_definitions(serialmodule PUBLIC _CRT_SECURE_NO_WARNINGS=1)
endif()

# === Link SimpleLog if not in embedded environment ===
if(NOT DEFINED META_OPENEMBEDDED)
	if(DEFINED HEADER_SIMPLELOG)
		message(STATUS "Using SimpleLog header at: ${HEADER_SIMPLELOG}")
		target_include_directories(serialmodule PUBLIC "${HEADER_SIMPLELOG}")
	else()
		message(FATAL_ERROR
			"\n[SimpleLog Missing]\n"
			"HEADER_SIMPLELOG is not defined.\n"
			"Please install SimpleLog-Topic from: https://github.com/thuanalg/simplelog-topic\n"
			"Then pass the header path using:\n"
			"  -DHEADER_SIMPLELOG=/path/to/simplelog/include"
		)
	endif()

	if(DEFINED LIB_PATH_SIMPLELOG)
		message(STATUS "Using SimpleLog library path: ${LIB_PATH_SIMPLELOG}")
		target_link_directories(serialmodule PUBLIC "${LIB_PATH_SIMPLELOG}")
	else()
		message(FATAL_ERROR
			"\n[SimpleLog Missing]\n"
			"LIB_PATH_SIMPLELOG is not defined.\n"
			"Please install SimpleLog-Topic from: https://github.com/thuanalg/simplelog-topic\n"
			"Then pass the library path using:\n"
			"  -DLIB_PATH_SIMPLELOG=/path/to/simplelog/lib"
		)
	endif()
endif()

# === Executable: test_serial_module (pure C) ===
add_executable(test_serial_module ${SOURCE_TEST_MODULE_FILES})
if(DEFINED UNIX_LINUX)
	target_compile_definitions(test_serial_module PUBLIC UNIX_LINUX=1)
else()
	target_compile_definitions(test_serial_module PUBLIC _CRT_SECURE_NO_WARNINGS=1 )
endif()

target_link_libraries(test_serial_module 
	simplelog
	serialmodule
)

# === Visual Studio only: include existing .vcxproj ===
if(MSVC)
	add_executable(testMFCSerialPort WIN32 ${SOURCE_MFC_MODULE_FILES})
	target_compile_definitions(testMFCSerialPort PUBLIC _CRT_SECURE_NO_WARNINGS=1 _AFXDLL=1)
	target_link_libraries(testMFCSerialPort 
		simplelog
		serialmodule
	)	
	# Enable MFC
	set_target_properties(testMFCSerialPort PROPERTIES
	    USES_MFC 2
	)	
endif()

# === Installation section for Unix/Linux ===
if(DEFINED UNIX_LINUX)
	install(TARGETS serialmodule LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
	install(FILES include/serialmodule.h DESTINATION include)
endif()

# === Link for serialmodule ===
target_link_libraries(serialmodule 
	simplelog
)
