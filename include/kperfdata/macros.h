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

#ifndef KPERFDATA_INCLUDE_MACROS_H_
#define KPERFDATA_INCLUDE_MACROS_H_

#if defined(__cplusplus)
#define KPERFDATA_START_CPP_NAMESPACE \
  namespace kperfdata {               \
  extern "C" {
#define KPERFDATA_END_CPP_NAMESPACE \
  }                                 \
  }
#else
#define KPERFDATA_START_CPP_NAMESPACE
#define KPERFDATA_END_CPP_NAMESPACE
#endif

#if defined(_WIN32)
#ifdef KPERFDATA_LIBRARY_IMPL
#define KPERFDATA_EXPORT __declspec(dllexport)
#else
#define KPERFDATA_EXPORT __declspec(dllimport)
#endif
#else
#define KPERFDATA_EXPORT
#endif

#define KPERFDATA_RET_OK 0
#define KPERFDATA_RET_FAIL -1

#define KPERFDATA_MAX_RECORDS 10000

#define KPERFDATA_SIZEOF_RAW_HEADER_V1 0x18
#define KPERFDATA_SIZEOF_RAW_HEADER_V2 0x120

#define KPERFDATA_SIZEOF_KD_THREADMAP_32 0x1c
#define KPERFDATA_SIZEOF_KD_THREADMAP_64 0x20
#define KPERFDATA_SIZEOF_KD_BUF_32 0x20
#define KPERFDATA_SIZEOF_KD_BUF_64 0x40

#define KPERFDATA_RAW_VERSION1 0x55aa0101
#define KPERFDATA_RAW_VERSION2 0x55aa0200
#define KPERFDATA_IS_64BIT 0x1

#define KPERFDATA_PAGE_SIZE 4096
#define KPERFDATA_PAGE_MASK (KPERFDATA_PAGE_SIZE - 1)
#define KPERFDATA_PAGE_ALIGN(s) (((s) + KPERFDATA_PAGE_MASK) & ~(KPERFDATA_PAGE_MASK))

#define KPERFDATA_STATE_HEADER_NOT_DECODED 0
#define KPERFDATA_STATE_32_BIT_HEADER 1
#define KPERFDATA_STATE_64_BIT_HEADER 2

#define KPERFDATA_DEBUGID(class, subclass, code, func)                        \
  (((unsigned)((class) & 0xff) << 24) | ((unsigned)((subclass)&0xff) << 16) | \
   ((unsigned)((code)&0x3fff) << 2) | func)

#define KPERFDATA_TIMESTAMP_MASK 0x00ffffffffffffffULL
#define KPERFDATA_CPU_MASK 0xff00000000000000ULL
#define KPERFDATA_CPU_SHIFT 56

#endif  // KPERFDATA_INCLUDE_MACROS_H_
