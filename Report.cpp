#include "Report.h"
#include <iostream>
#include <iomanip>

// ============================================================
//  Report.cpp — Implementation of Report methods
// ============================================================

void Report::print() const {
    std::cout << "\n";
    std::cout << "============================================================\n";
    std::cout << "  " << title << "\n";
    std::cout << "============================================================\n";
    for (const auto& line : entries) {
        std::cout << line << "\n";
    }
    std::cout << "============================================================\n";
}
