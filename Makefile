# CtrLang Makefile

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

.PHONY: all clean ornek platform kur test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

# Yardımcı hedefler
ornek: all
	@./$(TARGET) --token ornekler/merhaba.ctr

platform: all
	@./$(TARGET) --platform

kur: all
	@chmod +x kur.sh
	@./kur.sh

test: all
	@echo "Test: merhaba.ctr"
	@./$(TARGET) run ornekler/merhaba.ctr

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Tam temizlik (vendor objeleri dahil)
temizle:
	rm -rf $(BUILD_DIR) $(TARGET)
	rm -f *.ctr.c ornekler/*.ctr.c
	rm -f ornekler/merhaba ornekler/sayfa

# .PHONY tekrar
.PHONY: all clean ornek platform kur test temizle
