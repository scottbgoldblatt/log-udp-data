// udp_test_sender_boost.cpp
// Build: link with Boost::system (and Boost::asio headers must be discoverable)
#include <boost/asio.hpp>
#include <iostream>
#include <vector>
#include <cstring>
#include <chrono>
#include <thread>
#include <cstdint>

using boost::asio::ip::udp;

static void write_u64_be(uint8_t *dst, uint64_t v)
{
    dst[0] = uint8_t((v >> 56) & 0xFF);
    dst[1] = uint8_t((v >> 48) & 0xFF);
    dst[2] = uint8_t((v >> 40) & 0xFF);
    dst[3] = uint8_t((v >> 32) & 0xFF);
    dst[4] = uint8_t((v >> 24) & 0xFF);
    dst[5] = uint8_t((v >> 16) & 0xFF);
    dst[6] = uint8_t((v >> 8) & 0xFF);
    dst[7] = uint8_t((v >> 0) & 0xFF);
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: udp_test_sender_boost DEST_IP DEST_PORT [NUM_PACKETS=1] [INTERVAL_MS=200]\n";
        return 1;
    }

    const std::string dest_ip = argv[1];
    const unsigned short dest_port = static_cast<unsigned short>(std::stoi(argv[2]));
    const int num_packets = (argc >= 4) ? std::stoi(argv[3]) : 1;
    const int interval_ms = (argc >= 5) ? std::stoi(argv[4]) : 200;

    const int NUM_VALUES = 73;
    const std::size_t BUF_LEN = NUM_VALUES * 8;

    try
    {
        boost::asio::io_context io;
        udp::socket sock(io);
        sock.open(udp::v4());

        udp::endpoint remote_ep(boost::asio::ip::make_address(dest_ip), dest_port);

        std::vector<uint8_t> buffer(BUF_LEN);

        std::cout << "Sending " << num_packets << " packet(s) to " << dest_ip << ":" << dest_port
                  << " (" << BUF_LEN << " bytes each)\n";

        for (int pkt = 0; pkt < num_packets; ++pkt)
        {
            for (int i = 0; i < NUM_VALUES; ++i)
            {
                double value = static_cast<double>(i + 1) + static_cast<double>(pkt) * 1000.0;
                uint64_t u;
                std::memcpy(&u, &value, sizeof(u)); // host-order IEEE-754 bits
                // write big-endian bytes
                write_u64_be(buffer.data() + i * 8, u);
            }

            std::size_t bytes_sent = sock.send_to(boost::asio::buffer(buffer), remote_ep);
            std::cout << "Sent packet " << (pkt + 1) << " (" << bytes_sent << " bytes)\n";

            if (pkt + 1 < num_packets)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
            }
        }

        sock.close();
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Exception: " << ex.what() << "\n";
        return 1;
    }

    std::cout << "Done.\n";
    return 0;
}