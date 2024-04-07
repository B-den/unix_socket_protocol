#pragma once

#include "srcs/crcspeed/crc64speed.h"
#include <inttypes.h>

inline uint64_t checksum(uint64_t crc /*0*/, const void* data, uint64_t len) {
    return crc64(crc, data, len);
}
