CC=gcc
OUTPUT=bin/main.out
INPUT=src/main.c

all: run

bin:
	mkdir $@

compile: $(INPUT) bin
	$(CC) -g -o $(OUTPUT) $(INPUT)

run: compile
	./$(OUTPUT)

memcheck: compile
	valgrind --tool=memcheck --leak-check=full -s $(output)

debug: compile
	gdb $(OUTPUT)