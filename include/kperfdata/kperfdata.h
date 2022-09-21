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

#ifndef KPERFDATA_INCLUDE_KPERFDATA_H_
#define KPERFDATA_INCLUDE_KPERFDATA_H_

#include <stddef.h>  // size_t
#include <stdint.h>  // uint32_t, uint64_t

#include "kperfdata/macros.h"

KPERFDATA_START_CPP_NAMESPACE

// clang-format off

/**
 * RAW Version 1:
 *
 * RAW_header, with version_no set to RAW_VERSION1
 * kd_threadmap[]
 * Empty space to pad alignment to the nearest page boundary.
 * kd_buf[]
 */
typedef struct {
  int version_no;                                     // +0x00, size=0x04
  int thread_count;                                   // +0x04, size=0x04
  uint64_t TOD_secs;                                  // +0x08, size=0x08
  uint32_t TOD_usecs;                                 // +0x10, size=0x04
  // padding                                          // +0x14, size=0x04
} RAW_header_v1;                                      // size=0x18

/**
 * RAW Version 2:
 *
 * RAW_header, with version_no set to RAW_VERSION2
 * kd_threadmap[]
 * Empty space to pad alignment to the nearest page boundary.
 * kd_buf[]
 */
typedef struct {
  int version_no;                                     // +0x00, size=0x04
  int thread_count;                                   // +0x04, size=0x04
  uint64_t TOD_secs;                                  // +0x08, size=0x08
  uint32_t TOD_usecs;                                 // +0x10, size=0x04
  uint32_t flags;                                     // +0x14, size=0x04, 0: 32-bit, 1: 64-bit
  uint64_t frequency;                                 // +0x18, size=0x08
  // padding:                                         // +0x20, size=0x100
} RAW_header_v2;                                      // size=0x120

/**
 * threadmap (32-bit)
 */
typedef struct {
  uint32_t thread;                                    // +0x00, size=0x04
  int valid;                                          // +0x04, size=0x04, 0: invalid, 1: kernel_task, otherwise the PID
  char command[20];                                   // +0x08, size=0x14, process name
} kd_threadmap_32;                                    // size=0x1c, align=0x04

/**
 * threadmap (64-bit)
 */
typedef struct {
  uint64_t thread;                                    // +0x00, size=0x08, the thread ID
  int valid;                                          // +0x08, size=0x04, 0: invalid, 1: kernel_task, otherwise the PID
  char command[20];                                   // +0x0c, size=0x14, process name
} kd_threadmap_64;                                    // size=0x20, align=0x08

/**
 * kd_buf (32-bit)
 */
typedef struct {
  uint64_t timestamp;                                 // +0x00, size=0x08
  uint32_t arg1;                                      // +0x08, size=0x04
  uint32_t arg2;                                      // +0x0c, size=0x04
  uint32_t arg3;                                      // +0x10, size=0x04
  uint32_t arg4;                                      // +0x14, size=0x04
  uint32_t arg5; /* the thread ID */                  // +0x18, size=0x04
  uint32_t debugid;                                   // +0x1c, size=0x04
} kd_buf_32;                                          // size=0x20(32)

/**
 * kd_buf (64-bit)
 */
typedef struct {
  uint64_t timestamp;                                 // +0x00, size=0x08
  uint64_t arg1;                                      // +0x08, size=0x08
  uint64_t arg2;                                      // +0x10, size=0x08
  uint64_t arg3;                                      // +0x18, size=0x08
  uint64_t arg4;                                      // +0x20, size=0x08
  uint64_t arg5; /* the thread ID */                  // +0x28, size=0x08
  uint32_t debugid;                                   // +0x30, size=0x04
  uint32_t cpuid;                                     // +0x34, size=0x04
  uint64_t unused;                                    // +0x38, size=0x08
} kd_buf_64;                                          // size=0x40(64)

typedef kd_buf_64 kd_buf;

