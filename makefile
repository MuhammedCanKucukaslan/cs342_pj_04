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
v: fat
	./fat disk1 -v
s: fat
	./fat disk1 -s 32
c: fat
	./fat disk1 -c 2 # equivalent to combining sector 2064 & 2065
t: fat
	./fat disk1 -t
r: fat
	./fat disk1 -r /DIR2/F1.TXT 100 64
b: fat
	./fat disk1 -b /DIR2/F1.TXT
a: fat
	./fat disk1 -a /DIR2/F1.TXT
n: fat
	./fat disk1 -n /DIR1/AFILE1.BIN
m: fat
	./fat disk1 -m 100
f: fat
	./fat disk1 -f 50
d: fat
	./fat disk1 -d /DIR1/AFILE1.BIN
l: fat
	./fat disk1 -l /
l2: fat
	./fat disk1 -l /DIR2
h: fat
	./fat disk1 -h
val: all
	valgrind -s --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes ./fat disk1 -v
hel: all
	valgrind --tool=helgrind ./fat disk1 -v