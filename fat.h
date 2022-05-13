#include <byteswap.h>
#include <linux/msdos_fs.h> // fat_boot_sector and msdos_dir_entry
#include <linux/types.h>
#include <stdlib.h> // u_int8_t

#ifdef DEBUG
#define _DEBUG(fmt, args...) printf("%s:%s:%d: " fmt, __FILE__, __FUNCTION__, __LINE__, args)
#else
#define _DEBUG(fmt, args...)
#endif
#ifndef FAT
#define FAT

#define SECS_PER_MIN 60
#define SECS_PER_HOUR (60 * 60)
#define SECS_PER_DAY (SECS_PER_HOUR * 24)
/* days between 1.1.70 and 1.1.80 (2 leap days) */
#define DAYS_DELTA (365 * 10 + 2)
/* 120 (2100 - 1980) isn't leap year */
#define YEAR_2100 120
#define IS_LEAP_YEAR(y) (!((y)&3) && (y) != YEAR_2100)
static long days_in_year[] = {
    /* Jan  Feb  Mar  Apr  May  Jun  Jul  Aug  Sep  Oct  Nov  Dec */
    0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 0, 0, 0,
};
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
 * 16 bytes of the sector printed out in hex form. First, in a line, the offset
 * of the first byte in the sequence is printed in hex form (using 8 hex
 * digits). Then, the sequence of 16 bytes are printed out in hex form (between
 * 2 hex digits we have a SPACE character). Then the same sequence of 16 bytes
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

/**
 * fat DISKIMAGE -a PATH: print the content of the ascii text file
 * indicated with PATH to the screen as it is. You can assume the specified
 * file contains printable ascii text characters. Where each line will end will
 * be dictated by the newline characters that may exist in the specified file.
 * Hence you will not worry about the line length. Just print the characters
 * (by using the %c format specifier) as they appear in the file. An example
 * output is below, which is the content of the file /DIR2/F1.TXT. We will
 * invoke this option only for ascii text files.
 * Command: ./fat disk1 -a /DIR2/F1.TXT
 * @param disk_image
 * @param path
 */
void print_a(char *disk_image, /*unsigned*/ char *path);

/**
 * fat DISKIMAGE -b PATH: print the content (byte sequence) of the file
 * indicated with PATH to the screen in hex form in the following format. The
 * file can be a binary file or an ascii file, does not matter. An example
 * output is shown below. Use the same format. It is the output for the file
 * /DIR2/F1.TXT (a portion of it).
 * Command: ./fat disk1 -b /DIR2/F1.TXT
 * @param disk_image
 * @param path
 */
void print_b(char *disk_image, /*u_*/ char *path);

/**
 * 11. fat DISKIMAGE -r PATH OFFSET COUNT: read COUNT bytes from the
 * file indicated with PATH starting at OFFSET (byte offset into the file) and
 * print the bytes read to the screen as shown in the example below, which
 * shows the output for 200 bytes from the file starting at offset 14 of the file
 * (the first byte of a file has offset 0). Use the same format. If OFFSET +
 * COUNT is greater that the filesize, just read till the end of the file and print
 * the respective content. Command: ./fat disk1 -r /FILE5.TXT 14 200
 * @param disk_image
 * @param path
 */
void print_r(char *disk_image, /*u_*/ char *path, int offset, int count);

/**
 * 9. fat DISKIMAGE -d PATH: print the content of the directory entry of the
 * file or directory indicated with PATH. Some information from the directory
 * entry will be printed out. An example output is shown below. Use the
 * same format and parameters.
 * Command: ./fat disk1 -d /FILE2.BIN
 * name = FILE2.BIN
 * type = FILE
 * firstcluster = 7
 * clustercount = 10
 * size(bytes) = 10240
 * date = 09-04-2022
 * time = 10:00
 */
void print_d(char *disk_image, char *path);

