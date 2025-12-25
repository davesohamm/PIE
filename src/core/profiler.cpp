#include "pie/core/profiler.h"
#include <cstdio>
#include <cstdlib>
<cstring>
#include <array>
#include <stdexcept>
#include <regex>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

namespace pie {

Profiler::Profiler() = default;
Profiler::~Profiler() = default;

bool Profiler::IsAvailable() {
    // Check if perf is available
    FILE* pipe = popen("which perf 2>/dev/null", "r");
    if (!pipe) return false;
    
    std::array<char, 128> buffer;
    std::string result;
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    pclose(pipe);
    
    return !result.empty();
}

PerformanceMetrics Profiler::Profile(std::function<void()> workload) {
    PerformanceMetrics metrics;
    
    // Measure wall time
    auto start = std::chrono::high_resolution_clock::now();
    
    // Execute workload
    workload();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    metrics.wall_time_ms = duration.count() / 1000.0;
    
    // Note: In-process profiling is limited. For detailed hardware counters,
    // we'd need to use libperf or PAPI. For now, we profile external binaries.
    
    return metrics;
}

PerformanceMetrics Profiler::ProfileBinary(const std::string& binary_path,
                                           const std::vector<std::string>& args) {
    // Build perf command
    std::string perf_cmd = "perf stat -e ";
    
    if (detailed_mode_) {
        perf_cmd += "cycles,instructions,"
                   "L1-dcache-loads,L1-dcache-load-misses,"
                   "LLC-loads,LLC-load-misses,"
                   "branches,branch-misses,"
                   "cache-references,cache-misses";
    } else {
        perf_cmd += "cycles,instructions,cache-misses,branches,branch-misses";
    }
    
    perf_cmd += " -- " + binary_path;
    for (const auto& arg : args) {
        perf_cmd += " " + arg;
    }
    
    // Redirect stderr to stdout since perf outputs to stderr
    perf_cmd += " 2>&1";
    
    // Execute and capture output
    std::string output = ExecuteCommand(perf_cmd);
    
    // Parse metrics
    return ParsePerfOutput(output);
}

PerformanceMetrics Profiler::ParsePerfOutput(const std::string& output) const {
    PerformanceMetrics metrics;
    
    // Regex patterns for perf stat output
    std::regex cycles_regex(R"(([\d,]+)\s+cycles)");
    std::regex instructions_regex(R"(([\d,]+)\s+instructions)");
    std::regex l1_loads_regex(R"(([\d,]+)\s+L1-dcache-loads)");
    std::regex l1_misses_regex(R"(([\d,]+)\s+L1-dcache-load-misses)");
    std::regex llc_loads_regex(R"(([\d,]+)\s+LLC-loads)");
    std::regex llc_misses_regex(R"(([\d,]+)\s+LLC-load-misses)");
    std::regex cache_refs_regex(R"(([\d,]+)\s+cache-references)");
    std::regex cache_misses_regex(R"(([\d,]+)\s+cache-misses)");
    std::regex branches_regex(R"(([\d,]+)\s+branches)");
    std::regex branch_misses_regex(R"(([\d,]+)\s+branch-misses)");
    std::regex time_regex(R"(([\d.]+)\s+seconds time elapsed)");
    
    std::smatch match;
    
    // Helper to extract number
    auto extract_number = [](const std::string& str) -> uint64_t {
        std::string clean = str;
        clean.erase(std::remove(clean.begin(), clean.end(), ','), clean.end());
        return std::stoull(clean);
    };
    
    // Parse each metric
    if (std::regex_search(output, match, cycles_regex)) {
        metrics.cycles = extract_number(match[1]);
    }
    
    if (std::regex_search(output, match, instructions_regex)) {
        metrics.instructions = extract_number(match[1]);
    }
    
    if (std::regex_search(output, match, l1_loads_regex)) {
        metrics.l1_dcache_loads = extract_number(match[1]);
    }
    
    if (std::regex_search(output, match, l1_misses_regex)) {
        metrics.l1_dcache_load_misses = extract_number(match[1]);
    }
    
    if (std::regex_search(output, match, llc_loads_regex)) {
        metrics.llc_references = extract_number(match[1]);
    }
    
    if (std::regex_search(output, match, llc_misses_regex)) {
        metrics.llc_misses = extract_number(match[1]);
    }
    
    if (std::regex_search(output, match, cache_refs_regex)) {
        metrics.l2_cache_references = extract_number(match[1]);
    }
    
    if (std::regex_search(output, match, cache_misses_regex)) {
        metrics.l2_cache_misses = extract_number(match[1]);
    }
    
    if (std::regex_search(output, match, branches_regex)) {
        metrics.branches = extract_number(match[1]);
    }
    
    if (std::regex_search(output, match, branch_misses_regex)) {
        metrics.branch_misses = extract_number(match[1]);
    }
    
    if (std::regex_search(output, match, time_regex)) {
        metrics.wall_time_ms = std::stod(match[1]) * 1000.0;
    }
    
    return metrics;
}

std::string Profiler::ExecuteCommand(const std::string& command) const {
    std::array<char, 128> buffer;
    std::string result;
    
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("Failed to execute command: " + command);
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    
    int status = pclose(pipe);
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        std::cerr << "Command failed with exit code: " << WEXITSTATUS(status) << std::endl;
        std::cerr << "Output: " << result << std::endl;
    }
    
    return result;
}

bool Profiler::ExportMetrics(const PerformanceMetrics& metrics,
                            const std::string& output_path) const {
    std::ofstream file(output_path);
    if (!file.is_open()) {
        return false;
    }
    
    file << "{\n";
    file << "  \"cycles\": " << metrics.cycles << ",\n";
    file << "  \"instructions\": " << metrics.instructions << ",\n";
    file << "  \"ipc\": " << metrics.ipc() << ",\n";
    file << "  \"l1_dcache_loads\": " << metrics.l1_dcache_loads << ",\n";
    file << "  \"l1_dcache_load_misses\": " << metrics.l1_dcache_load_misses << ",\n";
    file << "  \"l1_miss_rate\": " << metrics.l1_miss_rate() << ",\n";
    file << "  \"l2_cache_references\": " << metrics.l2_cache_references << ",\n";
    file << "  \"l2_cache_misses\": " << metrics.l2_cache_misses << ",\n";
    file << "  \"l2_miss_rate\": " << metrics.l2_miss_rate() << ",\n";
    file << "  \"llc_references\": " << metrics.llc_references << ",\n";
    file << "  \"llc_misses\": " << metrics.llc_misses << ",\n";
    file << "  \"llc_miss_rate\": " << metrics.llc_miss_rate() << ",\n";
    file << "  \"branches\": " << metrics.branches << ",\n";
    file << "  \"branch_misses\": " << metrics.branch_misses << ",\n";
    file << "  \"branch_miss_rate\": " << metrics.branch_miss_rate() << ",\n";
    file << "  \"wall_time_ms\": " << metrics.wall_time_ms << "\n";
    file << "}\n";
    
    file.close();
    return true;
}

} // namespace pie