/**
 * kprecord
 *
 * Objective-C Type Encodings:
 * refs: https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/ObjCRuntimeGuide/Articles/ocrtTypeEncodings.html#//apple_ref/doc/uid/TP40008048-CH100
 * type string:
 * v16@?0^{
 * kpdecode_record=
 *  Q                       // field_1
 *  Q
 *  Q
 *  i
 *  (?={?=[20c]}{?=[20c]})  // field_5
 *  {?=I[4Q]}
 *  {?=iiQ}
 *  {kpdecode_callstack=II[256Q]}
 *  {kpdecode_callstack=II[256Q]}
 *  {kpdecode_pmc=i[32Q]}   // field_10
 *  {?=IIII}
 *  {?=IQQQQ}
 *  {?=QQIssb3b3b3b3}
 *  {?=QiiQQ}
 *  {?=QQsC}                // field_15
 *  {?=Q}
 *  {?=II}
 *  {?=Qi}
 *  {?=i^Q}
 *  {?=Q}                   // field_20
 *  {?=Ii}
 *  {?=[256c]QQI}
 *  {?=QQQQ}
 *  {?=QQ}
 *  {?=b3b3b3}              // field_25
 *  {?=[64c]Q}
 *  {?=[64c]Q}
 *  {?={?=ISi}QQ}
 *  {?=QQQQQQQ}
 *  {?=III}                 // field_30
 * }8
 * size=0x1498(5272)
 */
typedef struct {
  unsigned int flags;                                 // +0x00, size=0x04
  unsigned int nframes;                               // +0x04, size=0x04
  unsigned long long frames[256];                     // +0x08, size=0x800, #define MAX_UCALLSTACK_FRAMES 256
} kpdecode_callstack;                                 // size=0x808,

typedef kpdecode_callstack kp_ucallstack;
typedef kpdecode_callstack kp_kcallstack;

typedef struct {
  int counterc;                                       // +0x00, size=0x04
  // padding                                          // +0x04, size=0x04
  unsigned long long counterv[32];                    // +0x08, size=0x100, #define KPC_MAX_COUNTERS 32
} kpdecode_pmc;                                       // size=0x108, kpcdata

