#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>

namespace pie {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    static Logger& Instance() {
        static Logger instance;
        return instance;
    }
    
    void Log(LogLevel level, const std::string& message) {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        
        std::cout << std::put_time(&tm, "[%H:%M:%S] ");
        
        switch (level) {
            case LogLevel::DEBUG:   std::cout << "[DEBUG] "; break;
            case LogLevel::INFO:    std::cout << "[INFO] "; break;
            case LogLevel::WARNING: std::cout << "[WARN] "; break;
            case LogLevel::ERROR:   std::cout << "[ERROR] "; break;
        }
        
        std::cout << message << std::endl;
    }
    
private:
    Logger() = default;
};

} // namespace pie

