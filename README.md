# Async C/C++ I/O with COM/Serial Port Library

## Principle: Unix Philosophy
KISS: Keep it simple, stupid!

## Overview 
This library provides a multi-threaded, cross-platform solution for managing multiple COM/Serial ports. It supports Windows, Linux, and macOS while maintaining compatibility with all C/C++ versions from ANSI C89 to C++20. No external libraries are required; it solely relies on Win32/POSIX API and termios.

## Features
- **Multi-threaded (Thread-safe)**: Designed for concurrent access to multiple serial ports.
- **Cross-platform**: Works on Windows, Linux, and macOS.
- **Minimal dependencies**: No external libraries required.
- **ANSI C89 to C++20 compatible**: Ensuring broad compiler support.
- **Efficient and high-performance**: Optimized for real-time communication.

## API Reference

### 1. Initialize and Cleanup
#### `int spsr_module_init();`
- **Description**: Initializes the serial module.
- **Thread-safety**: Yes, but should be called at the start of the `main` function.
- **Return Value**: `0` on success, positive value on failure.

#### `int spsr_module_finish();`
- **Description**: Cleans up resources before exiting.
- **Thread-safety**: Yes, but should be called at the end of the `main` function.
- **Return Value**: `0` on success, positive value on failure.

### 2. Open and Close Serial Ports
#### `int spsr_inst_open(SP_SERIAL_INPUT_ST* input);`
- **Description**: Opens a COM/serial port.
- **Parameters**:
  - `SP_SERIAL_INPUT_ST* input`: Structure containing port name, baud rate, callback function, callback object, delayed time, and other settings.
- **Thread-safety**: Yes.
- **Return Value**: `0` on success, positive value on failure.

#### `int spsr_inst_close(const char* portname);`
- **Description**: Closes an open serial port.
- **Parameters**:
  - `portname`: The name of the COM port to close.
- **Thread-safety**: Yes.
- **Return Value**: `0` on success, positive value on failure.

### 3. Data Transmission
#### `int spsr_inst_write(const char* portname, const char* data, int size);`
- **Description**: Writes data to the specified serial port.
- **Parameters**:
  - `portname`: The name of the COM port.
  - `data`: Pointer to the data buffer.
  - `size`: Number of bytes to write.
- **Thread-safety**: Yes.
- **Return Value**: `0` on success, positive value on failure.

### 4. Additional Features (Planned)
- **Reading Data**: The sample is at **spsr_test_callback** in https://github.com/thuanalg/libserialmodule/blob/main/src/main.c, and **SPSERIAL_MODULE_EVENT**.
- **Status Check**: Please see error code **SPSERIAL_PORT_ERR**.
- **Error Handling Improvements**: Please see error code **SERIAL_PORT_ERR**.

## Build and Use
	1. Windows: Come to "src" and call "cmake ..."
		64 bit: cmake -G "Visual Studio 17 2022" -B ../build
		32 bit: cmake -G "Visual Studio 17 2022" -B ../build -A win32
	2. Linux: Come to "src/linux" and call "make debug" or "make release". I don't use "cmake" in this case.
	3. Mac OSX : Come to "src/mach" and call "make debug" or "make release". I don't use "cmake" in this case.

## Example Usage
Link: https://github.com/thuanalg/libserialmodule/blob/main/src/main.c

## License
Link: https://github.com/thuanalg/libserialmodule/blob/main/LICENSE.txt

## Contributions
Contributions are welcome! Feel free to submit issues or pull requests to enhance functionality.

## Reference & Dedication

   - "UNIX Network Programming, Volume 2: Interprocess Communications, Second Edition": https://www.amazon.com/UNIX-Network-Programming-Interprocess-Communications/dp/0130810819
   - "Unix Network Programming: The Sockets Networking API": https://www.amazon.com/Unix-Network-Programming-Sockets-Networking/dp/0131411551
   - My ex-colleagues: Lê Duy Cường, Bùi Khánh Duy, Nguyễn Công Đức , ... in my old company FPT (https://fpt.com/).