#include <boost/asio.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>

namespace fs = std::filesystem;
using boost::asio::ip::udp;

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

int main(int argc, char **argv)
{
    //    try
    {
        const unsigned short port = 25001;
        const int numValuesPerMsg = 73;
        const std::size_t BufLen = numValuesPerMsg * 8;

        boost::asio::io_context io;
        udp::socket sock(io, udp::endpoint(udp::v4(), port));

        // Output directory (replace with your timestamp/IP naming as needed)
        fs::path outdir = fs::path("out");
        fs::create_directories(outdir);

        std::ofstream out(outdir / "SensorData.txt", std::ios::app);
        if (!out)
        {
            std::cerr << "Failed to open output file\n";
            return 1;
        }

        std::vector<unsigned char> buf(BufLen);
        udp::endpoint sender;

        std::cout << "Receiving UDP on port " << port << "...\n";

        while (true)
        {
            std::size_t n = sock.receive_from(boost::asio::buffer(buf), sender);

            if (n != BufLen)
            {
                // You can decide whether to drop/handle partial or different packets
                std::cerr << "Got " << n << " bytes from " << sender.address().to_string()
                          << " (expected " << BufLen << "), dropping.\n";
                continue;
            }

            // Decode: assumes sender sends doubles in network byte order (big-endian).
            // If your sender is little-endian raw doubles, change load_u64_be accordingly.
            std::array<double, 73> values{};
            for (int j = 0; j < numValuesPerMsg; ++j)
            {
                const unsigned char *p = buf.data() + j * 8;
                uint64_t u = load_u64_be(p);
                values[j] = u64_to_double(u);
            }

            // Write one line
            out << 0 << '\t';
            for (double d : values)
                out << std::scientific << d << '\t';
            out << "\n";
        }
    }
    //   catch (const std::exception &e)
    {
        //  std::cerr << "Fatal: " << e.what() << "\n";
        // return 1;
    }
}