typedef struct {
  unsigned long long flags;                           // +0x00, size=0x08
  unsigned long long timestamp;                       // +0x08, size=0x08
  unsigned long long tid;                             // +0x10, size=0x08
  int cpuid;                                          // +0x18, size=0x04
  union {
      struct {
          char task_name[20];
      } _field1;
      struct {
          char task_name[20];                         // +0x1C, size=0x14, kd_task_name, PERF_TI_XSAMPLE
      } _field2;
  } task_info;
  struct {
      unsigned int debugid;                           // +0x30, size=0x04
      // padding:                                     // +0x34, size=0x04
      unsigned long long args[4];                     // +0x38, size=0x08, arg1
                                                      // +0x40, size=0x08, arg2
                                                      // +0x48, size=0x08, arg3
                                                      // +0x50, size=0x08, arg4
  } kd_buf;
  struct {
      int kpthi_pid;                                  // +0x58, size=0x04
      int kpthi_runmode;                              // +0x5C, size=0x04
      unsigned long long kpthi_dq_addr;               // +0x60, size=0x08
  } kperf_thread_info;
  kp_ucallstack ucallstack;                           // +0x68    size=0x808
  //    unsigned int flags;                           // +0x68,   size=0x04
  //    unsigned int nframes;                         // +0x6c,   size=0x04
  //    unsigned long long frames[256];               // +0x70,   size=0x800
  kpdecode_callstack kcallstack;                      // +0x870,  size=0x808
  //    unsigned int flags;                           // +0x870,  size=0x04
  //    unsigned int nframes;                         // +0x874,  size=0x04 
  //    unsigned long long frames[256];               // +0x878,  size=0x800 
  kpdecode_pmc pmc_counters;                          // +0x1078, size=0x108
  //    int counterc;                                 // +0x1078, size=0x04
  //    padding                                       // +0x107c, size=0x04
  //    unsigned long long counterv[32];              // +0x1080, size=0x100
  struct {
      unsigned int running;                           // +0x1180, size=0x04
      unsigned int kpc_get_counter_count;             // +0x1184, size=0x04 
      unsigned int counterc;                          // +0x1188, size=0x04 
      unsigned int configc;                           // +0x118C, size=0x04
  } pmc_config;   
  struct {
      unsigned int flag;                              // +0x1190, size=0x04, flag, 001: phys_footprint=-1, 010: purgeable_volatile=-1, 100: purgeable_volatile_compressed=-1
      // padding                                      // +0x1194, size=0x04
      unsigned long long phys_footprint;              // +0x1198, size=0x08
      unsigned long long purgeable_volatile;          // +0x11A0, size=0x08
      unsigned long long purgeable_volatile_compressed; // +0x11A8, size=0x08
      unsigned long long unknown_field5;              // +0x11B0, size=0x08, unused?
  } meminfo;
  struct { 
      unsigned long long kpthsc_user_time;            // +0x11B8, size=0x08
      unsigned long long kpthsc_system_time;          // +0x11C0, size=0x08, TODO: kpthsc_runnable_time?
      unsigned int kpthsc_state;                      // +0x11C8, size=0x04
      short kpthsc_base_priority;                     // +0x11CC, size=0x02 
      short kpthsc_sched_priority;                    // +0x11CE, size=0x02
      unsigned int kpthsc_effective_qos :3;           // +0x11D0, size=0x01
      unsigned int kpthsc_requested_qos :3;           // +0x11D1, size=0x01
      unsigned int kpthsc_requested_qos_override :3;  // +0x11D2, size=0x01
      unsigned int kpthsc_requested_qos_promote :3;   // +0x11D3, size=0x01
      // padding                                      // +0x11D4, size=0x04
  } kperf_thread_scheduling;
  struct {
      unsigned long long kptksn_flags;                // +0x11D8, size=0x08
      int kptksn_suspend_count;                       // +0x11E0, size=0x04
      int kptksn_pageins;                             // +0x11E4, size=0x04
      unsigned long long user_time_in_terminated_threads;             // +0x11E8, size=0x08
      unsigned long long kptksn_system_time_in_terminated_threads;    // +0x11F0, size=0x08 
    } kperf_task_snapshot; 
  struct { 
      unsigned long long kpthsn_flags;                // +0x11F8, size=0x08, why use Q for uint8_t?
      unsigned long long kpthsn_last_made_runnable_time; // +0x1200, size=0x08
      short kpthsn_suspend_count;                     // +0x1208, size=0x02, 
      unsigned char kpthsn_io_tier;                   // +0x120A, size=0x01, 
      // padding                                      // +0x120B, size=0x05
  } _fielkperf_thread_snapshotd15; 
  struct {
      unsigned long long kpthdi_dq_serialno;          // +0x1210, size=0x08
  } kperf_thread_dispatch; // 
  struct {
      unsigned int actionid;                          // +0x1218, size=0x04
      unsigned int userdata;                          // +0x121C, size=0x04, userdata?
  } kperf_sample_args;
  struct {
      unsigned long long tid;                         // +0x1220, size=0x08
      int pid;                                        // +0x1228, size=0x04
      // padding                                      // +0x122C, size=0x04
  } kperf_cswitch;
  struct {
      int unknown_field1;                             // +0x1230, size=0x04, arg1?
      // padding                                      // +0x1234, size=0x04, -
      unsigned long long *unknown_field2;             // +0x1238, size=0x08, malloc(8 * arg1), used by SubClass: 153
    } unknown_field19;
  struct { 
      unsigned long long unknown_field1;              // +0x1240, size=0x08, cursor+0x8C8 cpuid unknown map?
  } unknown_field20;
  struct {
      unsigned int eventid;                           // +0x1248, size=0x04
      int function_qualifier;                         // +0x124C, size=0x04 
  } debugid;
  struct {
      char unknown_field1[256];                       // +0x1250, size=0x100, TRACE_STRING_GLOBAL?
      unsigned long long unknown_field2;              // +0x1350, size=0x08
      unsigned long long unknown_field3;              // +0x1358, size=0x08
      unsigned int unknown_field4;                    // +0x1360, size=0x04
      // padding                                      // +0x1364, size=0x04
  } unknown_field22;
  struct {
      unsigned long long vm_page_free_count;          // +0x1368, size=0x08
      unsigned long long vm_page_wire_count;          // +0x1370, size=0x08
      unsigned long long vm_page_external_count;      // +0x1378, size=0x08
      unsigned long long vm_page_total_count;         // +0x1380, size=0x08, vm_page_active_count + vm_page_inactive_count + vm_page_speculative_count
  } system_meminfo1; 
  struct {
      unsigned long long mt_core_instrs;              // +0x1388, size=0x08
      unsigned long long mt_core_cycles;              // +0x1390, size=0x08
  } kperf_thread_instrs_cycles;
  struct {
      unsigned int kpthsc_requested_qos_promote :3;   // +0x1398, size=0x01, kpthsc_requested_qos_promote
      unsigned int kpthsc_requested_qos_kevent_override :3;   // +0x1399, size=0x01, kpthsc_requested_qos_kevent_override
      unsigned int unknown_field3 :3;                 // +0x139A, size=0x01, arg3[23~20](3-bit)?
      // padding                                      // +0x139B, size=0x05
  } kperf_thread_scheduling_ext;
  struct {
      char unknown_field1[64];                        // +0x13A0, size=0x40
      unsigned long long unknown_field2;              // +0x13E0, size=0x08
  } unknown_field26;
  struct {
      char unknown_field1[64];                        // +0x13E8, size=0x40, thread name?, // #define STACKSHOT_MAX_THREAD_NAME_SIZE 64
      unsigned long long unknown_field2;              // +0x1428, size=0x08, strlen(thread_name)?
  } unknown_field27;
  struct {
      struct {
          unsigned int config;                        // +0x1430, size=0x04
          unsigned short counter;                     // +0x1434, size=0x02
          // padding                                  // +0x1436, size=0x02
          int flags;                                  // +0x1438, size=0x04
          // padding                                  // +0x143c, size=0x04
      } kperf_kpc_desc; 
      unsigned long long count;                       // +0x1440, size=0x08
      unsigned long long pc;                          // +0x1448, size=0x08
  } kperf_kpc_hndlr;
  struct {
      unsigned long long vm_page_anonymous_count;     // +0x1450, size=0x08
      unsigned long long vm_page_internal_count;      // +0x1458, size=0x08
      unsigned long long vm_pageout_compressions;     // +0x1460, size=0x08
      unsigned long long vm_page_compressor_count;    // +0x1468, size=0x08
      unsigned long long unknown_field5;              // +0x1470, size=0x08, arg1?
      unsigned long long unknown_field6;              // +0x1478, size=0x08, arg2?
      unsigned long long unknown_field7;              // +0x1480, size=0x08, arg3?
  } system_meminfo_ext;
  struct {
      unsigned int callstack_nframes;                 // +0x1488, size=0x04, 
      unsigned int unknown_field2;                    // +0x1488, size=0x04, callstack_hdr_arg3?
      unsigned int unknown_field3;                    // +0x1490, size=0x04, callstack_hdr_arg4?
  } xcallstack_hdr;
  // end of type string

  uint32_t ready;                                     // +0x1498, size=?, whether this record is ready(1) or not(0)
  // ...
  void* next;                                         // +0x14A0, size=0x08, the next item of linked list
  // +0x14A8, size=0x04, current_kernel_callstack_count
  // +0x14AC, size=0x04, current_user_callstack_count
  // +0x14B0, pmc_counters_count
  // +0x14B4, TODO:
  unsigned long long total_size_of_kevents;           // +0x14B8, cursor.size_of_kd_buf * cursor.kevent_count

} kpdecode_record; // size= 0x14C0

