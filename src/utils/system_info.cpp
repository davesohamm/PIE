#include <string>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <array>

namespace pie {
namespace utils {

std::string GetCPUModel() {
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    
    while (std::getline(cpuinfo, line)) {
        if (line.find("model name") != std::string::npos) {
            size_t pos = line.find(":");
            if (pos != std::string::npos) {
                return line.substr(pos + 2);
            }
        }
    }
    
    return "Unknown CPU";
}

int GetCPUCores() {
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    int cores = 0;
    
    while (std::getline(cpuinfo, line)) {
        if (line.find("processor") != std::string::npos) {
            cores++;
        }
    }
    
    return cores > 0 ? cores : 1;
}

size_t GetCacheSize(int level) {
    std::string path = "/sys/devices/system/cpu/cpu0/cache/index" + 
                      std::to_string(level) + "/size";
    
    std::ifstream file(path);
    if (!file.is_open()) return 0;
    
    std::string size_str;
    std::getline(file, size_str);
    
    // Parse size (e.g., "32K", "256K", "8192K")
    size_t size = std::stoul(size_str);
    if (size_str.back() == 'K') {
        size *= 1024;
    } else if (size_str.back() == 'M') {
        size *= 1024 * 1024;
    }
    
    return size;
}

std::string ExecCommand(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";
    
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    
    pclose(pipe);
    return result;
}

} // namespace utils
} // namespace pie

