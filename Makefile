CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -Iinclude/cjson -DDEBUG -g
LDFLAGS = -Llib -l:libcjson.a

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

TARGET = $(BIN_DIR)/myhttpd

all: $(TARGET) copy-www copy-config

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

copy-www: | $(BIN_DIR)
	cp -r www $(BIN_DIR)/

copy-config: | $(BIN_DIR)
	cp config.json $(BIN_DIR)/

.PHONY: all clean copy-www copy-config
