# CtrLang Makefile
CC      = cc
CFLAGS  = -std=c11 -Wall -Wextra -O2 -g
LDFLAGS =

SRC_DIR   = src
KUT_DIR   = kutuphane
VENDOR    = vendor
BUILD_DIR = build

SRCS = $(wildcard $(SRC_DIR)/*.c) \
       $(wildcard $(KUT_DIR)/*.c) \
       $(wildcard $(VENDOR)/mongoose/*.c) \
       $(wildcard $(VENDOR)/sqlite/*.c) \
       $(wildcard $(VENDOR)/cjson/*.c) \
       $(wildcard $(VENDOR)/sds/*.c)

OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))

TARGET = ctrc

INC = -I$(SRC_DIR) -I$(KUT_DIR) \
      -I$(VENDOR)/mongoose -I$(VENDOR)/sqlite \
      -I$(VENDOR)/cjson -I$(VENDOR)/sds \
      -I$(VENDOR)/uthash

# Platform algılama
UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
    LDFLAGS += -lpthread
else ifeq ($(UNAME),Linux)
    LDFLAGS += -lpthread -ldl -lm
endif

# Termux (Android) algılama
ifneq ($(wildcard /data/data/com.termux),)
    CFLAGS += -D__TERMUX__=1
    LDFLAGS += -llog
endif

.PHONY: all clean test run ornek platform

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

test: all
	@cd test && ./calistir.sh

run: all
	./$(TARGET) ornekler/merhaba.ctr

ornek: all
	@./$(TARGET) ornekler/merhaba.ctr

platform: all
	@./$(TARGET) --platform

clean:
	rm -rf $(BUILD_DIR) $(TARGET)