/**
 * kpdecode_cursor
 */
typedef struct {
  uint32_t state;                                     // +0x00(00), size=0x04, value=0: Initial, 1: 32-bit, 2: 64-bit
  // ...
  uint32_t size_of_kd_buf;                            // +0x08(08), size=0x04, value=0x20 on 32-bit, 0x40 on 64-bit
  uint32_t size_of_kd_threadmap;                      // +0x10(16), size=0x04, value=0x1c on 32-bit, 0x20 on 64-bit
  // ...
  const char* buffer;                                 // +0x18(24), size=0x08, the 2nd argument of kpdecode_cursor_setchunk()
  uint64_t buffer_size;                               // +0x20(32), size=0x08, the 3rd argument of kpdecode_cursor_setchunk()
  uint64_t unknown_28;                                // +0x28(40), size=0x08, offset?
  uint64_t buffer_size1;                              // +0x30(48), size=0x08, buffer_size
  // ...
  uint32_t header_decoded;                            // +0x40(64), size=0x04, value=0/1, whether the header has been decoded(1) or not(0)
  // ...
  char** buffer_ptr;                                  // +0x48(72), size=0x08, pointer to the buffer pointer
  char* cur_kd_buf_ptr;                               // +0x50(80), size=0x08, pointer to the current kd_buf
  char* cur_kd_threadmap_ptr;                         // +0x58(88), size=0x08, pointer to the current kd_threadmap
  char* end_kd_threadmap_ptr;                         // +0x60(96), size=0x08, pointer to the end of kd_threadmap
  kd_buf_64 kd_buf; // {
  //  uint64_t timestamp;                             // +0x68(104), size=0x08
  //  uint64_t arg1;                                  // +0x70(112), size=0x08
  //  uint64_t arg2;                                  // +0x78(120), size=0x08
  //  uint64_t arg3;                                  // +0x80(128), size=0x08
  //  uint64_t arg4;                                  // +0x88(136), size=0x08
  //  uint64_t tid;                                   // +0x90(144), size=0x08, arg5(tid)
  //  uint32_t debugid;                               // +0x98(152), size=0x04
  //  uint32_t cpuid;                                 // +0x9c(156), size=0x04
  //  uint64_t unused;                                // +0xa0(160), size=0x08
  //}
  uint64_t threadmap_decoded;                         // +0xA8(168),   size=0x08,  value=0/1, , whether the threadmap has been decoded(1) or not(0)
  kpdecode_record* kpdeocde_record_head;              // +0xB0(176),   size=0x08,  pointer to the first kpdecode_record
  kpdecode_record* kpdecode_record_tail;              // +0xB8(184),   size=0x08,  pointer to the last  kpdecode_record
  uint32_t kevent_count;                              // +0xC0(192),   size=0x04,  count of the kevents
  uint32_t kpdecode_record_count;                     // +0xC4(196),   size=0x04,  size of kpdecode_records
  uint64_t unknown_c8[64];                            // +0x0C8(200),  size=0x200, thread info sched map
  uint64_t unknown_2c8[64];                           // +0x2C8(712),  size=0x200, cpuid string1 map, global string(TRACE_STRING_GLOBAL)?
  uint64_t unknown_4c8[64];                           // +0x4C8(1224), size=0x200, cpuid string2 map
  uint64_t unknown_6c8[64];                           // +0x6C8(1736), size=0x200, cpuid string3 map, thread name?
  uint64_t unknown_8c8[64];                           // +0x8C8(2248), size=0x200, unknown bitmap, record+0x1240, timestamp?
  // ...
  // ...
  uint32_t unknown_cc8;                               // +0xCC8(3272), size=0x04?, the max number of ?
  // ...
  uint32_t unknown_option;                            // +0xCDC(3292), size=0x04, value=0/1
} kpdecode_cursor;                                    // sizeof=0xce0(3296)

