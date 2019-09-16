# Minimal make file
CC = g++
CFLAGS = -O3 -std=c++11

OBJ = obj
FILES = obj/current.o obj/large-current.o

#FILES := $(patsubst %.cpp,$(OBJ)/%.o,$(SRCS))

all: bin/driver

bin/driver: obj/driver.o $(FILES)
	@mkdir -p `dirname $@`
	@echo "Linking $@..."
	@$(CC) -o $@ $^

# General object files
$(OBJ)/%.o: src/%.cpp
	@mkdir -p `dirname $@`
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $< -c -o $@

.PHONY : clean
clean:
	rm -r -f bin/* obj/
