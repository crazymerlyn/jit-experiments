default: build



build: main.o
	gcc main.o -ljit -o main

main.o: main.c
	gcc main.c -c -g -O2 -o main.o

run: build
	./main

clean:
	rm main main.o

