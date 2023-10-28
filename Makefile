user2: user2.o librsocket.a user1
	gcc user2.o -L. -lrsocket -pthread -o user2

user1: user1.o librsocket.a
	gcc user1.o -L. -lrsocket -pthread -o user1

user2.o: user2.c rsocket.h
	gcc -c user2.c

user1.o:	user1.c rsocket.h
	gcc -c user1.c 


librsocket.a:	rsocket.o
		ar -rcs librsocket.a rsocket.o

rsocket.o:	rsocket.c rsocket.h
	gcc -c rsocket.c 

clean:
	rm user1 user2 user1.o user2.o librsocket.a rsocket.o
