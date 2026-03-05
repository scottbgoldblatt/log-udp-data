// File: include/imu_logger/crc32.hpp
#pragma once
#include <cstdint>
#include <cstddef>

namespace imu_logger
{

    // Simple CRC32 (IEEE 802.3 polynomial 0xEDB88320) over bytes.
    // Good enough for integrity checking file records.
    inline uint32_t crc32(const void *data, std::size_t len)
    {
        const auto *p = static_cast<const uint8_t *>(data);
        uint32_t crc = 0xFFFFFFFFu;

        for (std::size_t i = 0; i < len; ++i)
        {
            crc ^= p[i];
            for (int k = 0; k < 8; ++k)
            {
                uint32_t mask = -(crc & 1u);
                crc = (crc >> 1) ^ (0xEDB88320u & mask);
            }
        }
        return ~crc;
    }

} // namespace imu_logger