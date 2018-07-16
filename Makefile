NAME=dcpu16-as

SRC_DIR=src/
BUILD_DIR=build/
TESTS_DIR=tests/

FLAGS=-std=c11 -Wall -Wextra -Wno-sign-compare \
      -I./lib/ -I./include/
LIBS=

CXX=g++-7.3
CC=gcc

# -----------------------------------------------------

ifeq ($(OS), WINDOWS_NT)
    EXT=.exe
else
    EXT=
endif

BIN:=$(NAME)$(EXT)
BIN_DEBUG:=$(NAME).debug$(EXT)
BIN_TEST:=$(NAME).test$(EXT)

LEX_SRC:=$(shell find $(SRC_DIR) -type f -name '*.l')
BISON_SRC:=$(shell find $(SRC_DIR) -type f -name '*.y')
SRC:=$(shell find $(SRC_DIR) -type f -name '*.c') $(LEX_SRC:.l=.yy.c) $(BISON_SRC:.y=.tab.c)
OBJ:=$(SRC:$(SRC_DIR)%.c=$(BUILD_DIR)%.o)

INTERMEDIATES:=$(LEX_SRC:.l=.yy.c) $(BISON_SRC:.y=.tab.c) $(BISON_SRC:.y=.tab.h)

TEST_SRC:=$(shell find $(TESTS_DIR) -type f -name '*.c')
TEST_OBJ:=$(TEST_SRC:$(TESTS_DIR)%.c=$(BUILD_DIR)tests/%.o)

FLAGS+= -MT $@ -MMD -MP -MF $(dir $@).d/$(basename $(notdir $@)).d

default: release

release: FLAGS+= -O2
release: $(BIN)

debug: FLAGS+= -g -D _DEBUG
debug: $(BIN_DEBUG)

profile: FLAGS+= -pg
profile: debug

test: FLAGS+= -g -D _TESTING -I./lib/catch/ `pkg-config --cflags check`
test: LIBS+= `pkg-config --libs check`
test: $(BIN_TEST)
	./$(BIN_TEST)

clean:
	rm -f $(shell find $(SRC_DIR) -type f -name '*.yy.c')
	rm -f $(shell find $(SRC_DIR) -type f -name '*.tab.c')
	rm -f $(shell find $(SRC_DIR) -type f -name '*.tab.h')
	rm -rf $(BUILD_DIR)
	rm -f $(BIN) $(BIN_DEBUG) $(BIN_TEST)

.PHONY: release debug profile test clean

$(BIN): $(OBJ)
	$(warning $(SRC))
	$(warning $(OBJ))
	$(CC) $(FLAGS) $(OBJ) $(LIBS) -o $(BIN)
	rm -f $(INTERMEDIATES)

$(BIN_DEBUG): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) $(LIBS) -o $(BIN_DEBUG)
	rm -f $(INTERMEDIATES)

$(BIN_TEST): $(OBJ) $(TEST_OBJ)
	$(CC) $(FLAGS) $(OBJ) $(TEST_OBJ) $(LIBS) -o $(BIN_TEST)
	rm -f $(INTERMEDIATES)

$(SRC_DIR)%.tab.c $(SRC_DIR)%.tab.h: $(SRC_DIR)%.y
	bison --defines=$(SRC_DIR)$*.tab.h -o $(SRC_DIR)$*.tab.c $<

$(SRC_DIR)%.yy.c: $(SRC_DIR)%.l $(SRC_DIR)%.tab.h
	flex -o $@ $<

$(BUILD_DIR)%.o: $(SRC_DIR)%.c
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $@).d
	$(CC) -c $< $(FLAGS) -o $@

$(BUILD_DIR)tests/%.o: $(TESTS_DIR)%.c
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $@).d
	$(CC) -c $< $(FLAGS) -I./src -o $@

include $(shell find $(DEP_DIR) -type f -name '*.d')
