LIB_SUFF=so
#LIB_SUFF=dylib
MACOSX_FLAG=
#MACOSX_FLAG=-D __MACH__
target_1=libsimplelog.${LIB_SUFF}
target_2=libspserial.${LIB_SUFF}
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
CFLAGS=-std=gnu89 -D UNIX_LINUX -Wunused-variable -Wunused-function ${MACOSX_FLAG} 
#CPPFLAGS=-std=c++98 
#CPPFLAGS=-std=c++98 -D UNIX_LINUX -D __UNIX_LINUX_CPP11_AND_NEWERS
CPPFLAGS=-D UNIX_LINUX -D __UNIX_LINUX_CPP11_AND_NEWERS__ ${MACOSX_FLAG}

debug:
	gcc ${debugFlag} -o ${target_1} ${src} ${LIBS} ${CFLAGS} ${INC}
	gcc ${debugFlag} -o ${target_2} ${src1} ${LIBS} ${CFLAGS} ${LIBS_1} ${INC}
	gcc -g -o test_main ../main.c ${LIBS} ${CFLAGS} ${LIBS_1} ${LIBS_2} ${INC}

release:
	gcc ${releaseFlag} -o ${target_1} ${src} ${LIBS} ${CFLAGS} ${INC}
	gcc ${releaseFlag} -o ${target_2} ${src1} ${LIBS} ${CFLAGS} ${LIBS_1} ${INC}
	gcc -g -o test_main ../main.c ${LIBS} ${CFLAGS} ${LIBS_1} ${LIBS_2} ${INC}

clean:
	rm -rf ${target_1} ${target_2} *.dylib.* *_main* *.so
