bin/ejecutable: obj/main.o 
	gcc -Wall -pthread obj/main.o -o bin/ejecutable
obj/main.o: src/main.c
	gcc -Wall  -c src/main.c -o obj/main.o  
.PHONY: clean
clean:
	rm bin/ejecutable  obj/*
