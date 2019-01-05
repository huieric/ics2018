#include "fs.h"

size_t ramdisk_read(void* buf, size_t offset, size_t len);
size_t ramdisk_write(const void* buf, size_t offset, size_t len);

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

size_t invalid_read(void *buf, size_t offset, size_t len);
size_t invalid_write(const void *buf, size_t offset, size_t len);
size_t dispinfo_read(void* buf, size_t offset, size_t len);
size_t fb_write(const void* buf, size_t offset, size_t len);
size_t serial_write(const void* buf, size_t offset, size_t len);
size_t events_read(void* buf, size_t offset, size_t len);
int fs_open(const char* pathname, int flags, int mode);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  size_t open_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin", 0, 0, 0, invalid_read, invalid_write},
  {"stdout", 0, 0, 0, invalid_read, serial_write},
  {"stderr", 0, 0, 0, invalid_read, serial_write},
  {"/dev/fb", 0, 0, 0, invalid_read, fb_write},
  {"/proc/dispinfo", 0, 0, 0, dispinfo_read, invalid_write},
  {"/dev/events", 0, 0, 0, events_read, invalid_write},
  {"/dev/tty", 0, 0, 0, invalid_read, serial_write},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  int fd = fs_open("/dev/fb", 0, 0);
  file_table[fd].size = screen_width() * screen_height() * sizeof(uint32_t);
}

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

int fs_open(const char* pathname, int flags, int mode) {
  for (int i = 0; i < NR_FILES; i++) {
    if (strcmp(pathname, file_table[i].name) == 0) {
      file_table[i].open_offset = 0;
      return i;
    }
  }
  Log("pathname=%s", pathname);
  assert(0);
  return -1;
}

size_t fs_read(int fd, void* buf, size_t len) {
  Finfo* f = &file_table[fd];
  if (f->read) {
    size_t real_len = f->read(buf, f->open_offset, len);
    f->open_offset += real_len;
    return real_len;
  }
  if (f->size < f->open_offset + len) {
    len = f->size - f->open_offset;
  }
  size_t real_len = ramdisk_read(buf, f->disk_offset + f->open_offset, len);
  f->open_offset += real_len;
  assert(0 <= f->open_offset && f->open_offset <= f->size);
  return real_len;
}

size_t fs_write(int fd, const void* buf, size_t len) {
  Finfo* f = &file_table[fd];
  if (f->write) {
    Log("fd=%d buf=%s fname=%s", fd, buf, f->name);
    size_t real_len = f->write(buf, f->open_offset, len);
    f->open_offset += real_len;
    return real_len;
  }
  if (len > f->size - f->open_offset) {
    len = f->size - f->open_offset;
  }
  size_t real_len = ramdisk_write(buf, f->disk_offset + f->open_offset, len);
  f->open_offset += real_len;
  assert(0 <= f->open_offset && f->open_offset <= f->size);
  return real_len;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  Finfo f = file_table[fd];
  switch (whence) {
    case SEEK_SET: f.open_offset = offset; break;
    case SEEK_CUR: f.open_offset += offset; break;
    case SEEK_END: f.open_offset = f.size + offset; break;
    default: assert(0);
  }
  assert(0 <= f.open_offset && f.open_offset <= f.size);
  file_table[fd] = f;
  return f.open_offset;
}

int fs_close(int fd) {
  return 0;
}

size_t fs_filesz(int fd) {
  return file_table[fd].size;
}
