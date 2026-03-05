// File: src/bin_writer.cpp
#include "imu_logger/writer.hpp"
#include "imu_logger/crc32.hpp"
#include "imu_logger/packet.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace imu_logger
{

    // Simple binary file format (little-endian on disk for convenience):
    // File header:
    //   char magic[8] = "IMULOG1\0"
    //   uint32_t version = 1
    //   uint32_t reserved = 0
    //
    // Repeating record:
    //   uint64_t t_unix_ns
    //   uint32_t sender_ipv4_be
    //   uint16_t sender_port_be
    //   uint16_t payload_len
    //   uint32_t crc32_payload
    //   uint32_t reserved2 = 0
    //   uint8_t  payload[payload_len]
    //
    // MATLAB can fread these fields easily and verify crc32 over payload.

    class BinWriter final : public Writer
    {
    public:
        BinWriter(std::string outdir,
                  std::string prefix,
                  int rotate_min,
                  bool strict_size,
                  bool enable_crc)
            : outdir_(std::move(outdir)),
              prefix_(std::move(prefix)),
              rotate_min_(rotate_min),
              strict_size_(strict_size),
              enable_crc_(enable_crc) {}

        bool open() override
        {
            fs::create_directories(outdir_);
            rotate_start_ns_ = 0;
            open_new_file(/*now*/ 0);
            return out_.is_open();
        }

        void write_record(const PacketMeta &meta,
                          const unsigned char *payload,
                          std::size_t payload_len) override
        {
            if (!out_.is_open())
                return;

            if (payload_len != meta.payload_len)
                return;
            if (strict_size_ && payload_len != imu_logger::kPacketBytes)
                return;

            const uint32_t c = enable_crc_ ? crc32(payload, payload_len) : 0u;
            const uint32_t reserved2 = 0u;

            // write fixed fields
            out_.write(reinterpret_cast<const char *>(&meta.t_unix_ns), sizeof(meta.t_unix_ns));
            out_.write(reinterpret_cast<const char *>(&meta.sender_ipv4_be), sizeof(meta.sender_ipv4_be));
            out_.write(reinterpret_cast<const char *>(&meta.sender_port_be), sizeof(meta.sender_port_be));
            out_.write(reinterpret_cast<const char *>(&meta.payload_len), sizeof(meta.payload_len));
            out_.write(reinterpret_cast<const char *>(&c), sizeof(c));
            out_.write(reinterpret_cast<const char *>(&reserved2), sizeof(reserved2));

            // write payload bytes
            out_.write(reinterpret_cast<const char *>(payload), static_cast<std::streamsize>(payload_len));
        }

        void rotate_if_needed(uint64_t now_unix_ns) override
        {
            if (rotate_min_ <= 0)
                return;
            if (rotate_start_ns_ == 0)
            {
                rotate_start_ns_ = now_unix_ns;
                return;
            }
            const uint64_t rotate_ns = uint64_t(rotate_min_) * 60ull * 1000000000ull;
            if (now_unix_ns - rotate_start_ns_ >= rotate_ns)
            {
                open_new_file(now_unix_ns);
            }
        }

        void close() override
        {
            if (out_.is_open())
            {
                out_.flush();
                out_.close();
            }
        }

        std::string current_path() const override { return current_path_; }

    private:
        void write_file_header()
        {
            const char magic[8] = {'I', 'M', 'U', 'L', 'O', 'G', '1', '\0'};
            const uint32_t version = 1;
            const uint32_t reserved = 0;
            out_.write(magic, sizeof(magic));
            out_.write(reinterpret_cast<const char *>(&version), sizeof(version));
            out_.write(reinterpret_cast<const char *>(&reserved), sizeof(reserved));
        }

        void open_new_file(uint64_t now_unix_ns)
        {
            close();

            const std::string fname = prefix_ + "_" + std::to_string(now_unix_ns) + ".bin";
            fs::path p = fs::path(outdir_) / fname;
            current_path_ = p.string();

            out_.open(p, std::ios::binary | std::ios::app);

            // If file is new/empty, write header
            if (out_)
            {
                // Check size
                std::error_code e;
                auto sz = fs::exists(p, e) ? fs::file_size(p, e) : 0;
                if (!e && sz == 0)
                    write_file_header();
            }

            rotate_start_ns_ = now_unix_ns;
        }

        std::string outdir_;
        std::string prefix_;
        int rotate_min_;
        bool strict_size_;
        bool enable_crc_;

        std::ofstream out_;
        uint64_t rotate_start_ns_{0};
        std::string current_path_;
    };

    // factory
    std::unique_ptr<Writer> make_bin_writer(std::string outdir,
                                            std::string prefix,
                                            int rotate_min,
                                            bool strict_size,
                                            bool enable_crc)
    {
        return std::make_unique<BinWriter>(std::move(outdir), std::move(prefix), rotate_min, strict_size, enable_crc);
    }

} // namespace imu_logger