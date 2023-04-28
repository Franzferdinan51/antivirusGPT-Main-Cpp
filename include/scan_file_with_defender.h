#pragma once

#include <string>

struct ScanResult {
    bool is_infected;
    std::string virus_name;
};

ScanResult scan_file_with_defender(const std::string& file_path);