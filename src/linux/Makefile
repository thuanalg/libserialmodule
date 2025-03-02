target=libsimplelog.so
debugFlag=-g -fPIC -Wall -shared 
releaseFlag=-fPIC -Wall -shared
LIBS+=-lpthread
src=../simplelog/simplelog.c ../simplelog/simplelog.h
INC=-I../
#CFLAGS=-ansi
CFLAGS=-std=gnu89 -D UNIX_LINUX
#CPPFLAGS=-std=c++98 
#CPPFLAGS=-std=c++98 -D UNIX_LINUX -D __UNIX_LINUX_CPP11_AND_NEWERS
CPPFLAGS=-D UNIX_LINUX -D __UNIX_LINUX_CPP11_AND_NEWERS__

debug:
	gcc ${debugFlag} -o ${target} ${src} ${LIBS} ${CFLAGS}

release:
	gcc ${releaseFlag} -o ${target} ${src} ${LIBS} ${CFLAGS}

clean:
	rm -rf ${target}
