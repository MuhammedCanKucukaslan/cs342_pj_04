#include "fat.h"
#include <byteswap.h>
#include <ctype.h>
#include <fcntl.h>          // open()
#include <linux/msdos_fs.h> // fat_boot_sector and msdos_dir_entry
#include <linux/types.h>
#include <stdio.h>    // printf()
#include <string.h>   // for strcmp() method
#include <sys/mman.h> // stat()
#include <time.h>
#include <unistd.h> // and read()

// declare the constants
int data_start_sector;
const unsigned long content_length_per_line = 16;
#define RESERVED_SECTOR_COUNT 32
#define SECTOR_SIZE 512   // determined as such in last paragraph of page 1
#define CLUSTER_SIZE 1024 // determined as such in the first line of page 2
// # number of Xs in 1 MegaX, where MEGA_TO is X invariant
#define MEGA_TO 1048576 // 1024*1024

#define EOF "--EOF--"
#define FRE "--FREE--"

const char directory_seperator = '/';
const int type_volume = 8;
const int type_directory = 16;
const int type_file = 32;

// global variable declaration
struct fat_boot_sector fbs;
unsigned char num_fats, sectors_per_cluster;
unsigned int num_sectors, sectors_per_fat, fat_start_sector;
unsigned int root_start_cluster;
unsigned long int disk_size_in_bytes;
unsigned char type[9];
unsigned char label[12];
u_long number_of_clus;
u_long used_cc;
u_long free_cc;
struct file_clus *file_list = NULL;
struct file_clus *tail = NULL;
struct file_clus
{
    char path[100];
    char name[100];
    uint startClus;
    struct file_clus *next;
};

int main(int argc, char **argv)
{
    // printf("%s : %d\n", argv[0], argc);

    if (argc < 2)
    {
        pln("Not enough argument is provided. fat -h");
    }
    // terrible condition
    if (argc == 2)
    {
        // todo print help
    }

    init(argv[1]);

    if (strcmp(argv[2], "-v") == 0)
    {
        // pln("print info");
        print_v(argv[1]);
    }
    else if (strcmp(argv[2], "-s") == 0)
    {
        // printf("print -s sector %d of disk %s\n", atoi(argv[3]), argv[1]);
        print_s(argv[1], atoi(argv[3])); // strtol( argv[3], num_end_ptr, 10 ));
    }
    else if (strcmp(argv[2], "-c") == 0)
    {
        // printf("print -c cluster %d of disk %s\n", atoi(argv[3]), argv[1]);
        print_c(argv[1], atoi(argv[3])); // strtol( argv[3], num_end_ptr, 10 ));
    }
    else if (strcmp(argv[2], "-t") == 0)
    {
        // printf("print -t %d of disk %s\n", atoi(argv[3]), argv[1]);
        print_t(argv[1]);
    }
    else if (strcmp(argv[2], "-r") == 0)
    {
        // printf("print -r %d of disk %s\n", atoi(argv[3]), argv[1]);
        // print_r(argv[3]);
    }
    else if (strcmp(argv[2], "-b") == 0)
    {
        // printf("print -b %d of disk %s\n", atoi(argv[3]), argv[1]);
        print_b(argv[1], argv[3]);
    }
    else if (strcmp(argv[2], "-a") == 0)
    {
        // printf("print -a %d of disk %s\n", atoi(argv[3]), argv[1]);
        print_a(argv[1], argv[3]);
    }
    else if (strcmp(argv[2], "-n") == 0)
    {
        // printf("print -n %d of disk %s\n", atoi(argv[3]), argv[1]);
        // print_n(argv[3]);
    }
    else if (strcmp(argv[2], "-m") == 0)
    {
        // printf("print -m %d of disk %s\n", atoi(argv[3]), argv[1]);
        print_m(argv[1], atoi(argv[3]));
    }
    else if (strcmp(argv[2], "-f") == 0)
    {
        // printf("print -f %d of disk %s\n", atoi(argv[3]), argv[1]);
        // print_f(argv[3]);
    }
    else if (strcmp(argv[2], "-d") == 0)
    {
        // printf("print -d %d of disk %s\n", atoi(argv[3]), argv[1]);
        print_d(argv[1], argv[3]);
    }
    else if (strcmp(argv[2], "-l") == 0)
    {
        // printf("print -l %d of disk %s\n", atoi(argv[3]), argv[1]);
        print_l(argv[1], argv[3]);
    }
    else if (strcmp(argv[2], "-h") == 0)
    {
        // printf("print -h %d of disk %s\n", atoi(argv[3]), argv[1]);
        // print_h(argv[3]);
    }

    // pln("after if elses of the command flags!");
    return 0;
}
void print_m_helper(char *disk_image, char *path)
{
    toUpperCase(path);
    int fd = open(disk_image, O_SYNC | O_RDONLY);
    if (fd == -1)
    {
        printf("Error opening disk %s for the fat's -a flag.\n", disk_image);
        return; // terminate the method
    }
    struct msdos_dir_entry dep1;
    struct msdos_dir_entry *dep;


    u_int8_t cluster[CLUSTER_SIZE];
    char pathdentry[100];
    strcpy(pathdentry, path);
    get_dentry(disk_image, pathdentry, &dep1);
    dep = &dep1;
    u_int cur_clu_no = dep->start + (dep->starthi << 16);
    readcluster(fd, cluster, cur_clu_no);
    dep = (struct msdos_dir_entry *)cluster;
    for (int i = 0; i < CLUSTER_SIZE / 32; ++i)
    {
        if (dep->name[0] != 0xe5 && dep->name[0] != 0x00) // e5 and 00 is for empty
        {
            if (dep->attr == 0x10)
            {
                if (i < 2)
                {
                }
                else
                {
                    char name[MSDOS_NAME];
                    char ext[MSDOS_NAME];
                    trim_split_filename(dep->name, name, ext);
                    char newpath[100];
                    strcpy(newpath, path);
                    strcat(newpath, "/");
                    strcat(newpath, name);
                    if (file_list == NULL)
                    {
                        struct file_clus *curr = malloc(sizeof(struct file_clus));
                        curr->startClus = dep->start | (dep->starthi << 16);
                        strcpy(curr->name, name);
                        strcpy(curr->path, path);
                        tail = curr;
                        file_list = tail;
                    }
                    else
                    {
                        struct file_clus *curr = malloc(sizeof(struct file_clus));
                        curr->startClus = dep->start | (dep->starthi << 16);
                        strcpy(curr->name, name);
                        strcpy(curr->path, path);
                        tail->next = curr;
                        tail = tail->next;
                    }


                    // printf("(d) name=%s/%-14s", path, name);
                    print_m_helper(disk_image, newpath);
                    pln("");
                }
            }
            if (dep->attr == 0x20)
            {
                char name[MSDOS_NAME];
                char ext[MSDOS_NAME];
                trim_split_filename(dep->name, name, ext);
                strcat(name, ".");
                strcat(name, ext);
                if (file_list == NULL)
                {
                    struct file_clus *curr = malloc(sizeof(struct file_clus));
                    curr->next = NULL;
                    curr->startClus = dep->start | (dep->starthi << 16);
                    strcpy(curr->name, name);
                    strcpy(curr->path, path);
                    tail = curr;
                    file_list = tail;
                }
                else
                {
                    struct file_clus *curr = malloc(sizeof(struct file_clus));
                    curr->next = NULL;
                    curr->startClus = dep->start | (dep->starthi << 16);
                    strcpy(curr->name, name);
                    strcpy(curr->path, path);
                    tail->next = curr;
                    tail = tail->next;
                }
                // printf("(f) name=%s/%-14s", path, name);
            }
        }
        ++dep;
    }
    struct file_clus *curr = file_list;
    while (curr != NULL)
    {
        while (1)
        {
            int currCluster;
            if (currCluster >= 0) // if end then break
            {
                break;
            }
        }
        curr = curr->next;
    }

    close(fd);
}

