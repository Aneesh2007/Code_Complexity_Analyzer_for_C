#ifndef REPORT_H
#define REPORT_H

#include <string>
#include <vector>
#include <utility> // std::move

// ============================================================
//  Report.h — Stores analysis results
//  Demonstrates: Move Semantics (Move Constructor + Move Assignment)
// ============================================================

class Report {
private:
    std::string title;
    std::vector<std::string> entries; // Each line of the report

public:
    // Default constructor
    Report() = default;

    // Parameterized constructor
    explicit Report(const std::string& reportTitle) : title(reportTitle) {}

    // ---- Move Constructor (avoids deep copy of large result sets) ----
    Report(Report&& other) noexcept
        : title(std::move(other.title)),
          entries(std::move(other.entries)) {}

    // ---- Move Assignment Operator ----
    Report& operator=(Report&& other) noexcept {
        if (this != &other) {
            title   = std::move(other.title);
            entries = std::move(other.entries);
        }
        return *this;
    }

    // Copy constructor and copy assignment (explicitly defined for completeness)
    Report(const Report&)            = default;
    Report& operator=(const Report&) = default;

    ~Report() = default;

    // Add a single entry line to the report
    void addEntry(const std::string& line) {
        entries.push_back(line);
    }

    // Print all entries to console
    void print() const;

    // Getters
    const std::string& getTitle() const { return title; }
    bool isEmpty() const { return entries.empty(); }
};

#endif // REPORT_H
