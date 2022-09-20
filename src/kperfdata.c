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

#include <stdlib.h>  // malloc

KPERFDATA_START_CPP_NAMESPACE

kpdecode_cursor* kpdecode_cursor_create() { return calloc(1, sizeof(kpdecode_cursor)); }

void kpdecode_cursor_free(kpdecode_cursor* cursor) { free(cursor); }

long kpdecode_cursor_setchunk(kpdecode_cursor* cursor, const char* bytes, size_t size) {
  if (cursor->buffer == NULL) {
    cursor->buffer = bytes;
    cursor->unknown_28 = 0;
    cursor->buffer_size = size;
    cursor->unknown_28 = size;
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
        return first_record->unknown_14B8;
      } else {
        return cursor->size_of_kd_buf *
               cursor->thread_count;  // TODO: supposed to be: cursor->size_of_threadmap *
                                      // cursor->thread_count
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

long kpdecode_cursor_next_record(kpdecode_cursor* cursor, kpdecode_record* record) {
  return -1;  // TODO
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

KPERFDATA_END_CPP_NAMESPACE
