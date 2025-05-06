#target_1=libsimplelog.so
target_2=libspserial.so
#debugFlag=-g -fPIC -Wall -shared 
debugFlag=-g -fPIC -shared 
#releaseFlag=-fPIC -Wall -shared
releaseFlag=-fPIC -shared
LIBS+=-lpthread -lsimplelog
#LIBS_1+=-L./ -lsimplelog
LIBS_2+=-L./ -lspserial
src=../simplelog.c 
src1=../serialmodule.c 
INC=-I../ -I../../include
src_console=../../tests/console/main.c
src_gtk=../../tests/gtk/gtk_app.c
#CFLAGS=-ansi
#CFLAGS=-std=gnu89 -D UNIX_LINUX -D__SPSR_EPOLL__ -Wunused-variable -Wunused-function -DSPSR_SHOW_CONSOLE -Werror=format
CFLAGS=-std=gnu89 -D UNIX_LINUX  -Wunused-variable -Wunused-function -DSPSR_SHOW_CONSOLE -Werror=format
#CPPFLAGS=-std=c++98 
#CPPFLAGS=-std=c++98 -D UNIX_LINUX -D __UNIX_LINUX_CPP11_AND_NEWERS
CPPFLAGS=-D UNIX_LINUX -D __UNIX_LINUX_CPP11_AND_NEWERS__ -DSPSR_SHOW_CONSOLE

debug:
#	gcc ${debugFlag} -o ${target_1} ${src} ${LIBS} ${CFLAGS} ${INC}
	gcc ${debugFlag} -o ${target_2} ${src1} ${LIBS} ${CFLAGS} ${INC}
	gcc -g -o test_main ${src_console} ${LIBS} ${CFLAGS} ${LIBS_1} ${LIBS_2} ${INC}
	gcc -g -o test_with_gtk ${src_gtk} ${LIBS} ${CFLAGS} ${LIBS_1} ${LIBS_2} ${INC} `pkg-config --cflags --libs gtk+-3.0`

release:
#	gcc ${releaseFlag} -o ${target_1} ${src} ${LIBS} ${CFLAGS} ${INC}
	gcc ${releaseFlag} -o ${target_2} ${src1} ${LIBS} ${CFLAGS} ${INC}
	gcc -o test_main ${src_console} ${LIBS} ${CFLAGS} ${LIBS_1} ${LIBS_2} ${INC}
	gcc -o test_with_gtk ${src_gtk} ${LIBS} ${CFLAGS} ${LIBS_1} ${LIBS_2} ${INC} `pkg-config --cflags --libs gtk+-3.0`

clean:
	rm -rf *.so test_main
#Set baudrate from BASH
#stty -F /dev/ttyUSB0 speed 115200
#get current baudrate from BASH
#stty -F /dev/ttyUSB0 speed
#stty -F /dev/ttyUSB0 -a
#stty -F /dev/ttyUSB0 115200
