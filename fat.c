#include "fat.h"
#include <fcntl.h>         // open()
#include <linux/msdos_fs.h>// fat_boot_sector and msdos_dir_entry
#include <stdio.h>         // printf()
//#include <stdlib.h>        // u_int8_t
#include <byteswap.h>
#include <ctype.h>
#include <linux/types.h>
#include <string.h>  // for strcmp() method
#include <sys/mman.h>// stat()
#include <unistd.h>  // and read()

// declare the constants
int data_start_sector;
const unsigned long content_length_per_line = 16;
#define RESERVED_SECTOR_COUNT 32
#define SECTOR_SIZE 512  // determined as such in last paragraph of page 1
#define CLUSTER_SIZE 1024// determined as such in the first line of page 2
#define MEGA_TO 1048576  // 1024*1024

const char directory_seperator = '/';
const int type_volume = 8;
const int type_directory = 16;
const int type_file = 32;

// global variable declaration
struct fat_boot_sector *fbs;
unsigned char num_fats, sectors_per_cluster;
unsigned int num_sectors, sectors_per_fat;
unsigned int root_start_cluster;
unsigned long int disk_size_in_bytes;


int main(int argc, char **argv)
{
    printf("%s : %d\n", argv[0], argc);

    if (argc < 2) {
        pln("Not enough argument is provided. fat -h");
    }
    // terrible condition
    if (argc == 2) {
        // todo print help
    }

    init(argv[1]);

    if (strcmp(argv[2], "-v") == 0) {
        pln("print info");
        print_v(argv[1]);
    } else if (strcmp(argv[2], "-s") == 0) {
        printf("print sector %d of disk %s\n", atoi(argv[3]), argv[1]);
        print_s(argv[1], atoi(argv[3]));//strtol( argv[3], num_end_ptr, 10 ));
    } else if (strcmp(argv[2], "-c") == 0) {
        printf("print cluster %d of disk %s\n", atoi(argv[3]), argv[1]);
        print_c(argv[1], atoi(argv[3]));//strtol( argv[3], num_end_ptr, 10 ));
    } else if (strcmp(argv[2], "-t") == 0) {
        printf("print -t %d of disk %s\n", atoi(argv[3]), argv[1]);
        // print_t(argv[3]);
    } else if (strcmp(argv[2], "-r") == 0) {
        printf("print -r %d of disk %s\n", atoi(argv[3]), argv[1]);
        // print_r(argv[3]);
    } else if (strcmp(argv[2], "-b") == 0) {
        printf("print -b %d of disk %s\n", atoi(argv[3]), argv[1]);
        //print_b(argv[1], argv[3]);
    } else if (strcmp(argv[2], "-a") == 0) {
        printf("print -a %d of disk %s\n", atoi(argv[3]), argv[1]);
        print_a(argv[1], argv[3]);
    } else if (strcmp(argv[2], "-n") == 0) {
        printf("print -n %d of disk %s\n", atoi(argv[3]), argv[1]);
        // print_n(argv[3]);
    } else if (strcmp(argv[2], "-m") == 0) {
        printf("print -m %d of disk %s\n", atoi(argv[3]), argv[1]);
        // print_m(argv[3]);
    } else if (strcmp(argv[2], "-f") == 0) {
        printf("print -f %d of disk %s\n", atoi(argv[3]), argv[1]);
        // print_f(argv[3]);
    } else if (strcmp(argv[2], "-d") == 0) {
        printf("print -d %d of disk %s\n", atoi(argv[3]), argv[1]);
        // print_d(argv[3]);
    } else if (strcmp(argv[2], "-l") == 0) {
        printf("print -l %d of disk %s\n", atoi(argv[3]), argv[1]);
        // print_l(argv[3]);
    } else if (strcmp(argv[2], "-l") == 0) {
        printf("print -l %d of disk %s\n", atoi(argv[3]), argv[1]);
        // print_l(argv[3]);
    } else if (strcmp(argv[2], "-h") == 0) {
        printf("print -h %d of disk %s\n", atoi(argv[3]), argv[1]);
        // print_h(argv[3]);
    }

    pln("after if elses of the command flags!");
    return 0;
}


/*
 * initialize basic variables, especially the fat_boot_sector fbs
 */
void init(char *disk_image_path)
{
    int file = open(disk_image_path, O_RDONLY);
    if (file == -1) {
        pln("Error opening file");
    }
    // read file
    u_int8_t sector[SECTOR_SIZE];
    read(file, sector, SECTOR_SIZE);

    fbs = (struct fat_boot_sector *) sector;// type casting
    // the following example shows how we can
    // access the related data from the sector
    sectors_per_cluster = fbs->sec_per_clus;
    num_sectors = fbs->total_sect;
    num_fats = fbs->fats;
    sectors_per_fat = fbs->fat32.length;
    root_start_cluster = fbs->fat32.root_cluster;
    data_start_sector = RESERVED_SECTOR_COUNT + num_fats * sectors_per_fat;
    disk_size_in_bytes = num_sectors * SECTOR_SIZE;
    close(file);
}

/*
 * @brief  fat DISKIMAGE -v: print some summary information about the
 * specified FAT32 volume DISKIMAGE.
 */
