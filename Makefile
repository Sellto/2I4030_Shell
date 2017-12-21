#Create executable
all : main.o
		gcc main.o -o Rpi_Shell

main.o : main.c
		gcc -c main.c -o main.o

#Delete temporary files created during compile
clean :
		rm -rf *.o
