all: *c *h
	gcc -Wall -pedantic unixLockRanger.c -o lockRanger

clean: lockRanger
	rm -f lockRanger