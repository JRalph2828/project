CC=g++
CFLAGS=-g -O3 --std=c++11

run: project2.o
	$(CC) $(CFLAGS) $^ -o $@ -lpthread

%.o: %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@ -lpthread

clean:
	rm -f *.o run
	rm thread*
