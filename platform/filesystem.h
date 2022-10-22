//
// Created by Hongjian Zhu on 2022/10/22.
//

#pragma once

#include "defines.h"

// Holds a handle to a file
struct FileHandle {
  void *handle; // Opaque handle to internal file handle
};

enum FileMode : u8 {
  FILE_MODE_READ  = 0x1,
  FILE_MODE_WRITE = 0x2,
};

bool filesystem_exists(CString path);

bool filesystem_open(CString path, FileMode mode, bool isBinary, FileHandle *out);

void filesystem_close(FileHandle *handle);

/**
 * Reads up to a newline or EOF. Allocates *dst, which must be freed by the caller.
 * @param handle
 * @param line A pointer to a character array which will be allocated and populated.
 * @return
 */
bool filesystem_read_line(FileHandle *handle, char **line);

/**
 * Writes text to the provided file, appending a '\n' afterward.
 * @param handle
 * @param text The text to be written.
 * @return
 */
bool filesystem_write_line(FileHandle *handle, CString text);

/**
 * Reads up to `size` bytes of data. Allocates *dst, which must be freed by the caller.
 * @param handle
 * @param size The number of bytes to read.
 * @param dst A pointer to a block of memory to be populated.
 * @param read A pointer to a number which will be populated with the number of bytes actually read.
 * from the file.
 * @return
 */
bool filesystem_read(FileHandle *handle, u64 size, void *dst, u64 *read);

/**
 * Reads up to `size` bytes of data. Allocates *dst, which must be freed by the caller.
 * @param handle
 * @param dst A pointer to a byte array which will be allocated and populated.
 * @param read A pointer to a number which will be populated with the number of bytes actually read.
 * @return
 */
bool filesystem_read_all(FileHandle *handle, u8 **dst, u64 *read);

/**
 * Writes provided data to the file.
 * @param handle
 * @param size
 * @param src
 * @param written
 * @return
 */
bool filesystem_write(FileHandle *handle, u64 size, const void *src, u64 *written);
