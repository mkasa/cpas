all: cpas

cpas: cpas.c
	gcc -O2 cpas.c -o cpas
