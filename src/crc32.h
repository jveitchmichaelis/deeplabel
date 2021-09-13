/* Copyright 2015 The TensorFlow Authors. All Rights Reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef CRC32_H
#define CRC32_H

#include <cstdint>
#include <stddef.h>

namespace tf_crc{

    // Return the crc32c of concat(A, data[0,n-1]) where init_crc is the
    // crc32c of some string A.  Extend() is often used to maintain the
    // crc32c of a stream of data.
    extern uint32_t Extend(uint32_t init_crc, const char* data, size_t n);

    // Return the crc32c of data[0,n-1]
    inline uint32_t Value(const char* data, size_t n) { return Extend(0, data, n); }

    #if defined(PLATFORM_GOOGLE)
    extern uint32_t Extend(uint32_t init_crc, const absl::Cord& cord);
    inline uint32_t Value(const absl::Cord& cord) { return Extend(0, cord); }
    #endif

    static const uint32_t kMaskDelta = 0xa282ead8ul;

    // Return a masked representation of crc.
    //
    // Motivation: it is problematic to compute the CRC of a string that
    // contains embedded CRCs.  Therefore we recommend that CRCs stored
    // somewhere (e.g., in files) should be masked before being stored.
    inline uint32_t Mask(uint32_t crc) {
      // Rotate right by 15 bits and add a constant.
      return ((crc >> 15) | (crc << 17)) + kMaskDelta;
    }

    // Return the crc whose masked representation is masked_crc.
    inline uint32_t Unmask(uint32_t masked_crc) {
      uint32_t rot = masked_crc - kMaskDelta;
      return ((rot >> 17) | (rot << 15));
    }
}

#endif  // TENSORFLOW_CORE_LIB_HASH_CRC32C_H_
