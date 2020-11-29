all: *c *h
	gcc -Wall unixLockRanger.c -o lockRanger

clean: lockRanger
	rm -f lockRanger
