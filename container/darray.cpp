//
// Created by Hongjian Zhu on 2022/10/9.
//

#include "darray.h"
#include "logging.h"

void *darray_create(u64 capacity, u64 stride, MemoryTag tag) {
  u64  headerSize              = DArrayField::Max * sizeof(u64);
  u64  arraySize               = capacity * stride;
  auto array                   = (u64 *) memory_allocate(headerSize + arraySize, tag);
  array[DArrayField::Capacity] = capacity;
  array[DArrayField::Length]   = 0;
  array[DArrayField::Stride]   = stride;
  return array + DArrayField::Max;
}

void darray_destroy(void *array, MemoryTag tag) {
  auto header     = (u64 *) array - DArrayField::Max;
  u64  headerSize = DArrayField::Max * sizeof(u64);
  u64  totalSize  = headerSize + header[DArrayField::Capacity] * header[DArrayField::Stride];
  memory_free(header, totalSize, tag);
}

u64 darray_field_get(void *array, DArrayField field) {
  auto header = (u64 *) array - DArrayField::Max;
  return header[field];
}

void darray_field_set(void *array, DArrayField field, u64 value) {
  auto header   = (u64 *) array - DArrayField::Max;
  header[field] = value;
}

void *darray_resize(void *array) {
  u64   length = darray_length(array);
  u64   stride = darray_stride(array);
  void *darray = darray_create(DARRAY_RESIZE_FACTOR * darray_capacity(array), stride);
  memory_copy(darray, array, length * stride);
  darray_length_set(darray, length);
  darray_destroy(array);
  return darray;
}

void *darray_push(void *array, const void *src) {
  u64 length = darray_length(array);
  u64 stride = darray_stride(array);
  if (length >= darray_capacity(array)) { array = darray_resize(array); }
  auto dst = (u64 *) array;
  dst += (length * stride);
  memory_copy(dst, src, stride);
  darray_length_set(array, length + 1);
  return array;
}

void darray_pop(void *array, void *dst) {
  u64  length = darray_length(array);
  u64  stride = darray_stride(array);
  auto src    = (u64 *) array;
  src += ((length - 1) * stride);
  if (dst) { memory_copy(dst, src, stride); }
  darray_length_set(array, length - 1);
}

void *darray_insert_at(void *array, u64 index, const void *src) {
  u64 length = darray_length(array);
  u64 stride = darray_stride(array);
  if (index >= length) {
    LOG_ERROR("Index outside the bounds of the array. Length: %d, index: %d", length, index);
    return array;
  }
  if (length >= darray_capacity(array)) { array = darray_resize(array); }

  auto dst = (u64 *) array;
  if (index != length - 1) { // If not on the last element, copy the rest outward
    memory_copy(dst + (index + 1) * stride, dst + index * stride, stride * (length - index));
  }
  memory_copy(dst + index * stride, src, stride);
  darray_length_set(array, length + 1);
  return array;
}

void *darray_pop_at(void *array, u64 index, void *dst) {
  u64 length = darray_length(array);
  u64 stride = darray_stride(array);
  if (index >= length) {
    LOG_ERROR("Index outside the bounds of the array. Length: %d, index: %d", length, index);
    return array;
  }

  auto src = (u64 *) array;
  if (dst) { memory_copy(dst, src + index * stride, stride); }

  if (index != length - 1) {
    // If not on the last element, snip out the entry and copy the reset inward
    memory_copy(src + index * stride, src + (index - 1) * stride, stride * (length - index));
  }
  darray_length_set(array, length - 1);
  return array;
}

void darray_clear(void *array) { darray_field_set(array, DArrayField::Length, 0); }

u64 darray_capacity(void *array) { return darray_field_get(array, DArrayField::Capacity); }

u64 darray_length(void *array) { return darray_field_get(array, DArrayField::Length); }

u64 darray_stride(void *array) { return darray_field_get(array, DArrayField::Stride); }

void darray_length_set(void *array, u64 n) { darray_field_set(array, DArrayField::Length, n); }
