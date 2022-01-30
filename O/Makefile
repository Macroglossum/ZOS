CC=gcc

all:	clean comp

comp:
	${CC} commands.c directory.c main.c superblock.c utils.c vfs.c -o zos_vfs -lpthread -lm -Wall


clean:
	rm -f zos_vfs
	rm -f *.*~
