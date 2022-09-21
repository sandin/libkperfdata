// Copyright 2022 liudingsan
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "kperfdata/kperfdata.h"

#include <assert.h>   // assert
#include <stdbool.h>  // bool
#include <stdlib.h>   // malloc

KPERFDATA_START_CPP_NAMESPACE

kpdecode_cursor* kpdecode_cursor_create() { return calloc(1, sizeof(kpdecode_cursor)); }

void kpdecode_cursor_free(kpdecode_cursor* cursor) { free(cursor); }

long kpdecode_cursor_setchunk(kpdecode_cursor* cursor, const char* bytes, size_t size) {
  if (cursor->buffer == NULL) {
    cursor->buffer = bytes;
    cursor->unknown_28 = 0;
    cursor->buffer_size = size;
    cursor->buffer_size1 = size;
    cursor->cur_kd_buf_ptr = (char*)bytes;
    return KPERFDATA_RET_OK;
  }
  return KPERFDATA_RET_FAIL;
}

char* kpdecode_cursor_clearchunk(kpdecode_cursor* cursor) {
  char* chunk = NULL;
  if (cursor->buffer) {
    chunk = (char*)cursor->buffer;
    cursor->cur_kd_buf_ptr = NULL;
    cursor->end_kd_threadmap_ptr = NULL;
    cursor->unknown_28 = 0;
    cursor->buffer = NULL;
    cursor->threadmap_decoded = 1;
  }
  return chunk;
}

long kpdecode_cursor_get_stats(kpdecode_cursor* cursor, int arg2) {
  if (arg2 == 1) {
    if (cursor->header_decoded) {
      return cursor->kpdecode_record_count;
    }
  } else if (arg2 == 0) {
    if (cursor->header_decoded) {
      kpdecode_record* first_record = cursor->kpdeocde_record_head;
      if (first_record) {
        return first_record->total_size_of_kevents;
      } else {
        return cursor->size_of_kd_buf * cursor->kevent_count;
      }
    }
  }
  return KPERFDATA_RET_FAIL;
}

long kpdecode_cursor_set_option(kpdecode_cursor* cursor, int arg2, long arg3) {
  if (arg2 != 0) {
    long old_value = cursor->unknown_option;
    cursor->unknown_option = arg3 != 0;
    return old_value;
  }
  return KPERFDATA_RET_FAIL;
}

void kpdecode_record_free(kpdecode_record* record) {
  void* unknown_field2 = record->unknown_field19.unknown_field2;  // malloc when subclass: 153
  if (unknown_field2) {
    free(unknown_field2);
  }
  free(record);
}

void kpdecode_cursor_flush() {
  // pass
}

static bool record_ready(kpdecode_cursor* cursor) {
  if (!cursor->header_decoded) {
    return false;
  }

  kpdecode_record* first_record = cursor->kpdeocde_record_head;
  if (first_record == NULL) {
    return false;
  }
  if (first_record->ready) {
    return true;
  }

  if (cursor->kpdecode_record_count <= KPERFDATA_MAX_RECORDS) {
    return false;
  } else {
    first_record->flags |= 0x8000000000000000;
    first_record->ready = 1;
    uint32_t cpuid = first_record->cpuid;
    cursor->unknown_c8[cpuid] = 0;
    cursor->unknown_2c8[cpuid] = 0;
    cursor->unknown_4c8[cpuid] = 0;
    return true;
  }
}

