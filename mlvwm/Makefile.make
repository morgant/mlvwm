VERSION=\"0.6.2\"
CFLAGS = -g -Wall -DVERSION=$(VERSION) -fpcc-struct-return -DCONFIGNAME=\".mlvwmrc\"
CC = gcc
LDFLAGS = -lXpm -lXext -lX11 
CPPFLAGS = -I/usr/X11R6/include
SRC = add_window.c borders.c config.c event.c menus.c mlvwm.c functions.c\
	sound.c
OBJS = $(SRC:.c=.o)	
TARGET = mlvwm

all:$(TARGET)
$(TARGET):$$@.c $(OBJS)
	${CC} -o $@ $(OBJS) $(CFLAGS) $(LDFLAGS) $(LDLIBS)
clean:
	rm -f $(TARGET) *.o core *% *~ #*# 
clean.bak:
	rm -f core *% *~ #*#
