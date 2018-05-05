all:
	gcc -Wall -g -o fs fs.c format.c fs_helper_funcs/*.c

clean:
	rm fs