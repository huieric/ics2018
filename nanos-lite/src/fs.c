#include "fs.h"

size_t ramdisk_read(void* buf, size_t offset, size_t len);
size_t ramdisk_write(const void* buf, size_t offset, size_t len);

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  size_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin", 0, 0, 0, },
  {"stdout", 0, 0, 0, },
  {"stderr", 0, 0, 0, },
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

int fs_open(const char* pathname, int flags, int mode) {
  Log("called");
  for (int i = 0; i < NR_FILES; i++) {
    if (strcmp(pathname, file_table[i].name) == 0) {
      file_table[i].open_offset = 0;
      return i;
    }
  }
  assert(0);
  return -1;
}

size_t fs_read(int fd, void* buf, size_t len) {
  Finfo f = file_table[fd];
  if (f.size < f.open_offset + len) {
    len = f.size - f.open_offset;
  }
  Log("%s %d %d %d", f.name, f.size, f.disk_offset, f.open_offset);
  size_t real_len = ramdisk_read(buf, f.disk_offset + f.open_offset, len);
  f.open_offset += real_len;
  Log("0x%x 0x%x 0x%x 0x%x", fd, len, real_len, f.open_offset);
  assert(0 <= f.open_offset && f.open_offset <= f.size);
  return real_len;
}

size_t fs_write(int fd, const void* buf, size_t len) {
  Log("called");
  Finfo f = file_table[fd];
  if (len > f.size - f.open_offset) {
    len = f.size - f.open_offset;
  }
  size_t real_len = ramdisk_write(buf, f.disk_offset + f.open_offset, len);
  f.open_offset += real_len;
  assert(0 <= f.open_offset && f.open_offset <= f.size);
  return real_len;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  size_t* p = &file_table[fd].open_offset;
  Log("fd=0x%x open_offset=0x%x", fd, *p);
  switch (whence) {
    case SEEK_SET: *p = offset; break;
    case SEEK_CUR: *p += offset; break;
    case SEEK_END: *p = file_table[fd].size + offset; break;
    default: assert(0);
  }
  assert(0 <= *p && *p <= file_table[fd].size);
  return *p;
}

int fs_close(int fd) {
  return 0;
}

size_t fs_filesz(int fd) {
  return file_table[fd].size;
}
