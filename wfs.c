#include <wfs.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

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
    if (res != 0)
    {
        return -ENOENT;
    }

    inode.atim = time(NULL);

    stbuf->st_mode = inode.mode;
    stbuf->st_nlink = inode.nlinks;
    stbuf->st_size = inode.size;
    stbuf->st_uid = inode.uid;
    stbuf->st_gid = inode.gid;
    stbuf->st_atime = inode.atim;
    stbuf->st_mtime = inode.mtim;
    stbuf->st_ctime = inode.ctim;

    if (S_ISBLK(inode.mode) || S_ISCHR(inode.mode))
    {
        stbuf->st_rdev = inode.blocks[0];
    }

    return 0; // Return 0 on success
}

// Placeholder
static int wfs_mknod(const char *path, struct stat *stbuf)
{
    return 0; // Return 0 on success
}

// Placeholder
static int wfs_mkdir(const char *path, struct stat *stbuf)
{
    return 0; // Return 0 on success
}

// Placeholder
static int wfs_unlink(const char *path, struct stat *stbuf)
{
    return 0; // Return 0 on success
}

// Placeholder
static int wfs_rmdir(const char *path, struct stat *stbuf)
{
    return 0; // Return 0 on success
}

// Placeholder
static int wfs_read(const char *path, struct stat *stbuf)
{
    return 0; // Return 0 on success
}

// Placeholder
static int wfs_write(const char *path, struct stat *stbuf)
{
    return 0; // Return 0 on success
}

// Placeholder
static int wfs_readdir(const char *path, struct stat *stbuf)
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