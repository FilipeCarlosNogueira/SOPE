CFLAGS	=	-D_HPUX_SOURCE -Wall
OBJECTS	=	forensic.o	interface.o	file.o	variables.o
forensic: $(OBJECTS)
	gcc $(OBJECTS) -o forensic

forensic.o	interface.o:	interface.h	file.h	variables.h
file.o:	file.h	variables.h
variables.o:	variables.h