static kd_buf* kpdecode_cursor_next_kevent(kpdecode_cursor* cursor) {
  assert(sizeof(RAW_header_v1) == KPERFDATA_SIZEOF_RAW_HEADER_V1);
  assert(sizeof(RAW_header_v2) == KPERFDATA_SIZEOF_RAW_HEADER_V2);
  assert(sizeof(kd_threadmap_32) == KPERFDATA_SIZEOF_KD_THREADMAP_32);
  assert(sizeof(kd_threadmap_64) == KPERFDATA_SIZEOF_KD_THREADMAP_64);
  assert(sizeof(kd_buf_32) == KPERFDATA_SIZEOF_KD_BUF_32);
  assert(sizeof(kd_buf_64) == KPERFDATA_SIZEOF_KD_BUF_64);

  char* buffer = (char*)cursor->buffer;
  if (buffer == NULL) {
    return NULL;
  }

  // decode header
  if (cursor->state == KPERFDATA_STATE_HEADER_NOT_DECODED) {
    uint64_t size = cursor->buffer_size;
    if (size >= sizeof(RAW_header_v2)) {
      uint32_t version = *(uint32_t*)buffer;

      uint32_t header_size;
      int thread_count;
      uint32_t size_of_kd_threadmap;
      uint32_t size_of_kd_buf;
      int state;
      if (version == KPERFDATA_RAW_VERSION2) {
        RAW_header_v2* header = (RAW_header_v2*)buffer;
        thread_count = header->thread_count;
        header_size = sizeof(RAW_header_v2);
        bool is64bit = (header->flags & KPERFDATA_IS_64BIT) == KPERFDATA_IS_64BIT;
        if (is64bit) {
          state = KPERFDATA_STATE_64_BIT_HEADER;
          size_of_kd_threadmap = sizeof(kd_threadmap_64);
          size_of_kd_buf = sizeof(kd_buf_64);
        } else {
          state = KPERFDATA_STATE_32_BIT_HEADER;
          size_of_kd_threadmap = sizeof(kd_threadmap_32);
          size_of_kd_buf = sizeof(kd_buf_32);
        }
      } else if (version == KPERFDATA_RAW_VERSION1) {
        RAW_header_v1* header = (RAW_header_v1*)buffer;
        thread_count = header->thread_count;
        header_size = sizeof(RAW_header_v1);
        state = KPERFDATA_STATE_64_BIT_HEADER;
        size_of_kd_threadmap = sizeof(kd_threadmap_64);
        size_of_kd_buf = sizeof(kd_buf_64);
      } else {
        state = KPERFDATA_STATE_HEADER_NOT_DECODED;  // unknown version
      }

      if (state > KPERFDATA_STATE_HEADER_NOT_DECODED) {
        cursor->state = state;
        cursor->size_of_kd_threadmap = size_of_kd_threadmap;
        cursor->size_of_kd_buf = size_of_kd_buf;

        uint32_t threadmap_size = size_of_kd_threadmap * thread_count;
        uint32_t RAW_file_offset =
            KPERFDATA_PAGE_ALIGN(header_size + threadmap_size);  // TODO: test it

        cursor->header_decoded = 1;
        cursor->buffer_ptr = *((char*)cursor->buffer);

        char* RAW_file_ptr = buffer + RAW_file_offset;
        char* kd_buf_ptr = NULL;
        if (size >= RAW_file_offset + size_of_kd_buf) {  // includes at least one complete kd_buf
          kd_buf_ptr = RAW_file_ptr;
        }
        cursor->cur_kd_buf_ptr = kd_buf_ptr;

        char* threadmap_ptr = buffer + header_size;
        cursor->cur_kd_threadmap_ptr = threadmap_ptr;
        cursor->end_kd_threadmap_ptr = threadmap_ptr + threadmap_size;
      }  // endif (state > 0)
    }    // endif (size >= KPERFDATA_SIZEOF_RAW_HEADER_V2)
  }      // endif (cursor->state == 0)

  // The header has already been decoded
  if (!cursor->header_decoded) {
    return NULL;
  }

  // decode threadmap
  bool is32bit = cursor->state == KPERFDATA_STATE_32_BIT_HEADER;
  if (!cursor->threadmap_decoded && cursor->cur_kd_threadmap_ptr != NULL) {
    char* cur_threadmap_ptr;
    while (true) {
      cur_threadmap_ptr = cursor->cur_kd_threadmap_ptr;
      if (cur_threadmap_ptr >= cursor->end_kd_threadmap_ptr) {
        cursor->threadmap_decoded = true;
        break;
      }

      char* command;
      int valid;
      uint64_t tid;
      if (is32bit) {
        kd_threadmap_32* threadmap = (kd_threadmap_32*)cur_threadmap_ptr;
        tid = threadmap->thread;
        valid = threadmap->valid;
        command = threadmap->command;
      } else {  // is64Bit
        kd_threadmap_64* threadmap = (kd_threadmap_64*)cur_threadmap_ptr;
        tid = threadmap->thread;
        valid = threadmap->valid;
        command = threadmap->command;
      }

      if (valid) {
        kd_buf* kevent = &cursor->kd_buf;
        kevent->timestamp = 0;
        kevent->debugid = KPERFDATA_DEBUGID(7, 1, 2, 0);
        kevent->arg5 = tid;

        // clang-format off
        // copy command[20](20 bytes) to kd_buf_64.args[4](32 bytes):
        // |-------------------------------------------------------------------------------------------------|
        // | 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 | // byte
        // |-------------------------------------------------------------------------------------------------|
        // |            threadmap.command[20]                           |
        // |-------------------------------------------------------------------------------------------------|
        // |     kd_buf.arg1        |      kd_buf.arg2      |       kd_buf.arg3     |       kd_buf.arg4      |
        // |-------------------------------------------------------------------------------------------------|
        // clang-format on
        kevent->arg1 = *(uint64_t*)(command);
        kevent->arg2 = *(uint64_t*)(command + 8);
        kevent->arg3 = *(uint64_t*)(command + 16);
        return kevent;  // return this threadmap as a kevent
      }                 // else (invalid) continue
      cursor->cur_kd_threadmap_ptr =
          cur_threadmap_ptr +
          cursor->size_of_kd_threadmap;  // step to the next item in the threadmap
    }                                    // end while(true)
  }  // endif (!cursor->threadmap_decoded && cursor->cur_kd_threadmap_ptr != NULL)

  // decode kd_buf
  char* cur_kd_buf_ptr = cursor->cur_kd_buf_ptr;
  if (cur_kd_buf_ptr != NULL) {
    kd_buf* kevent;
    if (is32bit) {
      kd_buf_32* kd_buf = (kd_buf_32*)kd_buf;
      kevent = &cursor->kd_buf;  // on 32-bit, we need copy the data from kd_buf_32 in buffer to the
                                 // kd_buf_64 in cursor
      kevent->timestamp = kd_buf->timestamp & KPERFDATA_TIMESTAMP_MASK;
      kevent->arg1 = (uint64_t)kd_buf->arg1;
      kevent->arg2 = (uint64_t)kd_buf->arg2;
      kevent->arg3 = (uint64_t)kd_buf->arg3;
      kevent->arg4 = (uint64_t)kd_buf->arg4;
      kevent->arg5 = (uint64_t)kd_buf->arg5;
      kevent->debugid = kd_buf->debugid;
      kevent->cpuid = (uint32_t)((kd_buf->timestamp & KPERFDATA_CPU_MASK) >> KPERFDATA_CPU_SHIFT);
    } else {  // is64Bit
      kd_buf_64* kd_buf = (kd_buf_64*)kd_buf;
      kevent =
          kd_buf;  // on 64-bit, we just return the pointer to kd_buf in buffer, no need to copy it
    }

    char* next_kd_buf_ptr = cur_kd_buf_ptr + cursor->size_of_kd_buf;
    char* end_of_buffer = *cursor->buffer_ptr + cursor->buffer_size1;
    if (next_kd_buf_ptr >= end_of_buffer) {
      cursor->cur_kd_buf_ptr = NULL;  // EOF
    } else {
      cursor->cur_kd_buf_ptr =
          cur_kd_buf_ptr + cursor->size_of_kd_buf;  // step to the next item in the kd_buf
    }

    return kevent;  // return this kd_buf as a kevent
  }                 // endif (cur_kd_buf_ptr != NULL)
  return NULL;
}

long kpdecode_cursor_next_record(kpdecode_cursor* cursor, kpdecode_record* record) {
  kd_buf* kevent = NULL;
  while (!record_ready(cursor)) {
    kevent = kpdecode_cursor_next_kevent(cursor);
    if (kevent == NULL) {
      break;
    }
    cursor->kevent_count += 1;

    // TODO:
  }

  return -1;
}

KPERFDATA_END_CPP_NAMESPACE
