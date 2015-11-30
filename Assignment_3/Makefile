CFLAGS=-D_REENTRANT
LDFLAGS=-lpthread

assign3: main.c helper.c
	$(CC) -o main main.c helper.c $(CFLAGS)

c:
	rm -rf *.o main
d:
	gcc -o main main.c -DDEBUG
