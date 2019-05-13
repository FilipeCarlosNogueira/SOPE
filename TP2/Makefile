CFLAGS	=	-D_HPUX_SOURCE	-D_REENTRANT	-Wall	-Wextra	-Werror
OBJECTS1	=	server.o	log.o
OBJECTS2	=	user.o	log.o

all:	user	server

server: $(OBJECTS1)
	gcc $(OBJECTS1) -o server

user: $(OBJECTS2)
	gcc $(OBJECTS2) -o user


server.o:	sope.h	constants.h	types.h
user.o:	sope.h	constants.h	types.h
log.o:	sope.h	constants.h	types.h

clean:
	rm *.o server user