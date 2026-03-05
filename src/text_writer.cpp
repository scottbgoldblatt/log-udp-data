// File: src/text_writer.cpp
#include "imu_logger/writer.hpp"
#include "imu_logger/packet.hpp"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>

namespace fs = std::filesystem;

namespace imu_logger
{

    class TextWriter final : public Writer
    {
    public:
        TextWriter(std::string outdir,
                   std::string prefix,
                   int rotate_min,
                   bool strict_size,
                   int precision)
            : outdir_(std::move(outdir)),
              prefix_(std::move(prefix)),
              rotate_min_(rotate_min),
              strict_size_(strict_size),
              precision_(precision) {}

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
            (void)meta;
            if (!out_.is_open())
                return;

            if (payload_len != kPacketBytes)
            {
                if (strict_size_)
                    return;
                // If not strict, we still cannot decode safely; just skip.
                return;
            }

            auto values = decode_doubles_be(payload);

            // Write a simple line: id (0), then 73 doubles
            out_ << 0 << '\t';
            for (double d : values)
                out_ << d << '\t';
            out_ << '\n';
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
        void open_new_file(uint64_t now_unix_ns)
        {
            close();

            // filename with timestamp
            // use ns value to avoid locale/time parsing complexity
            const std::string fname = prefix_ + "_" + std::to_string(now_unix_ns) + ".txt";
            fs::path p = fs::path(outdir_) / fname;
            current_path_ = p.string();

            out_.open(p, std::ios::app);
            out_.setf(std::ios::scientific);
            out_ << std::setprecision(precision_);

            rotate_start_ns_ = now_unix_ns;
        }

        std::string outdir_;
        std::string prefix_;
        int rotate_min_;
        bool strict_size_;
        int precision_;

        std::ofstream out_;
        uint64_t rotate_start_ns_{0};
        std::string current_path_;
    };

    // factory
    std::unique_ptr<Writer> make_text_writer(std::string outdir,
                                             std::string prefix,
                                             int rotate_min,
                                             bool strict_size,
                                             int precision)
    {
        return std::make_unique<TextWriter>(std::move(outdir), std::move(prefix), rotate_min, strict_size, precision);
    }

} // namespace imu_logger