void print_m(char *disk_image, int count)
{
    char *path = "/";
    toUpperCase(path);
    int fd = open(disk_image, O_SYNC | O_RDONLY);
    if (fd == -1)
    {
        printf("Error opening disk %s for the fat's -a flag.\n", disk_image);
        return; // terminate the method
    }
    struct msdos_dir_entry dep1;
    struct msdos_dir_entry *dep;

    u_int8_t cluster[CLUSTER_SIZE];
    readcluster(fd, cluster, 2);
    dep = (struct msdos_dir_entry *)cluster;
    for (int i = 0; i < CLUSTER_SIZE / 32; ++i)
    {
        if (dep->name[0] != 0xe5 && dep->name[0] != 0x00) // e5 and 00 is for empty
        {
            if (dep->attr == 0x10)
            {
                char name[MSDOS_NAME];
                char ext[MSDOS_NAME];
                trim_split_filename(dep->name, name, ext);
                char newpath[100];
                strcpy(newpath, path);
                strcat(newpath, name);
                if (file_list == NULL)
                {
                    struct file_clus *curr = malloc(sizeof(struct file_clus));
                    curr->next = NULL;
                    curr->startClus = dep->start | (dep->starthi << 16);
                    strcpy(curr->name, name);
                    strcpy(curr->path, path);
                    tail = curr;
                    file_list = tail;
                }
                else
                {
                    struct file_clus *curr = malloc(sizeof(struct file_clus));
                    curr->next = NULL;
                    curr->startClus = dep->start | (dep->starthi << 16);
                    strcpy(curr->name, name);
                    strcpy(curr->path, path);
                    tail->next = curr;
                    tail = tail->next;
                }
                // printf("(d) name=%s%-14s", path, name);
                print_m_helper(disk_image, newpath);
            }
            if (dep->attr == 0x20)
            {
                char name[MSDOS_NAME];
                char ext[MSDOS_NAME];
                trim_split_filename(dep->name, name, ext);
                strcat(name, ".");
                strcat(name, ext);
                if (file_list == NULL)
                {
                    struct file_clus *curr = malloc(sizeof(struct file_clus));
                    curr->next = NULL;
                    curr->startClus = dep->start | (dep->starthi << 16);
                    strcpy(curr->name, name);
                    strcpy(curr->path, path);
                    tail = curr;
                    file_list = tail;
                }
                else
                {
                    struct file_clus *curr = malloc(sizeof(struct file_clus));
                    curr->next = NULL;
                    curr->startClus = dep->start | (dep->starthi << 16);
                    strcpy(curr->name, name);
                    strcpy(curr->path, path);
                    tail->next = curr;
                    tail = tail->next;
                }
                // printf("(f) name=%s%-14s", path, name);
            }
        }
        ++dep;
    }


    close(fd);
}
void print_l(char *disk_image, char *path)
{
    toUpperCase(path);
    int fd = open(disk_image, O_SYNC | O_RDONLY);
    if (fd == -1)
    {
        printf("Error opening disk %s for the fat's -a flag.\n", disk_image);
        return; // terminate the method
    }
    struct msdos_dir_entry dep1;
    struct msdos_dir_entry *dep;

    if (strcmp("/", path) == 0)
    {
        u_int8_t cluster[CLUSTER_SIZE];
        readcluster(fd, cluster, 2);
        dep = (struct msdos_dir_entry *)cluster;
        for (int i = 0; i < CLUSTER_SIZE / 32; ++i)
        {
            if (dep->name[0] != 0xe5 && dep->name[0] != 0x00) // e5 and 00 is for empty
            {
                int day, month, year, /*secs,*/ mins, hours;
                {
                    day = dep->date & 31;
                    month = ((dep->date >> 5) - 1) & 15;
                    year = dep->date >> 9;
                    // secs = (dep->time & 31) * 2;
                    mins = ((dep->time >> 5) & 63);
                    hours = (dep->time >> 11);
                    /* days since 1.1.70 plus 80's leap day */
                    // printf("date = %.2d-%.2d-%.4d\n", day, month, year + 1980); // 09-04-2022
                    // printf("time = %.2d:%.2d\n", hours, mins);                  // 10:00
                }
                if (dep->attr == 0x10)
                {
                    printf("(d) name=%-14s fcn=%6d size(bytes)=       0 date = %.2d-%.2d-%.4d time = %.2d:%.2d",
                           dep->name, (dep->starthi << 16 | dep->start), day, month, year + 1980, hours, mins);
                }
                if (dep->attr == 0x20)
                {
                    char name[MSDOS_NAME];
                    char ext[MSDOS_NAME];
                    trim_split_filename(dep->name, name, ext);
                    strcat(name, ".");
                    strcat(name, ext);
                    printf("(f) name=%-14s fcn=%6d size(bytes)=%8d date = %.2d-%.2d-%.4d time = %.2d:%.2d", name,
                           (dep->starthi << 16 | dep->start), dep->size, day, month, year + 1980, hours, mins);
                }
                pln("");
            }
            ++dep;
        }
    }
    else
    {
        u_int8_t cluster[CLUSTER_SIZE];
        get_dentry(disk_image, path, &dep1);
        dep = &dep1;
        u_int cur_clu_no = dep->start + (dep->starthi << 16);
        readcluster(fd, cluster, cur_clu_no);
        dep = (struct msdos_dir_entry *)cluster;
        for (int i = 0; i < CLUSTER_SIZE / 32; ++i)
        {
            if (dep->name[0] != 0xe5 && dep->name[0] != 0x00) // e5 and 00 is for empty
            {
                int day, month, year, /*secs,*/ mins, hours;
                {
                    day = dep->date & 31;
                    month = ((dep->date >> 5) - 1) & 15;
                    year = dep->date >> 9;
                    // secs = (dep->time & 31) * 2;
                    mins = ((dep->time >> 5) & 63);
                    hours = (dep->time >> 11);
                    /* days since 1.1.70 plus 80's leap day */
                    // printf("date = %.2d-%.2d-%.4d\n", day, month, year + 1980); // 09-04-2022
                    // printf("time = %.2d:%.2d\n", hours, mins);                  // 10:00
                }
                if (dep->attr == 0x10)
                {
                    if (i < 2)
                    {
                        printf("(d) name=%-14s fcn=%6d size(bytes)=       0 date = %.2d-%.2d-%.4d time = %.2d:%.2d",
                               dep->name, 0, day, month, year + 1980, hours, mins);
                    }
                    else
                    {
                        printf("(d) name=%-14s fcn=%6d size(bytes)=       0 date = %.2d-%.2d-%.4d time = %.2d:%.2d",
                               dep->name, (dep->starthi << 16 | dep->start), day, month, year + 1980, hours, mins);
                    }
                }
                if (dep->attr == 0x20)
                {


                    char name[MSDOS_NAME];
                    char ext[MSDOS_NAME];
                    trim_split_filename(dep->name, name, ext);
                    strcat(name, ".");
                    strcat(name, ext);
                    printf("(f) name=%-14s fcn=%6d size(bytes)=%8d date = %.2d-%.2d-%.4d time = %.2d:%.2d", name,
                           (dep->starthi << 16 | dep->start), dep->size, day, month, year + 1980, hours, mins);
                }
                pln("");
            }
            ++dep;
        }
    }
    close(fd);
}
void print_t_helper(char *disk_image, char *path)
{
    pln("");
    toUpperCase(path);
    int fd = open(disk_image, O_SYNC | O_RDONLY);
    if (fd == -1)
    {
        printf("Error opening disk %s for the fat's -a flag.\n", disk_image);
        return; // terminate the method
    }
    struct msdos_dir_entry dep1;
    struct msdos_dir_entry *dep;


    u_int8_t cluster[CLUSTER_SIZE];
    char pathdentry[100];
    strcpy(pathdentry, path);
    get_dentry(disk_image, pathdentry, &dep1);
    dep = &dep1;
    u_int cur_clu_no = dep->start + (dep->starthi << 16);
    readcluster(fd, cluster, cur_clu_no);
    dep = (struct msdos_dir_entry *)cluster;
    for (int i = 0; i < CLUSTER_SIZE / 32; ++i)
    {
        if (dep->name[0] != 0xe5 && dep->name[0] != 0x00) // e5 and 00 is for empty
        {
            if (dep->attr == 0x10)
            {
                if (i < 2)
                {
                    printf("(d) name=%s/%-14s", path, dep->name);
                }
                else
                {
                    char name[MSDOS_NAME];
                    char ext[MSDOS_NAME];
                    trim_split_filename(dep->name, name, ext);
                    char newpath[100];
                    strcpy(newpath, path);
                    strcat(newpath, "/");
                    strcat(newpath, name);
                    printf("(d) name=%s/%-14s", path, name);
                    print_t_helper(disk_image, newpath);
                    pln("");
                }
            }
            if (dep->attr == 0x20)
            {
                char name[MSDOS_NAME];
                char ext[MSDOS_NAME];
                trim_split_filename(dep->name, name, ext);
                strcat(name, ".");
                strcat(name, ext);
                printf("(f) name=%s/%-14s", path, name);
            }
            pln("");
        }
        ++dep;
    }
    close(fd);
}

