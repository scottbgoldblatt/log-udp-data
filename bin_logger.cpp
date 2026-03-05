// main.cpp (Boost.Asio UDP text logger) — Ctrl+C reliable on macOS, and checks ./build too via scripts
// Key change vs your pasted version: make socket non-blocking + short sleep on would_block,
// so SIGINT is observed even when receive_from would otherwise block forever.

#include <boost/asio.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <array>
#include <cstdint>
#include <cstring>
#include <csignal>
#include <chrono>
#include <thread>
#include <iomanip>

namespace fs = std::filesystem;
using boost::asio::ip::udp;
using boost::system::error_code;

static inline uint64_t load_u64_be(const unsigned char *p)
{
    return (uint64_t(p[0]) << 56) | (uint64_t(p[1]) << 48) | (uint64_t(p[2]) << 40) | (uint64_t(p[3]) << 32) |
           (uint64_t(p[4]) << 24) | (uint64_t(p[5]) << 16) | (uint64_t(p[6]) << 8) | (uint64_t(p[7]) << 0);
}

static inline double u64_to_double(uint64_t u)
{
    double d;
    std::memcpy(&d, &u, sizeof(d));
    return d;
}

static volatile std::sig_atomic_t g_stop = 0;
static void handle_sigint(int) { g_stop = 1; }

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cerr << "Usage: hrg_logger <local_bind_ip> <local_port>\n"
                  << "Example: hrg_logger 0.0.0.0 25001\n";
        return 1;
    }

    const std::string bind_ip = argv[1];
    const std::string port_str = argv[2];

    int port = 0;
    try
    {
        port = std::stoi(port_str);
    }
    catch (...)
    {
        std::cerr << "Invalid port: " << port_str << "\n";
        return 1;
    }
    if (port <= 0 || port > 65535)
    {
        std::cerr << "Port out of range: " << port << "\n";
        return 1;
    }

    error_code ec;
    auto addr_v4 = boost::asio::ip::make_address_v4(bind_ip, ec);
    if (ec)
    {
        std::cerr << "Invalid IPv4 address for bind: " << bind_ip << " (" << ec.message() << ")\n";
        return 1;
    }

    constexpr int numValuesPerMsg = 73;
    constexpr std::size_t BufLen = numValuesPerMsg * 8;

    try
    {
        std::signal(SIGINT, handle_sigint);

        boost::asio::io_context io;
        udp::endpoint local_ep(addr_v4, static_cast<unsigned short>(port));

        udp::socket sock(io);
        sock.open(udp::v4());
        sock.bind(local_ep, ec);
        if (ec)
        {
            std::cerr << "Failed to bind to " << bind_ip << ":" << port << " -> " << ec.message() << "\n";
            return 1;
        }

        // ✅ Make Ctrl+C reliable: don't block forever in receive_from()
        sock.non_blocking(true);

        // Output directory named with bind ip and port (sanitized)
        std::string ip_name = bind_ip;
        for (char &c : ip_name)
            if (c == '.')
                c = '_';
        fs::path outdir = fs::path("out_" + ip_name + "_" + std::to_string(port));
        fs::create_directories(outdir);

        std::ofstream out(outdir / "SensorData.txt", std::ios::app);
        if (!out)
        {
            std::cerr << "Failed to open output file in " << outdir << "\n";
            return 1;
        }

        // Format once (don’t re-set every loop)
        out.setf(std::ios::scientific);
        out << std::setprecision(12);

        std::vector<unsigned char> buf(BufLen);
        udp::endpoint sender;

        std::cout << "Bound to " << bind_ip << ":" << port << " -- receiving UDP...\n";
        std::cout << "Logging to: " << (outdir / "SensorData.txt") << "\n";
        std::cout << "Press Ctrl+C to stop.\n";

        int wrong_size_warned = 0;

        while (!g_stop)
        {
            std::size_t n = sock.receive_from(boost::asio::buffer(buf), sender, 0, ec);

            // No data available in non-blocking mode
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

            if (n != BufLen)
            {
                // Throttle console spam
                if (wrong_size_warned < 10)
                {
                    std::cerr << "Got " << n << " bytes from " << sender.address().to_string()
                              << " (expected " << BufLen << "), dropping.\n";
                    if (wrong_size_warned == 9)
                        std::cerr << "(Suppressing further wrong-size warnings...)\n";
                }
                wrong_size_warned++;
                continue;
            }

            std::array<double, numValuesPerMsg> values{};
            for (int j = 0; j < numValuesPerMsg; ++j)
            {
                const unsigned char *p = buf.data() + j * 8;
                const uint64_t u = load_u64_be(p);
                values[j] = u64_to_double(u);
            }

            out << 0 << '\t';
            for (double d : values)
                out << d << '\t';
            out << '\n';
        }

        std::cout << "Shutting down, flushed output.\n";
        out.flush();
        sock.close();
        io.stop();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }

    return 0;
}