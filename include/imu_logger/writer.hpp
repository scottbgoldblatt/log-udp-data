// File: include/imu_logger/writer.hpp
#pragma once
#include <cstdint>
#include <string>

namespace imu_logger
{

    struct PacketMeta
    {
        uint64_t t_unix_ns;      // monotonic wall-ish timestamp in ns since Unix epoch
        uint32_t sender_ipv4_be; // IPv4 address in network byte order (a.b.c.d as uint32)
        uint16_t sender_port_be; // port in network byte order
        uint16_t payload_len;    // bytes received
    };

    // Abstract base writer: text and binary share the same receive loop.
    class Writer
    {
    public:
        virtual ~Writer() = default;

        // Called once after construction to open initial file(s).
        virtual bool open() = 0;

        // Write one datagram record. payload is the exact UDP bytes received.
        virtual void write_record(const PacketMeta &meta,
                                  const unsigned char *payload,
                                  std::size_t payload_len) = 0;

        // Rotation hook: call periodically or each record.
        virtual void rotate_if_needed(uint64_t now_unix_ns) = 0;

        // Flush and close.
        virtual void close() = 0;

        // Path to current output file (for printing).
        virtual std::string current_path() const = 0;
    };

} // namespace imu_logger