void print_t(char *disk_image)
{
    char *path = "/";
    toUpperCase(path);
    int fd = open(disk_image, O_SYNC | O_RDONLY);
    if (fd == -1)
    {
        printf("Error opening disk %s for the fat's -a flag.\n", disk_image);
        return; // terminate the method
    }
    struct msdos_dir_entry dep1;
    struct msdos_dir_entry *dep;

    u_int8_t cluster[CLUSTER_SIZE];
    readcluster(fd, cluster, 2);
    dep = (struct msdos_dir_entry *)cluster;
    for (int i = 0; i < CLUSTER_SIZE / 32; ++i)
    {
        if (dep->name[0] != 0xe5 && dep->name[0] != 0x00) // e5 and 00 is for empty
        {
            if (dep->attr == 0x10)
            {
                char name[MSDOS_NAME];
                char ext[MSDOS_NAME];
                trim_split_filename(dep->name, name, ext);
                char newpath[100];
                strcpy(newpath, path);
                strcat(newpath, name);
                printf("(d) name=%s%-14s", path, name);
                print_t_helper(disk_image, newpath);
            }
            if (dep->attr == 0x20)
            {
                char name[MSDOS_NAME];
                char ext[MSDOS_NAME];
                trim_split_filename(dep->name, name, ext);
                strcat(name, ".");
                strcat(name, ext);
                printf("(f) name=%s%-14s", path, name);
                pln("");
            }
        }
        ++dep;
    }


    close(fd);
}

