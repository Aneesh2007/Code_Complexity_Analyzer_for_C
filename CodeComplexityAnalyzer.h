#ifndef CODECOMPLEXITYANALYZER_H
#define CODECOMPLEXITYANALYZER_H

#include "Analyzer.h"
#include "Report.h"
#include <map>
#include <stack>
#include <string>
#include <vector>

// ============================================================
//  CodeComplexityAnalyzer.h — Derived class, does all analysis
//  Demonstrates: Inheritance (extends Analyzer), Polymorphism,
//                STL (map, vector, stack), String Manipulation
// ============================================================

class CodeComplexityAnalyzer : public Analyzer {
private:
    // ---- Structural counts (stored in a map for clean reporting) ----
    std::map<std::string, int> loopCounts;       // "for", "while", "do-while"
    std::map<std::string, int> conditionalCounts; // "if", "else if", "switch"
    std::map<std::string, int> jumpCounts;        // "break", "continue", "goto", "return"

    // ---- Nesting analysis ----
    int maxNestingDepth;       // Maximum loop nesting depth found
    int currentNestingDepth;   // Running depth tracker

    // ---- Space complexity indicators ----
    int arrayDeclarations;     // Count of C-style array declarations
    int containerDeclarations; // Count of STL container declarations (vector, map, etc.)

    // ---- Internal analysis helpers ----

    // Checks if a keyword exists as a whole word in the line
    bool containsKeyword(const std::string& line, const std::string& keyword) const;

    // Checks if line contains a loop keyword (for/while/do)
    bool isLoopLine(const std::string& line) const;

    // Counts opening and closing braces in a line
    int countChar(const std::string& line, char ch) const;

    // Detects STL container declarations (vector<, map<, etc.)
    bool hasContainerDeclaration(const std::string& line) const;

    // Detects C-style array declarations (e.g., int arr[])
    bool hasArrayDeclaration(const std::string& line) const;

    // ---- Report builders ----
    Report buildNestingReport() const;
    Report buildJumpReport() const;
    Report buildFullReport() const;

    // Converts nesting depth to Big-O notation string
    std::string depthToBigO(int depth) const;

    // Converts space indicators to Big-O string
    std::string spaceComplexityEstimate() const;

public:
    // Constructor — passes lines up to base class
    explicit CodeComplexityAnalyzer(const std::vector<std::string>& lines);

    // ---- Overrides from Analyzer (Polymorphism) ----
    void analyze() override;       // Runs full structural analysis
    void printReport() const override; // Prints default (full) report

    // ---- Specific report printers (called from the submenu) ----
    void printNestingReport() const;
    void printJumpReport() const;
    void printFullReport() const;
};

#endif // CODECOMPLEXITYANALYZER_H
