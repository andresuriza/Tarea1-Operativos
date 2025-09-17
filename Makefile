CC = gcc
CFLAGS = -Wall -Wextra -pthread
LDFLAGS = -lm
TARGET = server/server
SRCDIR = server
SRCS = $(SRCDIR)/server.c \
       $(SRCDIR)/handle_client.c \
       $(SRCDIR)/socket_helpers.c \
       $(SRCDIR)/pqueue.c \
       $(SRCDIR)/read_config.c \
	   $(SRCDIR)/ConfigFunctions.c\
	   $(SRCDIR)/ImgFunciones.c


OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Cómo compilar los .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
