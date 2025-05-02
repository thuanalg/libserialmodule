### Build and Use
1. Windows: Come to "src" and call "cmake ..."
	- Come to root folder, **mkdir build && cd build**
	- 64 bit: **cmake -G "Visual Studio 17 2022"**
	- 32 bit: **cmake -G "Visual Studio 17 2022" -A win32**
2. For Linux: **cmake .. -DUNIX_LINUX=1**.
3. Mac OSX : **cmake .. -DUNIX_LINUX=1 -DMACOSX=1**.
4. Mac OSX/Linux : **make; sudo make install**.
5. Note configuring file [simplelog.cfg](https://github.com/thuanalg/libserialmodule/blob/main/src/simplelog.cfg).