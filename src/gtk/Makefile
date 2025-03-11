INCL=-I../ -I../simplelog
LIBS+=-lpthread -L../linux -lsimplelog -lspserial

all:
	gcc -g `pkg-config --cflags --libs gtk+-3.0` -o gtk_app gtk_app.c ${INCL} ${LIBS}  -DUNIX_LINUX

clean:
	rm gtk_app
#./gtk_app

