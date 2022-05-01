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
v:
	./fat disk1 -v
s:
	./fat disk1 -s 32
c:
	./fat disk1 -c 2
t:
	./fat disk1 -t
r:
	./fat disk1 -r /DIR2/F1.TXT 100 64
b:
	./fat disk1 -b /DIR2/F1.TXT
a:
	./fat disk1 -a /DIR2/F1.TXT
n:
	./fat disk1 -n /DIR1/AFILE1.BIN
m:
	./fat disk1 -m 100
f:
	./fat disk1 -f 50
d:
	./fat disk1 -d /DIR1/AFILE1.BIN
l:
	./fat disk1 -l /
l2:
	./fat disk1 -l /DIR2
h:
	./fat disk1 -h

val: all
	valgrind -s --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes ./app
hel: all
	valgrind --tool=helgrind ./app