/**
 * fat DISKIMAGE -t: print all directories and their files and
 * subdirectories starting from the root directory, recursively, in a depth-first
 * search order. That means you will traverse the whole directory tree and
 * print the names of the files and directories encountered. An example
 * output is shown below. Use the same format. Each line gives the full
 * pathname of a file or directory. Whether the pathname is for a file or
 * directory is indicated with (f) or (d) in the beginning of the line.
 * Command: ./fat disk1 -t
 *
 * (f) /FILE1.BIN
 * (f) /FILE2.BIN
 * (f) /FILE3.BIN
 * (f) /FILE4.BIN
 * (f) /FILE5.TXT
 * (d) /DIR2
 * (d) /DIR2/.
 * (d) /DIR2/..
 * (f) /DIR2/F2.BIN
 * (f) /DIR2/F1.TXT
 * (d) /DIR1
 * (d) /DIR1/.
 * (d) /DIR1/..
 * (f) /DIR1/PROGRAM.C
 * (f) /DIR1/AFILE1.BIN
 * (d) /DIR1/DD1
 * (d) /DIR1/DD1/.
 * (d) /DIR1/DD1/..
 * (f) /DIR1/DD1/PFILE.BIN
 * (f) /DIR1/DD1/PF1.TXT
 * (f) /DIR1/DD1/PFILE2.BIN
 *
 */
void print_t(char *disk_image);

/**
 * fat DISKIMAGE -l PATH: print the names of the files and
 * subdirectories in the directory indicated with PATH. The pathname for the
 * root directory is “/”. In each line of output, print the name of a file or
 * directory (not pathname), its first cluster number, its size (in bytes), and
 * its date-time information. You will get the date and time information from
 * offset 22 (4 bytes) of a directory entry (or from the date, time field of the
 * msdos_dir_entry structure explained later). An example output is
 * shown below. Use the same format. Command: ./fat disk1 -l /DIR1
 *
 * (d) name=.            fcn=28  size(bytes)=0       date=09-04-2022:10:38
 * (d) name=..           fcn=0   size(bytes)=0       date=09-04-2022:10:38
 * (f) name=PROGRAM.C    fcn=32  size(bytes)=102     date=09-04-2022:10:40
 * (f) name=AFILE1.BIN   fcn=60  size(bytes)=51200   date=09-04-2022:10:48
 * (d) name=DD1          fcn=110 size(bytes)=0        date=10-04-2022:13:18
 *
 * @param disk_image
 * @param path
 */
void print_l(char *disk_image, char *path);

 /** 10. fat DISKIMAGE -f COUNT: print the content of the FAT table. The first
 * COUNT entries will be printed out. Each line will include a cluster number
 * (FAT index in decimal form) and the respective FAT entry content in
 * decimal form. Entry index will start from 0 (that means cluster numbers
 * will start from 0). If COUNT is -1, information about all entries will be
 * printed out. An example output is shown below (for the first 15 entries).
 * Use the same format. Command: ./fat disk1 -f 15
 * 0000000: EOF
 * 0000001: EOF
 * 0000002: EOF
 * 0000003: 4
 * 0000004: 5
 */
void print_f(char *disk_image, int count);

void print_n(char *disk_image, char *path) ;

void print_h();


void word_to_binary(unsigned long int num, char *binary);

void init(char *disk_image_path);

int readsector(int fd, unsigned char *buf,
               unsigned int snum); // given in assignment
int readcluster(int fd, unsigned char *buf,
                unsigned int cnum); // given in assignment
unsigned long int u8_to_ul(__u8 *arr, int length);
void print_content(u_char *content, unsigned long int offset);
void print_content_for_r(u_char *content, unsigned long int offset, int length);

int get_dentry(char *disk_image, char *path, struct msdos_dir_entry *result);
void trim_split_filename(const char *full_8_3_filename, char *filename, char *extension);
void toUpperCase(char *str);
int findUntilNext(char *result, char *str, char delimiter);

/**
 * return -1 on failure
 * return 1 on success
 */
int get_dentry_helper(int file_handle, struct msdos_dir_entry *result, struct msdos_dir_entry *cur_dentry,
                      char *remaining_path);
void print_d_helper(int fd, struct msdos_dir_entry *dep);
unsigned int readFAT(int file, u_int cnum);
#endif