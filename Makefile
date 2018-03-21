target:
	clear
	gcc -g -o main -Wall -lpthread main.c

clean:
	rm main

run:
	./main