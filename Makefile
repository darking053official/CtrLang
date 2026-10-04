CC      = cc
CFLAGS  = -std=c11 -Wall -Wextra -O2 -g -Wno-unused-parameter
LDFLAGS =

SRC_DIR   = src
BUILD_DIR = build

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

TARGET = ctrc
INC = -I$(SRC_DIR) -Ikutuphane \
      -Ivendor/mongoose -Ivendor/sqlite \
      -Ivendor/cjson -Ivendor/sds -Ivendor/uthash

UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
    LDFLAGS += -lpthread
else ifeq ($(UNAME),Linux)
    LDFLAGS += -lpthread -lm
endif

ifneq ($(wildcard /data/data/com.termux),)
    CFLAGS += -D__TERMUX__=1
endif

.PHONY: all clean ornek platform

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

ornek: all
	@./$(TARGET) --token ornekler/merhaba.ctr

platform: all
	@./$(TARGET) --platform

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
