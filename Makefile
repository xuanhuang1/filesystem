all:
	gcc -Wall -g -o fs fs.c format.c test.c fsCmds.c fs_helper_funcs/*.c

clean:
	rm fs