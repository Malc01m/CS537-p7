#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include "wfs.h"
// #include <fuse.h>

char *disk_img;
int num_inodes = -1;
int num_blocks = -1;

void parse_args(int argc, char **argv)
{
    // Print Debug Statements Y = 1 N = 0
    int debug_statements = 0;

    int input_value;
    while ((input_value = getopt(argc, argv, "d:i:b:")) != -1)
    {
        switch (input_value)
        {
        case 'd':
            // Make room for the disk image file
            disk_img = (char *)malloc((strlen(optarg) + 1) * sizeof(char));
            // Copy the disk image file into the disk_img variable
            strcpy(disk_img, optarg);
            break;
        case 'i':
            num_inodes = atoi(optarg);
            break;
        case 'b':
            num_blocks = atoi(optarg);
            break;
        default:
            printf("Usage: ./mkfs -d <disk_image> -i <num_inodes> -b <num_blocks>\n");
            exit(EXIT_FAILURE);
        }
    }
    if (disk_img == NULL || num_inodes == -1 || num_blocks == -1 || num_blocks < 1 || num_inodes < 1)
    {
        printf("Usage: ./mkfs -d <disk_image> -i <num_inodes> -b <num_blocks>\n");
        exit(EXIT_FAILURE);
    }

    if (debug_statements)
    {
        printf("Disk image: %s\n", disk_img);
        printf("Number of inodes: %d\n", num_inodes);
        printf("Number of blocks: %d\n", num_blocks);
    }
}

int main(int argc, char *argv[])
{
    parse_args(argc, argv);

    return 0;
}