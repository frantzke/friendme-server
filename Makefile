PORT=52792
CFLAGS= -DPORT=\$(PORT) -g -Wall -std=c99 -Werror

friends_server: friends_server.o friendme.o friends.o 
	gcc $(CFLAGS) -o friends_server friends_server.o friendme.o friends.o

friends_server.o: friends_server.c friendme.h
	gcc $(CFLAGS) -c friends_server.c

friendme.o: friendme.c friendme.h
	gcc $(CFLAGS) -c friendme.c

friends.o: friends.c friends.h
	gcc $(CFLAGS) -c friends.c

clean: 
	rm friends_server *.o
