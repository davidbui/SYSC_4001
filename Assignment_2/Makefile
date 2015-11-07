CC=gcc
CFLAGS=-I.

assign2: producer.c helper.c
	$(CC) -o producer producer.c helper.c $(CFLAGS)
	$(CC) -o consumer consumer.c helper.c $(CFLAGS)

c:
	rm -rf *.o consumer producer
d:
	gcc -o consumer consumer.c -DDEBUG
	gcc -o producer producer.c -DDEBUG 
