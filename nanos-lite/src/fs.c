#include "fs.h"

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
  assert(0);
  return -1;
}

size_t fs_read(int fd, void* buf, size_t len) {
  Finfo f = file_table[fd];
  if (f.size < f.open_offset + len) {
    len = f.size - f.open_offset;
  }
  size_t real_len = ramdisk_read(buf, f.disk_offset + f.open_offset, len);
  f.open_offset += real_len;
  assert(0 <= f.open_offset && f.open_offset <= f.size);
  file_table[fd] = f;
  return real_len;
}

size_t fs_write(int fd, const void* buf, size_t len) {
  Finfo f = file_table[fd];
  if (len > f.size - f.open_offset) {
    len = f.size - f.open_offset;
  }
  size_t real_len = ramdisk_write(buf, f.disk_offset + f.open_offset, len);
  f.open_offset += real_len;
  assert(0 <= f.open_offset && f.open_offset <= f.size);
  file_table[fd] = f;
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
  Log("fd=%d offset=%d open_offset=%d", fd, offset, f.open_offset);
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
