all: *c *h
	gcc -Wall -pedantic *.c -o lockRanger

clean: lockRanger
	rm -f lockRanger