/*
 * initialize basic variables, especially the fat_boot_sector fbs
 */
void init(char *disk_image_path)
{
    int file = open(disk_image_path, O_RDONLY);
    if (file == -1)
    {
        pln("Error opening  disk");
    }
    // read file
    u_int8_t buf[SECTOR_SIZE];
    read(file, buf, SECTOR_SIZE);

    // we copy the content. "=" is another thing.
    memcpy(&fbs, (struct fat_boot_sector *)buf,
           sizeof(struct fat_boot_sector)); // type casting

    sectors_per_cluster = fbs.sec_per_clus;
    num_sectors = fbs.total_sect;
    fat_start_sector = RESERVED_SECTOR_COUNT;
    num_fats = fbs.fats;
    sectors_per_fat = fbs.fat32.length;
    root_start_cluster = fbs.fat32.root_cluster;
    data_start_sector = RESERVED_SECTOR_COUNT + num_fats * sectors_per_fat;
    disk_size_in_bytes = num_sectors * SECTOR_SIZE;

    number_of_clus = (sectors_per_fat * SECTOR_SIZE) / 4; /*size used to represent a cluster*/
    used_cc = 0;
    free_cc = 0;
    readsector(file, buf, fat_start_sector);

    u_int tmp;
    for (int i = 2; i < 2 + number_of_clus; ++i)
    {
        if (0 == i % (SECTOR_SIZE / 4))
        {
            readsector(file, buf, fat_start_sector + (int)(i / (SECTOR_SIZE / 4)));
            // printf("used: %lu; free %lu.\n") ;
        }
        // // should we include first two clusters?
        tmp = ((u_int *)buf)[i % (SECTOR_SIZE / 4)];
        //
        // printf("%u: %.8x\n", i, tmp);
        if (tmp == 0)
        {
            free_cc++;
        }
        else
        {
            used_cc++;
        }

        // print_content((u_char *)(buf)[4 * (i % SECTOR_SIZE)],i);
    }

    for (int i = 0; i < 8 && fbs.fat32.fs_type[i] != '\0'; i++)
    {
        type[i] = fbs.fat32.fs_type[i];
    }
    type[8] = '\0';

    for (int i = 0; i < 11 && fbs.fat32.vol_label[i] != '\0'; i++)
    {
        label[i] = fbs.fat32.vol_label[i];
    }
    label[11] = '\0';

    close(file);
}

