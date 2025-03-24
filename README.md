# Serial Port Management Library

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
- **Reading Data**: API to read data from the serial port.
- **Status Check**: API to check if a port is open.
- **Error Handling Improvements**: Define specific error codes for better debugging.

## Example Usage
```c
#include "spsr.h"

int main() {
    if (spsr_module_init() < 0) {
        printf("Failed to initialize serial module\n");
        return -1;
    }
    
    SP_SERIAL_INPUT_ST input = {"COM3", 115200, my_callback};
    if (spsr_inst_open(&input) < 0) {
        printf("Failed to open serial port\n");
        return -1;
    }
    
    const char* message = "Hello, Serial Port!";
    spsr_inst_write("COM3", message, strlen(message));
    
    spsr_inst_close("COM3");
    spsr_module_finish();
    
    return 0;
}
```

## License
This library is open-source and distributed under the MIT License.

## Contributions
Contributions are welcome! Feel free to submit issues or pull requests to enhance functionality.

