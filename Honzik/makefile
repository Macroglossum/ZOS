CC = gcc
CFLAGS = -g

BIN = fs.exe
OBJ = main.o fs.o commandline.o constants.o 

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN) $(CFLAGS)

main.o: main.c
	gcc -c main.c $(CFLAGS)

fs.o: fs.c fs.h
	gcc -c fs.c $(CFLAGS)

commandline.o: commandline.c commandline.h
	gcc -c commandline.c $(CFLAGS)

constants.o: constants.c constants.h
	gcc -c constants.c $(CFLAGS)
