CC      = cc
CFLAGS  = -std=c11 -Wall -Wextra -O2 -g
LDFLAGS =

SRC_DIR   = src
KUT_DIR   = kutuphane
BUILD_DIR = build

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

TARGET = ctrc

INC = -I$(SRC_DIR) -I$(KUT_DIR)

# Platform algılama
UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
    LDFLAGS += -lpthread
else ifeq ($(UNAME),Linux)
    LDFLAGS += -lpthread -lm
endif

# Termux (Android)
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
	rm -rf $(BUILD_DIR) $(TARGET) *.c