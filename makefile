all: *c *h
	gcc -Wall lockRanger.c -o lockRanger

clean: lockRanger
	rm -f lockRanger
