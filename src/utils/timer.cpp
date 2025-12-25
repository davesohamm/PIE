#include <chrono>

namespace pie {
namespace utils {

class Timer {
public:
    Timer() : start_(std::chrono::high_resolution_clock::now()) {}
    
    void Reset() {
        start_ = std::chrono::high_resolution_clock::now();
    }
    
    double ElapsedMilliseconds() const {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_);
        return duration.count() / 1000.0;
    }
    
    double ElapsedSeconds() const {
        return ElapsedMilliseconds() / 1000.0;
    }
    
private:
    std::chrono::high_resolution_clock::time_point start_;
};

} // namespace utils
} // namespace pie