/*
 * @brief  fat DISKIMAGE -v: print some summary information about the
 * specified FAT32 volume DISKIMAGE.
 */
void print_v(char *disk_image)
{
    printf("File system type: %s\n", type); // == NULL ? "FAT32" : "FAT16");
    printf("Volume label: %s\n", label);
    printf("Number of sectors in disk: %d\n", num_sectors);
    printf("Sector size in bytes: %lu\n", u8_to_ul(fbs.sector_size, 2));
    printf("Number of reserved sectors: %d\n", fbs.reserved);
    printf("Number of sectors per FAT table: %d\n", fbs.fat32.length);
    printf("Number of FAT tables: %u\n", num_fats);
    printf("Number of sectors per cluster: %d\n", sectors_per_cluster);
    // "the number of clusters that the file system can manage can be found as
    // follows: cluster_count = (sectors_per_FAT * 512 / 4)"
    // qtd. from the last 3 lines of the 3rd page  of the assignment
    printf("Number of clusters = %lu\n", number_of_clus);
    printf("Data region starts at sector: %d\n", data_start_sector);
    printf("Root directory starts at sector: %d\n", data_start_sector);
    printf("Root directory starts at cluster: %d\n", root_start_cluster);
    printf("Disk size in bytes: %lu bytes\n", disk_size_in_bytes);
    printf("Disk size in Megabytes: %lu MB\n", (disk_size_in_bytes / MEGA_TO));
    printf("Number of used clusters: %lu\n", used_cc);
    printf("Number of free clusters: %lu\n", free_cc);
}

/*
 * fat DISKIMAGE -s SECTORNUM: print the content (byte sequence) of
 * the specified sector to screen in hex form.
 */
void print_s(char *disk_image, int sectorNum)
{
    if (sectorNum < 0)
    {
        printf("The sector number must be non negative but was %d\n", sectorNum);
        return;
    }
    int file = open(disk_image, O_RDONLY);
    if (file == -1)
    {
        printf("Error opening  disk %s for the fat's -s flag.\n", disk_image);
        return; // terminate the method
    }

    // read file
    unsigned char buf[SECTOR_SIZE];
    if (readsector(file, buf, sectorNum) != 0)
    {
        printf("Error reading %dth sector in the file for the fat's -s flag.\n", sectorNum);
    }
    else
    {
        off_t offset;
        offset = sectorNum * SECTOR_SIZE;
        for (int i = 0; i < SECTOR_SIZE; i = i + (int)content_length_per_line)
        {
            print_content(&buf[i], offset + i);
        }
    }

    // close file
    close(file);
}

void print_c(char *disk_image, int clusterNum)
{
    if (clusterNum < 2)
    {
        printf("The cluster number should be greater or equal to 2 but was %d\n", clusterNum);
        return;
    }
    // open file
    int file = open(disk_image, O_RDONLY);
    if (file == -1)
    {
        printf("Error opening  disk %s for the fat's -c flag.\n", disk_image);
        return; // terminate the method
    }

    // read file
    unsigned char buf[CLUSTER_SIZE];
    if (readcluster(file, buf, clusterNum) != 0)
    {
        printf("Error reading %dth cluster in the file for the fat's -s flag.\n", clusterNum);
    }
    else
    {
        off_t offset;
        unsigned int snum; // sector number
        snum = data_start_sector + (clusterNum - 2) * sectors_per_cluster;
        offset = snum * SECTOR_SIZE;
        for (int i = 0; i < CLUSTER_SIZE; i = i + (int)content_length_per_line)
        {
            print_content(&buf[i], offset + i);
        }
    }

    // close file
    close(file);
}