void print_v(char *disk_image)
{
    int todo = -1;
    unsigned char type[9];
    for (int i = 0; i < 8; i++) {
        type[i] = fbs->fat32.fs_type[i];
    }
    type[8] = '\0';
    unsigned char label[12];
    for (int i = 0; i < 11; i++) {
        label[i] = fbs->fat32.vol_label[i];
    }
    label[11] = '\0';

    printf("File system type: %s\n", type);// == NULL ? "FAT32" : "FAT16");
    printf("Volume label: %s\n", label);   //TODO
    printf("Number of sectors in disk: %d\n", num_sectors);
    printf("Sector size in bytes: %lu\n", u8_to_ul(fbs->sector_size, 2));
    printf("Number of reserved sectors: %d\n", fbs->reserved);
    printf("Number of sectors per FAT table: %d\n", fbs->fat32.length);
    printf("Number of FAT tables: %u\n", fbs->fats);
    printf("Number of sectors per cluster: %d\n", fbs->sec_per_clus);
    printf("Number of clusters = %d\n", (fbs->total_sect - (RESERVED_SECTOR_COUNT + num_fats * sectors_per_fat)) /
                                                fbs->sec_per_clus);// TODO doesn't match the example
    printf("Data region starts at sector: %d\n", data_start_sector);
    printf("Root directory starts at sector: %d\n", data_start_sector);
    printf("Root directory starts at cluster: %d\n", root_start_cluster);
    printf("Disk size in bytes: %lu bytes\n", disk_size_in_bytes);
    printf("Disk size in Megabytes: %lu MB\n", (disk_size_in_bytes / MEGA_TO));
    printf("Number of used clusters: %d\n", todo);// todo calculate
    printf("Number of free clusters: %d\n", todo);// todo calculate
}
/*
 * fat DISKIMAGE -s SECTORNUM: print the content (byte sequence) of
* the specified sector to screen in hex form.
 */
void print_s(char *disk_image, int sectorNum)
{
    if (sectorNum < 0) {
        printf("The sector number must be non negative but was %d\n", sectorNum);
        return;
    }
    int file = open(disk_image, O_RDONLY);
    if (file == -1) {
        printf("Error opening file for the fat's -s flag.\n");
        return;// terminate the method
    }

    // read file
    unsigned char buf[SECTOR_SIZE];
    if (readsector(file, buf, sectorNum) != 0) {
        printf("Error reading %dth sector in the file for the fat's -s flag.\n", sectorNum);
    } else {
        off_t offset;
        offset = sectorNum * SECTOR_SIZE;
        for (int i = 0; i < SECTOR_SIZE; i = i + (int) content_length_per_line) {
            print_content(&buf[i], offset + i);
        }
    }

    // close file
    close(file);
}


void print_c(char *disk_image, int clusterNum)
{
    if (clusterNum < 2) {
        printf("The cluster number should be greater or equal to 2 but was %d\n", clusterNum);
        return;
    }
    // open file
    int file = open(disk_image, O_RDONLY);
    if (file == -1) {
        printf("Error opening file for the fat's -s flag.\n");
        return;// terminate the method
    }

    // read file
    unsigned char buf[CLUSTER_SIZE];
    if (readcluster(file, buf, clusterNum) != 0) {
        printf("Error reading %dth cluster in the file for the fat's -s flag.\n", clusterNum);
    } else {
        off_t offset;
        unsigned int snum;// sector number
        snum = data_start_sector + (clusterNum - 2) * sectors_per_cluster;
        offset = snum * SECTOR_SIZE;
        for (int i = 0; i < CLUSTER_SIZE; i = i + (int) content_length_per_line) {
            print_content(&buf[i], offset + i);
        }
    }

    // close file
    close(file);
}
/**
 * @brief Convert the unsigned long to char array
 * in binary form
 *
 * @param num
 * @param binary out mode
 */
void word_to_binary(unsigned long int num, char *binary)
{
    char ch = 'z';
    int length = 64;
    for (int j = length - 1; j >= 0; j--) {
        // printf(" num: %d \n", num);
        ch = (num >> j & 1) == 1 ? '1' : '0';

        binary[length - 1 - j] = ch;
    }
}

/*
 * As given in assignment
 */
int readsector(int fd, unsigned char *buf, unsigned int snum)
{
    off_t offset;
    long n;
    offset = snum * SECTOR_SIZE;
    lseek(fd, offset, SEEK_SET);
    n = read(fd, buf, SECTOR_SIZE);
    if (n == SECTOR_SIZE)
        return (0);
    else
        return (-1);
}

/*
 * As given in assignment
 */
int readcluster(int fd, unsigned char *buf, unsigned int cnum)
{
    off_t offset;
    int n;
    unsigned int snum;// sector number
    snum = data_start_sector + (cnum - 2) * sectors_per_cluster;
    offset = snum * SECTOR_SIZE;
    lseek(fd, offset, SEEK_SET);
    n = read(fd, buf, CLUSTER_SIZE);
    if (n == CLUSTER_SIZE)
        return (0);// success
    else
        return (-1);
}

/**
 * Convert array of __8 to unsigned long int
 * @param arr
 * @param length
 * @return
 */
unsigned long int u8_to_ul(__u8 *arr, int length)
{
    ulong result = 0;
    //int base = 256; // 2^8
    int mult = 1;
    for (int i = 0; i < length; i++) {
        result += mult * arr[i];
        mult <<= 8;
    }
    return result;
}

void pln(char *input)
{
    printf("%s\n", input);
}

/**
 *
 * @param content exactly "16" char length of content
 * @param offset
 */
void print_content(u_char *content, unsigned long offset)
{
    printf("%.8lx ", offset);
    for (int i = 0; i < content_length_per_line; i++) {
        printf("%.2x ", content[i]);
    }
    for (int i = 0; i < content_length_per_line; i++) {
        printf("%c", isprint(content[i]) ? content[i] : '.');
    }
    putc('\n', stdout);
}