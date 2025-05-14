# Async C/C++ I/O with COM/Serial Port Library

## Principle: Unix Philosophy
KISS: Keep it simple, stupid!

## Overview 
This library provides a multi-threaded, cross-platform solution for managing multiple COM/Serial ports. It supports Windows, Linux, and macOS while maintaining compatibility with all C/C++ versions from ANSI C89 to C++20. No external libraries are required; it solely relies on Win32/POSIX API and termios.

## Features
- **Multi-threaded (Thread-safe)**: Designed for concurrent access to multiple serial ports.
- **Cross-platform**: Works on Windows, Linux, and macOS.
- **ANSI C89 to C++20 compatible**: Ensuring broad compiler support.
- **Efficient and high-performance**: Optimized for real-time communication.

### [API Reference](https://github.com/thuanalg/libserialmodule/blob/main/API-Reference.md)


### Dependencies  
- Windows: [simplelog-topic](https://github.com/thuanalg/simplelog-topic)  
- Unix-Like(Linux/MacOSX): [simplelog-topic](https://github.com/thuanalg/simplelog-topic) + [BSD socket](https://linux.die.net/man/7/socket)  

### [INSTALL](https://github.com/thuanalg/libserialmodule/blob/main/INSTALL.md)  

### [TEST](https://github.com/thuanalg/libserialmodule/blob/main/TEST.md)  


## Example Usage
Link: [Example](https://github.com/thuanalg/libserialmodule/tree/main/tests/console/main.c)

## License
[License](https://github.com/thuanalg/libserialmodule/blob/main/LICENSE.txt)

## Contributions
Contributions are welcome! Feel free to submit issues or pull requests to enhance functionality.

## Reference & Dedication

   - [UNIX Network Programming, Volume 2: Interprocess Communications, Second Edition](https://www.amazon.com/UNIX-Network-Programming-Interprocess-Communications/dp/0130810819)
   - [Unix Network Programming: The Sockets Networking API](https://www.amazon.com/Unix-Network-Programming-Sockets-Networking/dp/0131411551)
   - My ex-colleagues: Lê Duy Cường, Bùi Khánh Duy, Nguyễn Công Đức , ... .
   - Jan Flik from [Intel](https://www.intel.com).

### Contact:
- [Email](mailto:nguyenthaithuanalg@gmail.com)

