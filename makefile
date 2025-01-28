trckr : main.c ./sqlite3/sqlite3.o
	gcc -o trckr \
		main.c trckr.c ./sqlite3/sqlite3.o ./arena/arena.c  \
		-I. -I./sqlite3 -I./arena \
		-ldl -pthread

./sqlite3/sqlite3.o :
	gcc -lpthread -ldl -o ./sqlite3/sqlite3.o -c ./sqlite3/sqlite3.c

clean :
ifeq ($(OS), Windows_NT)
	IF EXIST .\trckr DEL /F .\trckr
	IF EXIST .\sqlite3\sqlite3.o DEL /F .\sqlite3\sqlite3.o
else
	rm ./trckr ./sqlite3/sqlite3.o
endif