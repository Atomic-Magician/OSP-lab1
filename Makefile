CC = gcc

all:
	${CC} lab1.c lab1_functions.c -o lab1 -ldl
	${CC} -fPIC -shared crc_plugin.c crc_funcs.c -o crc_plugin.so
