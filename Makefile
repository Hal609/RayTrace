CC = gcc
CFLAGS = -g -lSDL2 -lm -lpthread -Isrc
SRC_DIR = src
OBJ_DIR = .
TARGET = main

# List of source files
SRCS = $(SRC_DIR)/sdl_setup.c $(SRC_DIR)/render_utils.c $(SRC_DIR)/render.c $(SRC_DIR)/vector.c main.c

# Rule for making the final target
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

.PHONY: clean

# Rule for cleaning the project
clean:
	rm -f $(TARGET)
