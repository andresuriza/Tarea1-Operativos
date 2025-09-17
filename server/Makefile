CC = gcc
CFLAGS = -Wall -Wextra -pthread
LDFLAGS = -lm
TARGET = server
SRCS = server.c handle_client.c socket_helpers.c pqueue.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)