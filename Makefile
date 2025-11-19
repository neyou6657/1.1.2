# Makefile for Unified Crypto Interface (UCI)

CC = gcc
AR = ar
CFLAGS = -Wall -Wextra -std=c11 -O2 -fPIC
LDFLAGS = -shared

INCLUDE_DIR = include
SRC_DIR = src
BUILD_DIR = build
LIB_DIR = libs

INCLUDES = -I$(INCLUDE_DIR)

LIBOQS_DIR = $(LIB_DIR)/liboqs
GMSSL_DIR = $(LIB_DIR)/GmSSL

ifdef USE_LIBOQS
INCLUDES += -I$(LIBOQS_DIR)/include
LDFLAGS += -L$(LIBOQS_DIR)/lib -loqs
CFLAGS += -DHAVE_LIBOQS
endif

ifdef USE_GMSSL
INCLUDES += -I$(GMSSL_DIR)/include
LDFLAGS += -L$(GMSSL_DIR)/lib -lgmssl
CFLAGS += -DHAVE_GMSSL
endif

SOURCES = $(SRC_DIR)/unified_crypto_interface.c \
          $(SRC_DIR)/algorithm_registry.c \
          $(SRC_DIR)/classic_crypto_adapter.c \
          $(SRC_DIR)/pqc_adapter.c \
          $(SRC_DIR)/hybrid_crypto.c

OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

TARGET_LIB = $(BUILD_DIR)/libuci.so
TARGET_STATIC = $(BUILD_DIR)/libuci.a

.PHONY: all clean install examples tests

all: $(BUILD_DIR) $(TARGET_LIB) $(TARGET_STATIC)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(TARGET_LIB): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

$(TARGET_STATIC): $(OBJECTS)
	$(AR) rcs $@ $^

examples: $(TARGET_LIB)
	mkdir -p $(BUILD_DIR)/examples
	$(CC) $(CFLAGS) $(INCLUDES) examples/demo.c -o $(BUILD_DIR)/examples/uci_demo -L$(BUILD_DIR) -luci $(LDFLAGS)
	$(CC) $(CFLAGS) $(INCLUDES) examples/list_algorithms.c -o $(BUILD_DIR)/examples/uci_list_algorithms -L$(BUILD_DIR) -luci $(LDFLAGS)
	$(CC) $(CFLAGS) $(INCLUDES) examples/signature_demo.c -o $(BUILD_DIR)/examples/uci_signature_demo -L$(BUILD_DIR) -luci $(LDFLAGS)
	$(CC) $(CFLAGS) $(INCLUDES) examples/kem_demo.c -o $(BUILD_DIR)/examples/uci_kem_demo -L$(BUILD_DIR) -luci $(LDFLAGS)

tests: $(TARGET_LIB)
	mkdir -p $(BUILD_DIR)/tests
	$(CC) $(CFLAGS) $(INCLUDES) tests/test_basic.c -o $(BUILD_DIR)/tests/test_basic -L$(BUILD_DIR) -luci $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)

install: $(TARGET_LIB) $(TARGET_STATIC)
	install -d $(DESTDIR)/usr/local/lib
	install -m 644 $(TARGET_LIB) $(DESTDIR)/usr/local/lib/
	install -m 644 $(TARGET_STATIC) $(DESTDIR)/usr/local/lib/
	install -d $(DESTDIR)/usr/local/include
	install -m 644 $(INCLUDE_DIR)/*.h $(DESTDIR)/usr/local/include/

help:
	@echo "Unified Crypto Interface - Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all        - Build UCI library (default)"
	@echo "  examples   - Build example programs"
	@echo "  tests      - Build test programs"
	@echo "  clean      - Remove build artifacts"
	@echo "  install    - Install library and headers"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Options:"
	@echo "  USE_LIBOQS=1  - Enable LibOQS support"
	@echo "  USE_GMSSL=1   - Enable GmSSL support"
	@echo ""
	@echo "Example:"
	@echo "  make USE_LIBOQS=1 USE_GMSSL=1"
	@echo "  make examples USE_LIBOQS=1"
