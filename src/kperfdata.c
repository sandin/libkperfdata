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

#include <assert.h>  // assert
#include <stdbool.h>  // bool
#include <stdlib.h>  // malloc

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
    first_record->ready = true;
    uint32_t cpuid = first_record->cpuid;
    cursor->unknown_c8[cpuid] = 0;
    cursor->unknown_2c8[cpuid] = 0;
    cursor->unknown_4c8[cpuid] = 0;
    return true;
  }
}

static kd_buf* kpdecode_cursor_next_kevent(kpdecode_cursor* cursor) {
  assert(sizeof(RAW_header_v1) == KPERFDATA_SIZEOF_RAW_HEADER_V1);
  assert(sizeof(RAW_header_v2) + 0x100 == KPERFDATA_SIZEOF_RAW_HEADER_V2);
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
    }  // endif (size >= KPERFDATA_SIZEOF_RAW_HEADER_V2)
  }  // endif (cursor->state == 0)

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
      }  // else (invalid):
      // step to the next item in the threadmap
      cursor->cur_kd_threadmap_ptr = cur_threadmap_ptr + cursor->size_of_kd_threadmap;
    }  // end of while
  }  // endif (!cursor->threadmap_decoded && cursor->cur_kd_threadmap_ptr != NULL)

  // decode kd_buf
  char* cur_kd_buf_ptr = cursor->cur_kd_buf_ptr;
  if (cur_kd_buf_ptr != NULL) {
    kd_buf* kevent;
    if (is32bit) {
      // on 32-bit, we need copy the data from kd_buf_32 in buffer to the kd_buf_64 in cursor
      kd_buf_32* kd_buf = (kd_buf_32*)kd_buf;
      kevent = &cursor->kd_buf;
      kevent->timestamp = kd_buf->timestamp & KPERFDATA_TIMESTAMP_MASK;
      kevent->arg1 = (uint64_t)kd_buf->arg1;
      kevent->arg2 = (uint64_t)kd_buf->arg2;
      kevent->arg3 = (uint64_t)kd_buf->arg3;
      kevent->arg4 = (uint64_t)kd_buf->arg4;
      kevent->arg5 = (uint64_t)kd_buf->arg5;
      kevent->debugid = kd_buf->debugid;
      kevent->cpuid = (uint32_t)((kd_buf->timestamp & KPERFDATA_CPU_MASK) >> KPERFDATA_CPU_SHIFT);
    } else {  // is64Bit
      // on 64-bit, we just return the pointer to kd_buf in buffer, no need to copy it
      kd_buf_64* kd_buf = (kd_buf_64*)kd_buf;
      kevent = kd_buf;
    }

    char* next_kd_buf_ptr = cur_kd_buf_ptr + cursor->size_of_kd_buf;
    char* end_of_buffer = *cursor->buffer_ptr + cursor->buffer_size1;
    if (next_kd_buf_ptr >= end_of_buffer) {
      cursor->cur_kd_buf_ptr = NULL;  // EOF
    } else {
      cursor->cur_kd_buf_ptr = next_kd_buf_ptr;  // step to the next item in the kd_buf
    }

    return kevent;
  }  // endif (cur_kd_buf_ptr != NULL)
  return NULL;
}

