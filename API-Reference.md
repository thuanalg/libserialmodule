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
#### `int spsr_inst_open(SPSR_INPUT_ST* input);`
- **Description**: Opens a COM/serial port.
- **Parameters**:
  - `SPSR_INPUT_ST* input`: Structure containing port name, baud rate, callback function, callback object, delayed time, and other settings.
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

### 4. Reading Data through by callback function
- **Reading Data**: The sample is at [spsr_test_callback](https://github.com/thuanalg/libserialmodule/tree/main/tests/console/main.c), and **SPSR_MODULE_EVENT**.
- **Status Check**: Please see error code **SPSR_PORT_ERR**.
- **Error Handling Improvements**: Please see error code **SPSR_PORT_ERR**.
