clear
clang main.c -o main.o -O2 -g -Wall -Wextra
if [ "$1" == "DEBUG" ]; then
    valgrind ./main.o
else
    ./main.o
fi
