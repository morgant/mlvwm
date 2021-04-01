VERSION=\"0.9.3\"
CFLAGS = -g -Wall -DVERSION=$(VERSION) -fpcc-struct-return -DCONFIGNAME=\".mlvwmrc\" -DUSE_LOCALE -DMLVWMLIBDIR=\"/usr/X11R6/lib/X11/mlvwm\"
CC = gcc
LDFLAGS = -lXpm -lXext -lX11 
CPPFLAGS = -I/usr/X11R6/include
SRC = add_window.c balloon.c borders.c config.c \
      event.c functions.c menus.c misc.c mlvwm.c wild.c
OBJS = $(SRC:.c=.o)	
TARGET = mlvwm

all:$(TARGET)
$(TARGET):$$@.c $(OBJS)
	${CC} -o $@ $(OBJS) $(CFLAGS) $(LDFLAGS) $(LDLIBS)
clean:
	rm -f $(TARGET) *.o core *% *~ #*# 
clean.bak:
	rm -f core *% *~ #*#
