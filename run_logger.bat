// main.cpp
#include <boost/asio.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <array>
#include <cstdint>
#include <cstring>
#include <csignal>

namespace fs = std::filesystem;
using boost::asio::ip::udp;
using boost::system::error_code;

static inline uint64_t load_u64_be(const unsigned char *p)
{
    // interpret p[0] as MSB (network order)
    return (uint64_t(p[0]) << 56) | (uint64_t(p[1]) << 48) | (uint64_t(p[2]) << 40) | (uint64_t(p[3]) << 32) |
           (uint64_t(p[4]) << 24) | (uint64_t(p[5]) << 16) | (uint64_t(p[6]) << 8) | (uint64_t(p[7]) << 0);
}

static inline double u64_to_double(uint64_t u)
{
    double d;
    std::memcpy(&d, &u, sizeof(d));
    return d;
}

volatile std::sig_atomic_t g_stop = 0;
void handle_sigint(int) { g_stop = 1; }

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cerr << "Usage: hrg_logger <local_bind_ip> <local_port>\n";
        std::cerr << "Example: hrg_logger 192.168.10.5 25001\n";
        return 1;
    }

    std::string bind_ip = argv[1];
    std::string port_str = argv[2];

    // Validate port number
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

    // Validate IP (IPv4 expected) using make_address_v4
    error_code ec;
    auto addr_v4 = boost::asio::ip::make_address_v4(bind_ip, ec);
    if (ec)
    {
        std::cerr << "Invalid IPv4 address for bind: " << bind_ip << " (" << ec.message() << ")\n";
        return 1;
    }

    const int numValuesPerMsg = 73;
    const std::size_t BufLen = numValuesPerMsg * 8;

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

        // Make output directory named with bind ip and port (sanitized)
        std::string ip_name = bind_ip;
        for (char &c : ip_name)
            if (c == '.') c = '_';
        std::string outdir_name = "out_" + ip_name + "_" + std::to_string(port);
        fs::path outdir = fs::path(outdir_name);
        fs::create_directories(outdir);

        std::ofstream out(outdir / "SensorData.txt", std::ios::app);
        if (!out)
        {
            std::cerr << "Failed to open output file in " << outdir << "\n";
            return 1;
        }

        std::vector<unsigned char> buf(BufLen);
        udp::endpoint sender;

        std::cout << "Bound to " << bind_ip << ":" << port << " -- receiving UDP on that interface...\n";
        std::cout << "Logging to: " << (outdir / "SensorData.txt") << "\n";
        std::cout << "Press Ctrl+C to stop.\n";

        while (!g_stop)
        {
            // This call blocks until a datagram arrives.
            std::size_t n = sock.receive_from(boost::asio::buffer(buf), sender, 0, ec);
            if (ec)
            {
                if (g_stop) break;
                std::cerr << "Receive error: " << ec.message() << "\n";
                continue;
            }

            if (n != BufLen)
            {
                std::cerr << "Got " << n << " bytes from " << sender.address().to_string()
                          << " (expected " << BufLen << "), dropping.\n";
                continue;
            }

            std::array<double, 73> values{};
            for (int j = 0; j < numValuesPerMsg; ++j)
            {
                const unsigned char *p = buf.data() + j * 8;
                uint64_t u = load_u64_be(p);
                values[j] = u64_to_double(u);
            }

            out << 0 << '\t';
            for (double d : values)
                out << std::scientific << d << '\t';
            out << "\n";
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