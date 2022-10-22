//
// Created by Hongjian Zhu on 2022/10/22.
//

#include "filesystem.h"
#include "logging.h"
#include "memory.h"
#include <cstdio>
#include <cstring>
#include <sys/stat.h>

bool filesystem_exists(CString path) {
  struct stat st {};
  return stat(path, &st) == 0;
}

bool filesystem_open(CString path, FileMode mode, bool isBinary, FileHandle *out) {
  CString flags;
  if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) != 0) {
    flags = isBinary ? "w+b" : "w+";
  } else if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) == 0) {
    flags = isBinary ? "rb" : "r";
  } else if ((mode & FILE_MODE_READ) == 0 && (mode & FILE_MODE_WRITE) != 0) {
    flags = isBinary ? "wb" : "w";
  } else {
    LOG_ERROR("Invalid file mode when opening file: '%s'", path);
    return false;
  }

  FILE *file = fopen(path, flags);
  if (!file) { return false; }

  out->handle = file;

  return true;
}

void filesystem_close(FileHandle *handle) {
  fclose((FILE *) handle->handle);
  handle->handle = nullptr;
}

bool filesystem_read_line(FileHandle *handle, char **line) {
  // Since it's reading a single line, it should be safe to assume this is enough characters.
  char buffer[32 * KiB];
  if (fgets(buffer, 32 * KiB, (FILE *) handle->handle) != nullptr) {
    u64 length = strlen(buffer);
    *line      = (char *) memory_allocate((sizeof(char) * length) + 1, MemoryTag::STRING);
    strcpy(*line, buffer);
    return true;
  }
  return false;
}

bool filesystem_write_line(FileHandle *handle, CString text) {
  i32 result = fputs(text, (FILE *) handle->handle);
  if (result != EOF) { result = fputc('\n', (FILE *) handle->handle); }
  fflush((FILE *) handle->handle);
  return result != EOF;
}

bool filesystem_read(FileHandle *handle, u64 size, void *dst, u64 *read) {
  *read = fread(dst, 1, size, (FILE *) handle->handle);
  return *read == size;
}

bool filesystem_read_all(FileHandle *handle, u8 **dst, u64 *read) {
  fseek((FILE *) handle->handle, 0, SEEK_END);
  u64 size = ftell((FILE *) handle->handle);
  rewind((FILE *) handle->handle);
  *dst  = (u8 *) memory_allocate(sizeof(u8) * size, MemoryTag::STRING);
  *read = fread(*dst, 1, size, (FILE *) handle->handle);
  return *read == size;
}

bool filesystem_write(FileHandle *handle, u64 size, const void *src, u64 *written) {
  *written = fwrite(src, 1, size, (FILE *) handle->handle);
  if (*written != size) { return false; }
  fflush((FILE *) handle->handle);
  return true;
}