long kpdecode_cursor_next_record(kpdecode_cursor* cursor, kpdecode_record** next_record) {
  int ret;
  kd_buf* kevent = NULL;
  while (!record_ready(cursor)) {
    kevent = kpdecode_cursor_next_kevent(cursor);
    if (kevent == NULL) {
      break;
    }

    // Got a new kevent
    cursor->kevent_count += 1;

    kpdecode_record* record =
        (kpdecode_record*)calloc(1, sizeof(kpdecode_cursor));  // malloc(sizeof(kpdecode_record));
    if (!record) {
      return KPERFDATA_RET_OOM;
    }
    record->ready = false;
    uint64_t timestamp = kevent->timestamp;
    record->timestamp = timestamp;
    record->total_size_of_kevents = cursor->size_of_kd_buf * cursor->kevent_count;

    uint32_t cpuid = kevent->cpuid;
    if (cpuid >= KPERFDATA_MAX_CPUS) {
      record->flags = 0x8000000000000017;
      record->kd_buf.debugid = kevent->debugid;
      record->kd_buf.args[0] = kevent->arg1;
      record->kd_buf.args[1] = kevent->arg2;
      record->kd_buf.args[2] = kevent->arg3;
      record->kd_buf.args[3] = kevent->arg4;
      record->tid = kevent->arg5;
      record->cpuid = cpuid;
      record->ready = true;
      // append a new record to the end of the linked list
      KPERFDATA_LINKED_LIST_APPEND_ITEM(cursor, record);

      ret = 0;
      goto SWITCH_CTRL;  // continue;
    }  // endif (cpuid >= KPERFDATA_MAX_CPUS)

    if (timestamp) {
      if (kevent->debugid != KPERFDATA_DEBUGID(KPERFDATA_DBG_PERF, 153, 0, 0)) {
        uint64_t kevent_count_pre_count = cursor->unknown_ac8[cpuid] + 1;
        cursor->unknown_ac8[cpuid] = kevent_count_pre_count;
        if (kevent_count_pre_count > cursor->unknown_cc8) {
          cursor->unknown_cc8 = kevent_count_pre_count;
        }
      }
    }

    uint64_t flags;
    uint32_t debugid;
    if (cursor->unknown_option != 0) {
      flags = 0x0000000000000017;
      record->flags = flags;
      debugid = kevent->debugid;
      record->kd_buf.debugid = debugid;
      record->kd_buf.args[0] = kevent->arg1;
      record->kd_buf.args[1] = kevent->arg2;
      record->kd_buf.args[2] = kevent->arg3;
      record->kd_buf.args[3] = kevent->arg4;
      record->tid = kevent->arg5;
      record->cpuid = cpuid;
    } else {
      flags = 0x0000000000000000;
      debugid = kevent->debugid;
    }

    // the begin of the switch statement:

    if (debugid == KPERFDATA_TRACE_LOST_EVENTS) {
      // clang-format off
      // |---------------------------------------------------------------------------------------------|
      // | Class: DBG_TRACE | SubClass: DBG_TRACE_INFO | Code: TRACE_LOST_EVENTS | Func: DBG_FUNC_NONE |
      // | Arg1: -          | Arg2: -                  | Arg3: -                 | Arg4: -             |
      // |---------------------------------------------------------------------------------------------|
      // clang-format on
      //
      // Emit a lost events tracepoint to indicate that previous events were lost -- the thread map
      // cannot be trusted.
      record->flags = flags | 0x0000000000010003;
      record->cpuid = kevent->cpuid;
      record->timestamp = timestamp;
      record->unknown_field20.unknown_field1 = cursor->unknown_8c8[cpuid];
      record->ready = true;
      cursor->unknown_8c8[cpuid] = timestamp;
      kpdecode_record* cpu_record = cursor->unknown_c8[cpuid];
      if (cpu_record != NULL) {
        cpu_record->flags |= 0x8000000000000000;
        cpu_record->ready = true;
        cursor->unknown_c8[cpuid] = NULL;
      }

      kpdecode_record* cpu_record1 = cursor->unknown_2c8[cpuid];
      if (cpu_record1 != NULL) {
        cpu_record->flags |= 0x8000000000000000;
        cpu_record->ready = true;
        cursor->unknown_2c8[cpuid] = NULL;

        cursor->unknown_4c8[cpuid]->flags |= 0x8000000000000000;
        cursor->unknown_2c8[cpuid] = NULL;
        ret = 0;  // continue
        goto NEXT_RECORD;
      }
      ret = 0;
      goto SWITCH_CTRL;
    }

    cursor->unknown_8c8[cpuid] = timestamp;
    if (debugid == KPERFDATA_PERF_GEN_EVENT_START) {
      // clang-format off
      // |---------------------------------------------------------------------------------------------|
      // | Class: PERF_GENERIC | SubClass: PERF_GENERIC | Code: PERF_GEN_EVENT | Func: DBG_FUNC_START  |
      // | Arg1: sample_what   | Arg2: actionid         | Arg3: userdata       | Arg4: sample_flags    |
      // |---------------------------------------------------------------------------------------------|
      // clang-format on
      //
      // Before calling kperf_sample_internal() and kperf_sample_user_internal()
      if (cursor->unknown_c8[cpuid] != NULL) {
        ret = 2;  // return
        goto NEXT_RECORD;
      }
      cursor->unknown_c8[cpuid] = record;  // save the first record of this cpu
      record->flags = flags | 0x0000000000002007;
      record->cpuid = kevent->cpuid;
      record->kperf_sample_args.actionid = kevent->arg2;
      record->tid = kevent->arg5;
      ret = 0;  // continue
      goto NEXT_RECORD;
    }

    // TODO: other cases

  NEXT_RECORD:  // LABEL_113:
    if (record->flags != 0) {
      if (cursor->unknown_cc8 < KPERFDATA_MAX_RECORDS_PRE_CPU) {
        record->flags |= 0x0000000000020000;
      }

      KPERFDATA_LINKED_LIST_APPEND_ITEM(cursor, record);
    } else {
      kpdecode_record_free(record);
    }

    // TODO: use a `switch` statement to get rid of the ugly `goto`.
  SWITCH_CTRL:  // LABEL_122:
    if (ret == 2) {
      return ret;
    } else if (ret == 1) {
      break;
    } else {  // ret == 0
      continue;
    }
  }  // end while

  if (record_ready(cursor)) {
    // pop the first record of the linked list
    kpdecode_record* first_record = cursor->kpdeocde_record_head;
    *next_record = first_record;
    --cursor->kpdecode_record_count;
    cursor->kpdeocde_record_head = (kpdecode_record*)first_record->next;
    if (cursor->kpdecode_record_tail == first_record) {
      cursor->kpdecode_record_tail = NULL;
    }
    first_record->next = NULL;
    return KPERFDATA_RET_OK;
  }
  return KPERFDATA_RET_NOT_READY;
}

KPERFDATA_END_CPP_NAMESPACE