void print_a(char *disk_image, char *path)
{
    toUpperCase(path);
    // pln(path);

    struct msdos_dir_entry dirEntry;

    if (-1 == get_dentry(disk_image, path, &dirEntry))
    {
        printf("The file \"%s\" could not be found!", path);
    }
    else
    {
        int fd = open(disk_image, O_RDONLY);
        if (fd == -1)
        {
            printf("Error opening disk %s for the fat's -a flag.\n", disk_image);
            return; // terminate the method
        }
        // print_d_helper(fd, &dirEntry);

        // pln("Start writing the file content!");
        // pln("----------------------------");
        long int rem_size = dirEntry.size;
        u_int cur_clu_no = dirEntry.start + (dirEntry.starthi << 16);
        u_char buf[CLUSTER_SIZE];
        while (rem_size > 0)
        {
            /*
             * A FAT table entry that is equal or bigger than
             * 0x0FFFFFF8 indicates the end of a cluster chain (end of file) (EOC or
             * EOF), for a file or directory. The value 0x0FFFFFF7 is bad cluster
             * mark.
             */
            if (cur_clu_no >= 0x0FFFFFF7)
            {
                printf("\nEnd of the cluster, or bad cluster: %#x.\n", cur_clu_no);
                break;
            }
            readcluster(fd, buf, cur_clu_no);
            if (rem_size < CLUSTER_SIZE)
            {
                buf[rem_size] = '\0';
                rem_size = 0;
            }
            else
            {
                rem_size = rem_size - CLUSTER_SIZE;
                cur_clu_no = readFAT(fd, cur_clu_no);
            }
            printf("%s", buf);
        }
        // pln("----------------------------");
        close(fd);
    }
}

void print_b(char *disk_image, char *path)
{
    toUpperCase(path);
    // pln(path);

    struct msdos_dir_entry dirEntry;

    if (-1 == get_dentry(disk_image, path, &dirEntry))
    {
        printf("The file \"%s\" could not be found!", path);
    }
    else
    {
        int fd = open(disk_image, O_RDONLY);
        if (fd == -1)
        {
            printf("Error opening disk %s for the fat's -a flag.\n", disk_image);
            return; // terminate the method
        }
        // print_d_helper(fd, &dirEntry);

        // pln("Start writing the file content!");
        // pln("----------------------------");
        off_t address;
        unsigned int snum; // sector number
        u_char content[content_length_per_line + 1];

        long int rem_size = dirEntry.size;
        u_int cur_clu_no = dirEntry.start + (dirEntry.starthi << 16);
        u_char buf[CLUSTER_SIZE];
        while (rem_size > 0)
        {
            int offset = 0;
            /*
             * A FAT table entry that is equal or bigger than
             * 0x0FFFFFF8 indicates the end of a cluster chain (end of file) (EOC or
             * EOF), for a file or directory. The value 0x0FFFFFF7 is bad cluster
             * mark.
             */
            if (cur_clu_no >= 0x0FFFFFF7)
            {
                printf("\nEnd of the cluster, or bad cluster: %#x.\n", cur_clu_no);
                break;
            }
            readcluster(fd, buf, cur_clu_no);
            if (rem_size < CLUSTER_SIZE)
            {
                buf[rem_size] = '\0';
                rem_size = 0;
            }
            else
            {
                rem_size = rem_size - CLUSTER_SIZE;
                cur_clu_no = readFAT(fd, cur_clu_no);
            }

            snum = data_start_sector + (cur_clu_no - 2) * sectors_per_cluster;
            address = snum * SECTOR_SIZE;
            while (offset * content_length_per_line < CLUSTER_SIZE)
            {
                memcpy(content, &buf[offset * content_length_per_line], content_length_per_line);
                content[content_length_per_line] = '\0';
                print_content(content, address + offset * content_length_per_line);
                offset++;
            }
        }
        // pln("----------------------------");
        close(fd);
    }
}

