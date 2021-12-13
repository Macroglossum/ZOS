BIN = example.exe
OBJ = main.o
CC = gcc
CCFLAGS = -Wall -Wextra

$(BIN): $(OBJ)
	gcc $(OBJ) -o $(BIN) $(CCFLAGS)

main.o: main.c ext.h
	$(CC) -c main.c $(CCFLAGS)

clean:
	del *.o
	del *.exe
