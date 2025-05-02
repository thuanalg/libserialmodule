### Build and Use
1. Windows: Come to "src" and call "cmake ..."
	- Come to root folder, **mkdir build && cd build**
	- 64 bit: **cmake .. -G "Visual Studio 17 2022"**
	- 32 bit: **cmake .. -G "Visual Studio 17 2022" -A win32**
2. For Linux: **cmake .. -DUNIX_LINUX=1**.
3. Mac OSX : **cmake .. -DUNIX_LINUX=1 -DMACOSX=1**.
4. Mac OSX/Linux : **make; sudo make install**.
5. Note configuring file [simplelog.cfg](https://github.com/thuanalg/libserialmodule/blob/main/src/simplelog.cfg).

### Test on Windows:  
- After run "**cmake**", open **.sln**
- Add subproject [testSerialPort.vcxproj](https://github.com/thuanalg/libserialmodule/blob/main/tests/testMFC/testSerialPort/testSerialPort/testSerialPort.vcxproj), and build solution (note: build libraries first).
- Run and see [windows_test](https://github.com/thuanalg/libserialmodule/blob/main/tests/images/windows_test.png)  
### Test on Linux: 
- Please note baudrate of your system, example: **stty -F /dev/ttyUSB0** or set baudrate: **stty -F /dev/ttyUSB0 115200**;
- If you did install `gtk+-3.0`, come to **src/linux** and run **make clean; make**. Run [sudo ./zgtk.sh](https://github.com/thuanalg/libserialmodule/blob/main/src/linux/zgtk.sh)

- If you did NOT install `gtk+-3.0`, come to **src/linux** and edit [makefile](https://github.com/thuanalg/libserialmodule/blob/main/src/linux/Makefile), run **make clean; make**, and run [sudo ./ztest.sh](https://github.com/thuanalg/libserialmodule/blob/main/src/linux/ztest.sh)