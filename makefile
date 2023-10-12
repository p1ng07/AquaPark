main: main.o
	cc *.o -g -o main

main.o: src/main.c
	cc -c src/main.c

run:
	./main

clean:
	rm -rf *.o