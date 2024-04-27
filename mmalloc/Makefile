
# makefile
VERSION = 1.0.0
CC = gcc
TARGET = test
DEBUG = -Wall -c -g

src_path = ./
src = $(foreach dir, $(src_path), $(wildcard $(dir)*.c))

obj_name = $(patsubst %.c, %.o, $(src))

INC = -I./

$(TARGET) : $(obj_name)
	$(CC) $(obj_name) -o $(TARGET)
	@echo "\nmakefile compile version :$(VERSION), target :$(TARGET)\n"

%.o : %.c
	$(CC) $(DEBUG) $(INC) $^ -o $@

.PHONY :
clean :
	@echo "start clean target and objects \n"
	rm -rf $(TARGET) $(obj_name)


