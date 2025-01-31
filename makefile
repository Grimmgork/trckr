trckr : ./arena/arena.c main.c trckr.c ./sqlite3/sqlite3.o
	gcc -o trckr \
		main.c trckr.c ./sqlite3/sqlite3.o ./arena/arena.c  \
		-I. -I./sqlite3 -I./arena

./sqlite3/sqlite3.o :
	gcc -o ./sqlite3/sqlite3.o -c ./sqlite3/sqlite3.c

clean :
	rm ./trckr ./trckr.exe ./sqlite3/sqlite3.o

test : trckr
	trckr