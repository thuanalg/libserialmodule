#INCL = -I../ -I../simplelog

gcc `pkg-config --cflags --libs gtk+-3.0` -o gtk_app gtk_app.c ${INCL} ${LIBS} -I../ -I../simplelog -DUNIX_LINUX

#./gtk_app

