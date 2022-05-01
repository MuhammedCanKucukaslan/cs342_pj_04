#ifndef FAT
#define FAT
#include <stdlib.h> // u_int8_t

// method declaration
void pln(char *input);
void print_v(char *disk_image);
void word_to_binary(unsigned long int num, char *binary);
void init(char* disk_image_path);

int readsector(int fd, unsigned char *buf, unsigned int snum);  // given in assignment
int readcluster(int fd, unsigned char *buf, unsigned int cnum); // given in assignment
unsigned long int u8_to_ul(__u8* arr, int length);
#endif