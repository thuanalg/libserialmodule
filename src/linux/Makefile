target_1=libsimplelog.so
target_2=libspserial.so
#debugFlag=-g -fPIC -Wall -shared 
debugFlag=-g -fPIC -shared 
#releaseFlag=-fPIC -Wall -shared
releaseFlag=-fPIC -shared
LIBS+=-lpthread
LIBS_1+=-L./ -lsimplelog
LIBS_2+=-L./ -lspserial
src=../simplelog/simplelog.c 
src1=../serialmodule.c 
INC=-I../simplelog/ -I../
#CFLAGS=-ansi
CFLAGS=-std=gnu89 -D UNIX_LINUX -D__SPSR_EPOLL__ -Wunused-variable -Wunused-function
#CPPFLAGS=-std=c++98 
#CPPFLAGS=-std=c++98 -D UNIX_LINUX -D __UNIX_LINUX_CPP11_AND_NEWERS
CPPFLAGS=-D UNIX_LINUX -D __UNIX_LINUX_CPP11_AND_NEWERS__

debug:
	gcc ${debugFlag} -o ${target_1} ${src} ${LIBS} ${CFLAGS} ${INC}
	gcc ${debugFlag} -o ${target_2} ${src1} ${LIBS} ${CFLAGS} ${LIBS_1} ${INC}
#	gcc -g -o test_main ../main.c ${LIBS} ${CFLAGS} ${LIBS_1} ${LIBS_2} ${INC}

release:
	gcc ${releaseFlag} -o ${target_1} ${src} ${LIBS} ${CFLAGS} ${INC}
	gcc ${releaseFlag} -o ${target_2} ${src1} ${LIBS} ${CFLAGS} ${LIBS_1} ${INC}
#	gcc -g -o test_main ../main.c ${LIBS} ${CFLAGS} ${LIBS_1} ${LIBS_2} ${INC}

clean:
	rm -rf ${target_1} ${target_2}
