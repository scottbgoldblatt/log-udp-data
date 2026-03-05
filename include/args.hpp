// File: src/args.hpp
#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>

namespace imu_logger
{

    struct Args
    {
        std::string mode = "bin"; // bin|text
        std::string bind = "0.0.0.0";
        int port = 25001;

        std::string outdir = "./logs";
        std::string prefix = "SensorData";

        int rotate_min = 0; // 0 = no rotation
        bool strict_size = true;
        bool crc32 = true;  // binary writer uses this
        int precision = 12; // text writer precision after decimal in scientific
    };

    inline void print_usage(const char *prog)
    {
        std::cerr
            << "Usage: " << prog << " [options]\n\n"
            << "Options:\n"
            << "  --mode bin|text           Output mode (default: bin)\n"
            << "  --bind <ip>               Bind IP (default: 0.0.0.0)\n"
            << "  --port <n>                UDP port (default: 25001)\n"
            << "  --outdir <path>           Output directory (default: ./logs)\n"
            << "  --prefix <name>           File prefix (default: SensorData)\n"
            << "  --rotate-min <n>          Rotate files every N minutes (0 = off)\n"
            << "  --strict-size on|off      Drop packets not exactly 584 bytes (default: on)\n"
            << "  --crc32 on|off             Binary mode: include CRC32 per record (default: on)\n"
            << "  --precision <n>           Text mode: digits after decimal in scientific (default: 12)\n"
            << "  -h, --help                Show this help\n\n"
            << "Example:\n"
            << "  " << prog << " --mode bin --bind 0.0.0.0 --port 25001 --outdir /var/log/imu_logger --rotate-min 10\n";
    }

    inline bool parse_on_off(const std::string &s, bool &out)
    {
        if (s == "on" || s == "1" || s == "true")
        {
            out = true;
            return true;
        }
        if (s == "off" || s == "0" || s == "false")
        {
            out = false;
            return true;
        }
        return false;
    }

    inline bool parse_args(int argc, char **argv, Args &a)
    {
        std::vector<std::string> v(argv + 1, argv + argc);

        auto need_value = [&](std::size_t i) -> bool
        {
            if (i + 1 >= v.size())
            {
                std::cerr << "Missing value for " << v[i] << "\n";
                return false;
            }
            return true;
        };

        for (std::size_t i = 0; i < v.size(); ++i)
        {
            const auto &s = v[i];

            if (s == "-h" || s == "--help")
            {
                print_usage(argv[0]);
                return false;
            }
            else if (s == "--mode")
            {
                if (!need_value(i))
                    return false;
                a.mode = v[++i];
                if (a.mode != "bin" && a.mode != "text")
                {
                    std::cerr << "--mode must be bin or text\n";
                    return false;
                }
            }
            else if (s == "--bind")
            {
                if (!need_value(i))
                    return false;
                a.bind = v[++i];
            }
            else if (s == "--port")
            {
                if (!need_value(i))
                    return false;
                a.port = std::stoi(v[++i]);
                if (a.port <= 0 || a.port > 65535)
                {
                    std::cerr << "Port out of range\n";
                    return false;
                }
            }
            else if (s == "--outdir")
            {
                if (!need_value(i))
                    return false;
                a.outdir = v[++i];
            }
            else if (s == "--prefix")
            {
                if (!need_value(i))
                    return false;
                a.prefix = v[++i];
            }
            else if (s == "--rotate-min")
            {
                if (!need_value(i))
                    return false;
                a.rotate_min = std::stoi(v[++i]);
                if (a.rotate_min < 0)
                    a.rotate_min = 0;
            }
            else if (s == "--strict-size")
            {
                if (!need_value(i))
                    return false;
                bool b;
                if (!parse_on_off(v[++i], b))
                {
                    std::cerr << "--strict-size must be on|off\n";
                    return false;
                }
                a.strict_size = b;
            }
            else if (s == "--crc32")
            {
                if (!need_value(i))
                    return false;
                bool b;
                if (!parse_on_off(v[++i], b))
                {
                    std::cerr << "--crc32 must be on|off\n";
                    return false;
                }
                a.crc32 = b;
            }
            else if (s == "--precision")
            {
                if (!need_value(i))
                    return false;
                a.precision = std::stoi(v[++i]);
                if (a.precision < 0)
                    a.precision = 0;
                if (a.precision > 17)
                    a.precision = 17;
            }
            else
            {
                std::cerr << "Unknown option: " << s << "\n";
                print_usage(argv[0]);
                return false;
            }
        }
        return true;
    }

} // namespace imu_logger