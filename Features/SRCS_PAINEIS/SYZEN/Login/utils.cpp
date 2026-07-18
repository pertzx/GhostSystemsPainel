#include "utils.hpp"
#include "skStr.h"
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <sstream>

namespace utils {
    std::string get_hwid() {
        std::ifstream cpuinfo(skCrypt("/proc/cpuinfo"));
        std::string line, hwid;
        while (std::getline(cpuinfo, line)) {
            if (line.find(skCrypt("Serial")) != std::string::npos) {
                hwid = line.substr(line.find(":") + 2);
                break;
            }
        }
        return hwid;
    }
    
    std::string get_license_from_terminal() {
        std::cout << skCrypt("Digite sua license key: ");
        std::string license;
        std::getline(std::cin, license);
        return license;
    }
}
