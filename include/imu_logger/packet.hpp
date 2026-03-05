// File: include/imu_logger/packet.hpp
#pragma once
#include <array>
#include <cstdint>
#include <cstring>

namespace imu_logger
{

    // 73 doubles per packet, big-endian in UDP payload
    inline constexpr int kNumValues = 73;
    inline constexpr std::size_t kPacketBytes = kNumValues * 8;

    inline uint64_t load_u64_be(const unsigned char *p)
    {
        return (uint64_t(p[0]) << 56) | (uint64_t(p[1]) << 48) | (uint64_t(p[2]) << 40) | (uint64_t(p[3]) << 32) |
               (uint64_t(p[4]) << 24) | (uint64_t(p[5]) << 16) | (uint64_t(p[6]) << 8) | (uint64_t(p[7]) << 0);
    }

    inline double u64_to_double(uint64_t u)
    {
        double d;
        std::memcpy(&d, &u, sizeof(d));
        return d;
    }

    inline std::array<double, kNumValues> decode_doubles_be(const unsigned char *buf)
    {
        std::array<double, kNumValues> values{};
        for (int j = 0; j < kNumValues; ++j)
        {
            const unsigned char *p = buf + j * 8;
            const uint64_t u = load_u64_be(p);
            values[j] = u64_to_double(u);
        }
        return values;
    }

} // namespace imu_logger