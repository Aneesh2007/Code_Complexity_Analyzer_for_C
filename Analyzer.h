#ifndef ANALYZER_H
#define ANALYZER_H

#include <string>
#include <vector>

// ============================================================
//  Analyzer.h — Abstract Base Class for all analyzers
//  Demonstrates: Inheritance, Polymorphism (pure virtual)
// ============================================================

class Analyzer {
protected:
    std::vector<std::string> sourceLines; // Stores all lines of the loaded file

public:
    // Constructor — loads lines into the object
    explicit Analyzer(const std::vector<std::string>& lines) : sourceLines(lines) {}

    // Virtual destructor — essential for polymorphic base classes
    virtual ~Analyzer() = default;

    // Pure virtual functions — every derived class MUST implement these
    virtual void analyze() = 0;
    virtual void printReport() const = 0;

    // Helper: returns total number of lines loaded
    int getLineCount() const { return static_cast<int>(sourceLines.size()); }
};

#endif // ANALYZER_H
