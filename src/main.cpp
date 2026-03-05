// File: src/main.cpp
#include <boost/asio.hpp>
#include <filesystem>
#include <iostream>
#include <vector>
#include <csignal>
#include <chrono>
#include <thread>

#include "imu_logger/writer.hpp"
#include "imu_logger/packet.hpp"
#include "args.hpp"

static uint64_t unix_now_ns()
{
    using namespace std::chrono;
    return (uint64_t)duration_cast<nanoseconds>(
               system_clock::now().time_since_epoch())
        .count();
}

uint64_t packet_count = 0;
uint64_t last_print = unix_now_ns();
// factories implemented in cpp files
namespace imu_logger
{
    std::unique_ptr<Writer> make_text_writer(std::string outdir, std::string prefix, int rotate_min, bool strict_size, int precision);
    std::unique_ptr<Writer> make_bin_writer(std::string outdir, std::string prefix, int rotate_min, bool strict_size, bool enable_crc);
}

using boost::asio::ip::udp;
using boost::system::error_code;

static volatile std::sig_atomic_t g_stop = 0;
static void handle_sigint(int) { g_stop = 1; }

int main(int argc, char **argv)
{
    imu_logger::Args args;
    if (!imu_logger::parse_args(argc, argv, args))
    {
        // parse_args prints help on --help; return nonzero if it was an error
        // If --help, it returns false too; that's fine.
        return 1;
    }

    // Validate bind IP
    error_code ec;
    auto addr_v4 = boost::asio::ip::make_address_v4(args.bind, ec);
    if (ec)
    {
        std::cerr << "Invalid IPv4 bind address: " << args.bind << " (" << ec.message() << ")\n";
        return 1;
    }

    // Choose writer
    std::unique_ptr<imu_logger::Writer> writer;
    if (args.mode == "text")
    {
        writer = imu_logger::make_text_writer(args.outdir, args.prefix, args.rotate_min, args.strict_size, args.precision);
    }
    else
    {
        writer = imu_logger::make_bin_writer(args.outdir, args.prefix, args.rotate_min, args.strict_size, args.crc32);
    }

    if (!writer->open())
    {
        std::cerr << "Failed to open writer output.\n";
        return 1;
    }

    std::signal(SIGINT, handle_sigint);

    boost::asio::io_context io;
    udp::endpoint local_ep(addr_v4, static_cast<unsigned short>(args.port));
    udp::socket sock(io);
    sock.open(udp::v4());
    sock.bind(local_ep, ec);
    if (ec)
    {
        std::cerr << "Bind failed: " << args.bind << ":" << args.port << " -> " << ec.message() << "\n";
        return 1;
    }

    // ✅ Non-blocking receive so Ctrl+C is reliable on macOS
    sock.non_blocking(true);

    std::vector<unsigned char> buf(imu_logger::kPacketBytes);
    udp::endpoint sender;

    std::cout << "Bound to " << args.bind << ":" << args.port << "\n";
    std::cout << "Mode: " << args.mode << "\n";
    std::cout << "Output: " << writer->current_path() << "\n";
    std::cout << "Press Ctrl+C to stop.\n";

    int bad_size_warned = 0;

    while (!g_stop)
    {
        const uint64_t now_ns = unix_now_ns();
        writer->rotate_if_needed(now_ns);

        std::size_t n = sock.receive_from(boost::asio::buffer(buf), sender, 0, ec);

        if (ec == boost::asio::error::would_block || ec == boost::asio::error::try_again)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        if (ec)
        {
            if (g_stop)
                break;
            std::cerr << "Receive error: " << ec.message() << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        if (args.strict_size && n != imu_logger::kPacketBytes)
        {
            if (bad_size_warned < 10)
            {
                std::cerr << "Got " << n << " bytes from " << sender.address().to_string()
                          << " (expected " << imu_logger::kPacketBytes << "), dropping.\n";
                if (bad_size_warned == 9)
                    std::cerr << "(Suppressing further wrong-size warnings...)\n";
            }
            bad_size_warned++;
            continue;
        }

        imu_logger::PacketMeta meta{};
        meta.t_unix_ns = now_ns;

        // sender address -> store as IPv4 BE (network order)
        if (sender.address().is_v4())
        {
            auto b = sender.address().to_v4().to_bytes();
            meta.sender_ipv4_be = (uint32_t(b[0]) << 24) | (uint32_t(b[1]) << 16) | (uint32_t(b[2]) << 8) | uint32_t(b[3]);
        }
        else
        {
            meta.sender_ipv4_be = 0;
        }

        const uint16_t sp = static_cast<uint16_t>(sender.port());
        meta.sender_port_be = uint16_t((sp >> 8) | (sp << 8));
        meta.payload_len = static_cast<uint16_t>(n);

        writer->write_record(meta, buf.data(), n);

        static bool first_packet = true;

        if (first_packet)
        {
            std::cout << "First packet received from "
                      << sender.address().to_string()
                      << ":" << sender.port() << "\n";
            first_packet = false;
        }

        packet_count++;

        uint64_t now = unix_now_ns();
        if (now - last_print > 1000000000ULL)
        {
            std::cout << "Packets received: " << packet_count << "\n";
            last_print = now;
        }
    }

    writer->close();
    sock.close();
    io.stop();

    std::cout << "Stopped.\n";
    return 0;
}