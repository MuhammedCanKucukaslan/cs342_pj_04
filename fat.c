#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h> // open() 
#include <unistd.h> // and read()
#include <sys/mman.h> // stat()

//#include <

void pln(char *input);
// const char* v = "-v";
void print_v(char *disk_image);

int main(int argc, char **argv)
{

    printf("%s : %d\n", argv[0], argc);
    pln("new line huh");
    if (argc < 2)
    {
        pln("Not enough argument is provided. fat -h");
    }
    // terrible condition
    if (argc == 2)
    {
        // todo print help
    }

    if (strcmp(argv[2], "-v") == 0)
    {
        pln("print info");
        print_v(argv[1]);
    }

    printf("Tmp\n");
    return 0;
}

void pln(char *input)
{
    printf("%s\n", input);
}

void print_v(char *disk_image)
{
    // open file in raw and read
    char* dev;

    int file = open(disk_image, O_RDONLY);
    if (file == -1)
    {
        pln("Error opening file");
    }
    // read file
    dev = (char *)mmap(NULL, 4096, PROT_READ, MAP_SHARED, file, 0);
    pln(dev);
    printf("\n1: 0x%x\n",(int )dev);
    // print file
    // close file
    close(file);
}