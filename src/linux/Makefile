target_1=libsimplelog.so
target_2=libspserial.so
debugFlag=-g -fPIC -Wall -shared 
releaseFlag=-fPIC -Wall -shared
LIBS+=-lpthread
LIBS_1+=-L./ -lsimplelog
LIBS_2+=-L./ -lspserial
src=../simplelog/simplelog.c 
src1=../serialmodule.c 
INC=-I../simplelog/ -I../
#CFLAGS=-ansi
CFLAGS=-std=gnu89 -D UNIX_LINUX
#CPPFLAGS=-std=c++98 
#CPPFLAGS=-std=c++98 -D UNIX_LINUX -D __UNIX_LINUX_CPP11_AND_NEWERS
CPPFLAGS=-D UNIX_LINUX -D __UNIX_LINUX_CPP11_AND_NEWERS__

debug:
	gcc ${debugFlag} -o ${target_1} ${src} ${LIBS} ${CFLAGS} ${INC}
	gcc ${debugFlag} -o ${target_2} ${src} ${LIBS} ${CFLAGS} ${LIBS_1}
	gcc -g -o test_main ../main.c ${LIBS} ${CFLAGS} ${LIBS_1} ${LIBS_2} ${INC}

release:
	gcc ${releaseFlag} -o ${target_1} ${src} ${LIBS} ${CFLAGS}
	gcc ${releaseFlag} -o ${target_2} ${src} ${LIBS} ${CFLAGS} ${LIBS_1}
	gcc -g -o test_main ../main.c ${LIBS} ${CFLAGS} ${LIBS_1} ${LIBS_2} ${INC}

clean:
	rm -rf ${target_1} ${target_2}