// clang-format on

/**
 * Create a new cursor to decode kperf data
 *
 * @return the new cursor
 */
KPERFDATA_EXPORT kpdecode_cursor* kpdecode_cursor_create();

/**
 * Release the cursor
 *
 * @param cursor the cursor
 */
KPERFDATA_EXPORT void kpdecode_cursor_free(kpdecode_cursor* cursor);

/**
 * Set a chunk buffer to the cursor
 *
 * @param cursor the cursor
 * @param bytes chunk buffer
 * @param size size of buffer
 * @return ret: 0 for success, -1 for failure
 */
KPERFDATA_EXPORT long kpdecode_cursor_setchunk(kpdecode_cursor* cursor, const char* bytes,
                                               size_t size);

/**
 * Clear the chunk buffer of the cursor
 *
 * @param cursor the cursor
 * @return the pointer of the chunk that was just cleared, or NULL
 */
KPERFDATA_EXPORT char* kpdecode_cursor_clearchunk(kpdecode_cursor* cursor);

/**
 * Get the stats of the cursor
 *
 * @param cursor the cursor
 * @param arg2 unknown, value=0/1
 * @return the stats
 */
KPERFDATA_EXPORT long kpdecode_cursor_get_stats(kpdecode_cursor* cursor, int arg2);

/**
 * Set the option of the cursor
 *
 * @param cursor the cursor
 * @param arg2 unknown, value=0/1, 0: do nothing, 1: set option
 * @param arg3 unknown, value=0/1
 * @return the old value of this option
 */
KPERFDATA_EXPORT long kpdecode_cursor_set_option(kpdecode_cursor* cursor, int arg2, long arg3);

/**
 * Get the next record of the cursor
 *
 * @param cursor the cursor
 * @param record the next record
 * @return ret: 0 for success, otherwise for failure
 */
KPERFDATA_EXPORT long kpdecode_cursor_next_record(kpdecode_cursor* cursor, kpdecode_record* record);

/**
 * Release the record
 *
 * @param record the record
 */
KPERFDATA_EXPORT void kpdecode_record_free(kpdecode_record* record);

/**
 * Flush the cursor
 */
KPERFDATA_EXPORT void kpdecode_cursor_flush();

KPERFDATA_END_CPP_NAMESPACE

#endif  // KPERFDATA_INCLUDE_KPERFDATA_H_
