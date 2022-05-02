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

/**
 * @brief  fat DISKIMAGE -v: print some summary information about the
 * specified FAT32 volume DISKIMAGE. Most of the information is obtained
 * from the boot sector (sector #0). An example output is given below (use
 * the same format and parameters). Command: ./fat disk1 -v
 * @param disk_image
 *
 * @example output can be
 * File system type: FAT32
 * Volume label: CS342
 * Number of sectors in disk: 262144
 * Sector size in bytes: 512
 * Number of reserved sectors: 32
 * Number of sectors per FAT table: 1016
 * Number of FAT tables: 2
 * Number of sectors per cluster: 2
 * Number of clusters = 130048
 * Data region starts at sector: 2064
 * Root directory starts at sector: 2064
 * Root directory starts at cluster: 2
 * Disk size in bytes: 134217728 bytes
 * Disk size in Megabytes: 128 MB
 * Number of used clusters: 142
 * Number of free clusters: 129906
 */

void print_v(char *disk_image);
/**
 * fat DISKIMAGE -s SECTORNUM: print the content (byte sequence) of
 * the specified sector to screen in hex form. An example output is shown
 * below. Use the same format. Each line of output will have a sequence of
 * 16 bytes of the sector printed out in hex form. First, in a line, the offset of
 * the first byte in the sequence is printed in hex form (using 8 hex digits).
 * Then, the sequence of 16 bytes are printed out in hex form (between 2 hex
 * digits we have a SPACE character). Then the same sequence of 16 bytes
 * are also printed out with printable characters if possible. For that you will
 * use the isprint() function. If for a character isprint() returns true,
 * then you will print the character using %c format specifier in printf().
 * If the character (byte) is not printable, i.e., isprint() returns false, then
 * you print the character ‘.’ (dot) instead of the byte, again using the %c
 * format specifier in printf().
 *
 * Command: ./fat disk1 -s 0
 * @param disk_image
 * @param sectorNum "the specified sector"s index
 */
void print_s(char *disk_image, int sectorNum);

/**
 * fat DISKIMAGE -c CLUSTERNUM: print the content (byte sequence) of
 * the specified cluster to the screen in hex form. Note that cluster 0 and
 * cluster 1 do not exist. An example output (partially) is shown below (for
 * cluster #2). Use the same format. Command: ./fat disk1 -c 2
 * @param disk_image
 * @param clusterNum
 */
void print_c(char *disk_image, int clusterNum);


void word_to_binary(unsigned long int num, char *binary);
void init(char *disk_image_path);

int readsector(int fd, unsigned char *buf, unsigned int snum); // given in assignment
int readcluster(int fd, unsigned char *buf, unsigned int cnum);// given in assignment
unsigned long int u8_to_ul(__u8 *arr, int length);
void print_content(u_char *content, unsigned long int offset);

#endif