void print_d(char *disk_image, char *path)
{
    toUpperCase(path);
    struct msdos_dir_entry dirEntry;
    if (-1 == get_dentry(disk_image, path, &dirEntry))
    {
        printf("The file \"%s\" could not be found!", path);
    }
    else
    {
        int fd = open(disk_image, O_RDONLY);
        if (fd == -1)
        {
            printf("Error opening disk %s for the fat's -a flag.\n", disk_image);
            return; // terminate the method
        }
        print_d_helper(fd, &dirEntry);
        close(fd);
    }
}
// ############################################################################
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
    for (int j = length - 1; j >= 0; j--)
    {
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
    unsigned int snum; // sector number
    snum = data_start_sector + (cnum - 2) * sectors_per_cluster;
    offset = snum * SECTOR_SIZE;
    lseek(fd, offset, SEEK_SET);
    n = read(fd, buf, CLUSTER_SIZE);
    if (n == CLUSTER_SIZE)
        return (0); // success
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
    // int base = 256; // 2^8
    int mult = 1;
    for (int i = 0; i < length; i++)
    {
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
    for (int i = 0; i < content_length_per_line; i++)
    {
        printf("%.2x ", content[i]);
    }
    for (int i = 0; i < content_length_per_line; i++)
    {
        printf("%c", isprint(content[i]) ? content[i] : '.');
    }
    putc('\n', stdout);
}

int get_dentry(char *disk_image, char *path, struct msdos_dir_entry *result)
{
    // open file
    int file = open(disk_image, O_RDONLY);
    if (file == -1)
    {
        printf("Error opening  disk %s for the fat's -s flag.\n", disk_image);
        return -1; // terminate the method
    }
    // eliminate root directory indicator "/"
    path = path[0] == directory_seperator ? &path[1] : path;
    // printf("PATH passed to get_dentry: #%s#\n", path);
    struct msdos_dir_entry *dep; // dir entry pointer
    // read file
    // get the root directory's entry
    unsigned char buf[CLUSTER_SIZE];
    if (readcluster(file, buf, 2) != 0)
    {
        printf("Error reading 2nd cluster in the file for the fat's -a flag.\n");
        return -1;
    }
    dep = (struct msdos_dir_entry *)(buf);
    get_dentry_helper(file, result, dep, path);
    // print_d_helper(result);
    close(file);
    return 1;
}

/*
 * return -1 on failure
 * retrun 1 on success
 */
int get_dentry_helper(int file_handle, struct msdos_dir_entry *result, struct msdos_dir_entry *cur_dentry,
                      char *remaining_path)
{
    if (cur_dentry == NULL)
    {
        printf("Cur_dentry was null, impossible!\n");
    }
    u_long rp_len = strlen(remaining_path);
    if (rp_len < 1)
    {
        memcpy(result, cur_dentry, sizeof(struct msdos_dir_entry));
        return 1;
    }
    else
    {
        // check if remaining_part is the "the entry"
        char search_entry_name[12]; // 11 would be sufficient for 8.3 filenames but
                                    // nevertheless better to be sorry than safe
        search_entry_name[11] = '\0';
        struct msdos_dir_entry *dep; // dir entry pointer
        if (-1 == findUntilNext(search_entry_name, remaining_path, directory_seperator))
        {
            remaining_path = "";
        }
        // pln(search_entry_name);
        // pln("---------------------");
        // pln(remaining_path);
        // remaining_path is the entry
        int dir_cluster_no = root_start_cluster;
        if (cur_dentry->attr != type_volume)
        { // if it is the root it starts from cluster 2 but
          // corresponding attributes are 0!
            dir_cluster_no = cur_dentry->start + (cur_dentry->starthi << 16);
        }
        unsigned char buf[CLUSTER_SIZE];

        readcluster(file_handle, buf, dir_cluster_no);
        dep = (struct msdos_dir_entry *)(buf);

        int i = 0;
        /*
         * A directory entry that starts with byte value 0xe5 is
         * empty, i.e., is available (was used earlier and then deleted). It can be
         * used for a new file that is created. An entry that starts with byte value
         * 0x00 is also an unused entry.
         */
        while (dep[i].name[0] != 0)
        {
            if (dep[i].name[0] == 229 /* 0xe5 */)
            {
                // printf("Yet another emptied d_entry when i=%d\n", i);
            }
            else
            {
                // if the current dep belong to a dir or volume check the name
                char dep_sname[12] = "\0";
                char n[9], ext[4];
                trim_split_filename((char *)dep[i].name, n, ext);
                strcat(dep_sname, n);
                // if(dep[i].attr == type_volume  ||  dep[i].attr == type_directory )
                if (dep[i].attr == type_file)
                {
                    strcat(dep_sname, ".");
                    strcat(dep_sname, ext);
                }
                if (strcmp(dep_sname, search_entry_name) == 0)
                {
                    // printf("\nFound the match!\n");
                    // printf("dep_sname:%s: search_entry_name:%s: \n", dep_sname,
                    // search_entry_name);
                    memcpy(cur_dentry, &dep[i], sizeof(struct msdos_dir_entry));
                    // print_d_helper(cur_dentry);
                    // print_d_helper(&dep[i]);
                    // pln("\nRemainingPath:");
                    // pln(remaining_path);
                    return get_dentry_helper(file_handle, result, cur_dentry, remaining_path);
                }
                // printf("%s%c%s\ndep_sname:%s: search_entry_name:%s: ", n, dep[i].attr
                // == type_file ? '.' : '\0', ext,dep_sname,search_entry_name);
            }
            i++;
        }
        //}
    }
    return -1;
}

void trim_split_filename(const char *full_8_3_filename, char *filename, char *extension)
{
    int name_length = 8;

    // trim file name and copy
    int start = 0;
    int end = name_length - 1;
    while (full_8_3_filename[start] == ' ')
    {
        start++;
    }
    while (full_8_3_filename[end] == ' ')
    {
        end = end - 1;
    }
    memcpy(filename, &full_8_3_filename[start], end - start + 1);
    filename[1 + end - start] = '\0';

    // EXTRACT EXTENSION
    start = name_length;
    end = name_length + 3;
    while (full_8_3_filename[start] == ' ')
    {
        start++;
    }
    while (full_8_3_filename[end] == ' ')
    {
        end = end - 1;
    }
    memcpy(extension, &full_8_3_filename[start], end - start + 1);
    extension[1 + end - start] = '\0';
    // pln("\n***");
    // pln(filename);
    // pln(extension);
    // pln("***");
}

void toUpperCase(char *str)
{
    for (u_long i = strlen(str); i > 0; i--)
    {
        if (str[i] > 'a' && str[i] <= 'z')
        {
            str[i] = (char)toupper(str[i]);
        }
    }
}

/*
 * It UPDATES the str to remaining part of the string if delimiter is found!
 * return -1 if it does not contain the delimiter
 * return the first index if found
 */
int findUntilNext(char *result, char *str, char delimiter)
{
    int i = 0;
    u_long length = strlen(str);
    while (str[i] != delimiter && i < length)
    {
        i++;
    }
    memcpy(result, str, i);
    result[i] = '\0';
    // printf("\n%s : %s : %d\n", result, str, i);
    if (str[i] == delimiter)
    {
        memcpy(str, &str[i + 1] /*don't include the delimiter*/, length < (i + 1) ? 0 : length - i - 1);
        length < (i + 1) ? 0 : (str[length - i - 1] = '\0');

        return i;
    }
    return -1;
}

void print_d_helper(int fd, struct msdos_dir_entry *dep)
{
    // NAME SHOULD NOT TAKE THIS LONG
    char dep_sname[12] = "\0";
    char n[9], ext[4];
    trim_split_filename((char *)dep->name, n, ext);
    strcat(dep_sname, n);
    // if(dep[i].attr == type_volume  ||  dep[i].attr == type_directory )
    if (dep->attr == type_file)
    {
        strcat(dep_sname, ".");
        strcat(dep_sname, ext);
    }
    printf("name = %s\n", dep_sname); // FILE2.BIN
    printf("type = %s\n", dep->attr == type_volume      ? "VOLUME"
                          : dep->attr == type_directory ? "DIRECTORY"
                          : dep->attr == type_file      ? "FILE"
                                                        : "UNKNOWN"); // FILE
    u_int fc = dep->start + (dep->starthi << 16);
    printf("firstcluster = %d\n", fc); // 7
    int cc = 0;
    /**
     * equal or bigger than 0x0FFFFFF8 indicates the end of a cluster chain (end
     * of file) (EOC or EOF), for a file or directory. The value 0x0FFFFFF7 is bad
     * cluster mark.
     */
    if (fc != 0)
    {
        while (fc < 0x0FFFFFF7)
        {
            fc = readFAT(fd, fc);
            cc++;
        };
    }
    printf("clustercount = %d\n", cc);       // 10
    printf("size(bytes) = %d\n", dep->size); // = 10240

    // refer to https://formats.kaitai.io/dos_datetime/
    int day, month, year, /*secs,*/ mins, hours;
    day = dep->date & 31;
    month = ((dep->date >> 5) - 1) & 15;
    year = dep->date >> 9;
    // secs = (dep->time & 31) * 2;
    mins = ((dep->time >> 5) & 63);
    hours = (dep->time >> 11);
    /* days since 1.1.70 plus 80's leap day */
    printf("date = %.2d-%.2d-%.4d\n", day, month, year + 1980); // 09-04-2022
    printf("time = %.2d:%.2d\n", hours, mins);                  // 10:00
}
unsigned int readFAT(int file, u_int cnum)
{
    // find the sector & the offset
    u_int snum =
        fat_start_sector + (cnum / (SECTOR_SIZE / 4 /*each one takes 4 bytes*/)); // since we expect nonnegative
                                                                                  // input it is safe to use
                                                                                  // ‘truncation toward zero’
    u_int offset = (cnum % (SECTOR_SIZE / 4));
    u_char buf[SECTOR_SIZE];
    readsector(file, buf, snum);

    return ((u_int *)buf)[offset];
}