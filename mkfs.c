#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "wfs.h"
// #include <fuse.h>

//Turn on/off debug statements
int debug_statements = 0;

char *disk_img;
int num_inodes = -1;
int num_blocks = -1;

void parse_args(int argc, char **argv)
{
    // Print Debug Statements Y = 1 N = 0

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

void init_fs(int fd, int num_inodes, int num_blocks) {
    struct wfs_sb sb;
    size_t bitmap_size = (num_blocks + 7) /8 ;

    sb.num_inodes = num_inodes;
    sb.num_data_blocks = num_blocks;
    sb.i_bitmap_ptr = sizeof(struct wfs_sb);
    sb.d_bitmap_ptr = sb.i_bitmap_ptr + ((num_inodes + 7) / 8);
    sb.i_blocks_ptr = sb.d_bitmap_ptr + bitmap_size;
    sb.d_blocks_ptr = sb.i_blocks_ptr + num_inodes * sizeof(struct wfs_inode);

    //Write the SB
    if (pwrite(fd, &sb, sizeof(sb), 0) != sizeof(sb)) {
        printf("ERROR: SB Writing error\n");
        exit(1);
    }

    //Write bitmaps
    char *zero_buf = calloc(bitmap_size, 1);
    pwrite(fd, zero_buf, bitmap_size, sb.i_bitmap_ptr);
    pwrite(fd, zero_buf, bitmap_size, sb.d_bitmap_ptr);
    free(zero_buf);
}

int main(int argc, char *argv[])
{
    parse_args(argc, argv);

    int fd = open(disk_img, O_RDWR | O_CREAT | O_TRUNC);
    if (fd == -1){
        printf("ERROR: fd open error.\n");
        exit(1);
    }

    init_fs(fd, num_inodes, num_blocks);

    close(fd);
    free(disk_img);

    return 0;
}