CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude

SRC = src/client.c \
      src/helpers/listdir.c \
      src/helpers/file_metadata.c \
      src/helpers/send_data.c \
	  src/helpers/ui_client.c \
	  src/helpers/wire.c \
	  src/helpers/read_config.c

OBJ = $(SRC:.c=.o)

all: client

client: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

clean:
	rm -f client $(OBJ)
