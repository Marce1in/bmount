CC := gcc

all: main.o parser.o
	$(CC) main.o parser.o -o bmount

debug: main.o parser.o
	$(CC) main.o parser.o -g -Wall -Wextra -pedantic -o bmount 

# main.o: main.c
# 	$(CC) -c main.c

# parser.o: parser.h parser.c
# 	$(CC) -c parser.h parser.c

%.o:%.c %.h
	$(CC) -c $^

install: all uninstall
	chmod +x bmount
	cp bmount /bin/

uninstall:
	rm -f /bin/bmount

clean:
	rm -f *.o *.gch bmount
