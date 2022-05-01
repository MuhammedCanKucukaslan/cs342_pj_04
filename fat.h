#include <byteswap.h>
#include <linux/types.h>
#include <stdlib.h>// u_int8_t

#ifdef DEBUG
#define _DEBUG(fmt, args...) printf("%s:%s:%d: " fmt, __FILE__, __FUNCTION__, __LINE__, args)
#else
#define _DEBUG(fmt, args...)
#endif
#ifndef FAT
#define FAT

// method declaration
void pln(char *input);
void print_v(char *disk_image);
void print_s(char *disk_image, int sectorNum);


void word_to_binary(unsigned long int num, char *binary);
void init(char *disk_image_path);

int readsector(int fd, unsigned char *buf, unsigned int snum); // given in assignment
int readcluster(int fd, unsigned char *buf, unsigned int cnum);// given in assignment
unsigned long int u8_to_ul(__u8 *arr, int length);
void print_content(u_char* content, unsigned long int offset );

#endif