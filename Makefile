.PHONY = build run

FILE = test

build:
	gcc main.c -o main -O2 -g -Wall -Wextra

run: build
	./main.o $(FILE)
