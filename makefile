trckr : main.c data.c ./sqlite3/sqlite3.o trckr.c data.c
	cc -o trckr main.c ./sqlite3/sqlite3.o -ldl -pthread

./sqlite3/sqlite3.o : 
	cc -lpthread -ldl -o ./sqlite3/sqlite3.o -c ./sqlite3/sqlite3.c

clean : 
	rm trckr ./sqlite3/sqlite3.o