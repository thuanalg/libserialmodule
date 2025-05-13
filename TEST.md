### Test on Windows:  
- After running "**cmake**", open **.sln**
- Set **testMFCSerialPort** as default and run (note: copy **simplelog.dll** into the folder.)
- Run and see [windows_test](https://github.com/thuanalg/libserialmodule/blob/main/tests/images/windows_test.png)  

### Test on Linux: 
- Please note baudrate of your system, example: **stty -F /dev/ttyUSB0** or set baudrate: **stty -F /dev/ttyUSB0 115200**;
- If you did install `gtk+-3.0`, come to **src/linux** and run **make clean; make**. Run [sudo ./zgtk.sh](https://github.com/thuanalg/libserialmodule/blob/main/src/linux/zgtk.sh), [gtk_view](https://github.com/thuanalg/libserialmodule/blob/main/tests/images/linux_gtk.png).

- If you did NOT install `gtk+-3.0`, come to **src/linux** and edit [Makefile](https://github.com/thuanalg/libserialmodule/blob/main/src/linux/Makefile) (by removing **test_with_gtk** target), run **make clean; make**, and run [sudo ./ztest.sh](https://github.com/thuanalg/libserialmodule/blob/main/src/linux/ztest.sh)

### Test on MAC OSX:
- Come to **src/mach**.
- Same with Linux.

### Contact:
- [Email](mailto:nguyenthaithuanalg@gmail.com)
