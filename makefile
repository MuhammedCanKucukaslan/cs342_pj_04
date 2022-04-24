.PHONY : all fat
all: fat

fat: fat.c
	gcc -Wall -g -o fat fat.c -L.
clean:
	rm -fr *~ fat
tar:
	tar czf ../21901779.tar.gz fat.h fat.c makefile readme.txt report.pdf --ignore-failed-read

run: all
	./fat disk1 -v
	make clean 

val: all
	valgrind -s --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes ./app
hel: all
	valgrind --tool=helgrind ./app