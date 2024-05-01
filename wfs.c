#include "wfs.h"
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

// Turn on/off debug statements
int debug_statements = 0;

char *disk_img = NULL;
struct wfs_sb *sb;

int get_inode(const char *path, struct wfs_inode *inode)
{
    return 0;
}

static int wfs_getattr(const char *path, struct stat *stbuf)
{
    // Implementation of getattr function to retrieve file attributes
    // Fill stbuf structure with the attributes of the file/directory indicated by path
    // ...
    memset(stbuf, 0, sizeof(struct stat));
    struct wfs_inode inode;

    int res = get_inode(path, &inode);

    if (debug_statements) {
        printf("%d",res);
    }

    if (res != 0)
    {
        return -ENOENT;
    }

    inode.atim = time(NULL);
    stbuf->st_atime = inode.atim;

    stbuf->st_mode = inode.mode;
    stbuf->st_nlink = inode.nlinks;
    stbuf->st_size = inode.size;
    stbuf->st_uid = inode.uid;
    stbuf->st_gid = inode.gid;
    stbuf->st_mtime = inode.mtim;
    stbuf->st_ctime = inode.ctim;

    if (S_ISBLK(inode.mode) || S_ISCHR(inode.mode))
    {
        stbuf->st_rdev = inode.blocks[0];
    }

    return 0; // Return 0 on success
}

// Placeholder
static int wfs_mknod(const char *path, mode_t mode, dev_t dev)
{
    return 0; // Return 0 on success
}

// Placeholder
static int wfs_mkdir(const char *path, mode_t mode)
{
    return 0; // Return 0 on success
}

// Placeholder
static int wfs_unlink(const char *path)
{
    return 0; // Return 0 on success
}

// Placeholder
static int wfs_rmdir(const char *path)
{
    return 0; // Return 0 on success
}

static int wfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    struct wfs_inode inode;

    if (get_inode(path, &inode) != 0) {
        return -ENOENT;
    }

    // Offset beyond EOF check
    if (offset >= inode.size) {
        return 0;
    }

    size_t total_read = 0;
    while (size > 0 && offset < inode.size) {
        off_t block_index = offset / BLOCK_SIZE;
        off_t block_offset = offset % BLOCK_SIZE;
        size_t bytes_in_block = BLOCK_SIZE - block_offset;
        size_t read_remain = (size < bytes_in_block) ? size : bytes_in_block;
        read_remain = (read_remain > inode.size - offset) ? inode.size - offset : read_remain;

        if (block_index < D_BLOCK) {
            char *src = disk_img + inode.blocks[block_index] * BLOCK_SIZE + block_offset;
            memcpy(buf, src, read_remain);
        }
        else if (block_index >= D_BLOCK && block_index < N_BLOCKS) {
            // Indirect block reading
            off_t indirect_index = block_index - D_BLOCK;
            off_t *indirect_block = (off_t *)(disk_img + sb->d_blocks_ptr + inode.blocks[IND_BLOCK] * BLOCK_SIZE);
            off_t data_block_addr = indirect_block[indirect_index];
            char *src = disk_img +  sb->d_blocks_ptr + data_block_addr * BLOCK_SIZE + block_offset;
            memcpy(buf, src, read_remain);
        }
        else {
            // idk how it would ever get here but just in case...
            printf("ERROR");
            return 1;
        }

        buf += read_remain;
        offset += read_remain;
        total_read += read_remain;
        size -= read_remain;
    }
    return total_read;
}

static int wfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    struct wfs_inode inode;

    if (get_inode(path, &inode) != 0) {
        return -ENOENT;
    }

    size_t total_write = 0;
    int fd = open (disk_img, O_RDWR);

    while (size > 0) {
        off_t block_index = offset / BLOCK_SIZE;
        off_t block_offset = offset % BLOCK_SIZE;
        size_t bytes_in_block = BLOCK_SIZE - block_offset;
        size_t write_amount = (size < bytes_in_block) ? size : bytes_in_block;

        if (block_index < D_BLOCK) {
            char *dest = disk_img + inode.blocks[block_index] * BLOCK_SIZE + block_offset;
            memcpy(dest, buf, write_amount);
        }
        else if (block_index >= D_BLOCK && block_index < N_BLOCKS) {
            off_t indirect_index = block_index - D_BLOCK;
            off_t *indirect_block = (off_t *)(disk_img + inode.blocks[IND_BLOCK] * BLOCK_SIZE);
            off_t data_block_addr = indirect_block[indirect_index];
            char *dest = disk_img + data_block_addr * BLOCK_SIZE + block_offset;
            memcpy(dest, buf, write_amount);
        }
        else {
            close(fd);
            printf("ERROR: ENOSPC wfs_write");
            return -ENOSPC;
        }

        buf += write_amount;
        offset += write_amount;
        total_write += write_amount;
        size -= write_amount;
    }

    // Update inode size & mtime
    if (offset > inode.size) {
        inode.size = offset;
    }
    inode.mtim = time(NULL);

    off_t inode_offset = sb->i_blocks_ptr + (inode.num * sizeof(struct wfs_inode));
    if (pwrite(fd, &inode, sizeof(struct wfs_inode), inode_offset) != sizeof(struct wfs_inode)) {
        printf("ERROR: writeback fail");
        close(fd);
        return 1;
    }

    close(fd);

    return total_write;
}

// Placeholder
static int wfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    return 0; // Return 0 on success
}

static struct fuse_operations ops = {
    .getattr = wfs_getattr,
    .mknod = wfs_mknod,
    .mkdir = wfs_mkdir,
    .unlink = wfs_unlink,
    .rmdir = wfs_rmdir,
    .read = wfs_read,
    .write = wfs_write,
    .readdir = wfs_readdir,
};

int main(int argc, char *argv[])
{
    // Initialize FUSE with specified operations
    // Filter argc and argv here and then pass it to fuse_main
    return fuse_main(argc, argv, &ops, NULL);
}