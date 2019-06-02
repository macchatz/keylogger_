all: a 

a: keylogger.c
	gcc keylogger.c -o keylogger
clean:
	rm -f *.o 
