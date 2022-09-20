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

#endif  // KPERFDATA_INCLUDE_MACROS_H_
