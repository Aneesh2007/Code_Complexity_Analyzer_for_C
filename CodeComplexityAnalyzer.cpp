#include "CodeComplexityAnalyzer.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

// ============================================================
//  CodeComplexityAnalyzer.cpp — Full analysis implementation
// ============================================================

// ---- Constructor ----
CodeComplexityAnalyzer::CodeComplexityAnalyzer(const std::vector<std::string>& lines)
    : Analyzer(lines),
      maxNestingDepth(0),
      currentNestingDepth(0),
      arrayDeclarations(0),
      containerDeclarations(0) {

    // Initialize all keyword counters to 0
    loopCounts["for"]     = 0;
    loopCounts["while"]   = 0;
    loopCounts["do"]      = 0;

    conditionalCounts["if"]      = 0;
    conditionalCounts["else if"] = 0;
    conditionalCounts["switch"]  = 0;

    jumpCounts["break"]    = 0;
    jumpCounts["continue"] = 0;
    jumpCounts["goto"]     = 0;
    jumpCounts["return"]   = 0;
}

// ============================================================
//  Helper: checks if keyword exists as a whole word in line
//  e.g., "format" should NOT match "for"
// ============================================================
bool CodeComplexityAnalyzer::containsKeyword(const std::string& line,
                                              const std::string& keyword) const {
    size_t pos = 0;
    while ((pos = line.find(keyword, pos)) != std::string::npos) {
        // Check left boundary: must be start of string or non-alphanumeric
        bool leftOk = (pos == 0) || (!std::isalnum(line[pos - 1]) && line[pos - 1] != '_');

        // Check right boundary: must be end of string or non-alphanumeric
        size_t afterPos = pos + keyword.size();
        bool rightOk = (afterPos >= line.size()) ||
                       (!std::isalnum(line[afterPos]) && line[afterPos] != '_');

        if (leftOk && rightOk) return true;
        pos++;
    }
    return false;
}

// ---- Checks if line has a loop keyword ----
bool CodeComplexityAnalyzer::isLoopLine(const std::string& line) const {
    return containsKeyword(line, "for")   ||
           containsKeyword(line, "while") ||
           containsKeyword(line, "do");
}

// ---- Counts occurrences of a character in a line ----
int CodeComplexityAnalyzer::countChar(const std::string& line, char ch) const {
    return static_cast<int>(std::count(line.begin(), line.end(), ch));
}

// ---- Detects STL container declarations ----
bool CodeComplexityAnalyzer::hasContainerDeclaration(const std::string& line) const {
    // STL containers that imply O(N) space
    static const std::vector<std::string> containers = {
        "vector<", "map<", "unordered_map<", "set<",
        "unordered_set<", "list<", "deque<", "queue<",
        "stack<", "priority_queue<", "multimap<", "multiset<"
    };
    for (const auto& c : containers) {
        if (line.find(c) != std::string::npos) return true;
    }
    return false;
}

// ---- Detects C-style array declarations ----
bool CodeComplexityAnalyzer::hasArrayDeclaration(const std::string& line) const {
    // Pattern: type identifier[size]  — look for "[" followed by digits or "]"
    size_t bracketPos = line.find('[');
    if (bracketPos == std::string::npos) return false;

    // Make sure it's not just array access (needs a type keyword before it)
    static const std::vector<std::string> typeKeywords = {
        "int", "float", "double", "char", "long", "short",
        "unsigned", "string", "bool"
    };
    for (const auto& t : typeKeywords) {
        if (line.find(t) != std::string::npos &&
            line.find(t) < bracketPos) {
            return true;
        }
    }
    return false;
}

// ============================================================
//  analyze() — Main analysis pass, goes line by line
//  Polymorphic override of Analyzer::analyze()
// ============================================================
void CodeComplexityAnalyzer::analyze() {
    // ---- Nesting depth strategy ----
    // We use a std::stack<bool> to track each open brace block.
    // true  = this block was opened by a loop/conditional (counts toward depth)
    // false = this block is a regular scope (function body, namespace, etc.)
    std::stack<bool> blockStack;

    bool nextBlockIsLoop = false; // Set when current line has a loop keyword
    currentNestingDepth = 0;
    maxNestingDepth     = 0;

    for (const auto& line : sourceLines) {

        // ---- Count loops ----
        if (containsKeyword(line, "for"))   loopCounts["for"]++;
        if (containsKeyword(line, "while") &&
            !containsKeyword(line, "do"))   loopCounts["while"]++;
        if (containsKeyword(line, "do") &&
            !containsKeyword(line, "while"))loopCounts["do"]++;

        // ---- Count conditionals ----
        if (containsKeyword(line, "else if")) {
            conditionalCounts["else if"]++;
        } else if (containsKeyword(line, "if")) {
            conditionalCounts["if"]++;
        }
        if (containsKeyword(line, "switch")) conditionalCounts["switch"]++;

        // ---- Count jump statements ----
        if (containsKeyword(line, "break"))    jumpCounts["break"]++;
        if (containsKeyword(line, "continue")) jumpCounts["continue"]++;
        if (containsKeyword(line, "goto"))     jumpCounts["goto"]++;
        if (containsKeyword(line, "return"))   jumpCounts["return"]++;

        // ---- Space complexity indicators ----
        if (hasContainerDeclaration(line)) containerDeclarations++;
        if (hasArrayDeclaration(line))     arrayDeclarations++;

        // ---- Does THIS line have a loop/conditional keyword? ----
        bool thisLineIsControl = isLoopLine(line);

        // A '{' on this line opens a control block if EITHER:
        // (a) previous line set the flag, OR (b) this very line has a keyword
        bool opensControlBlock = nextBlockIsLoop || thisLineIsControl;

        // ---- Process braces character by character ----
        for (char ch : line) {
            if (ch == '{') {
                blockStack.push(opensControlBlock);
                if (opensControlBlock) {
                    currentNestingDepth++;
                    if (currentNestingDepth > maxNestingDepth) {
                        maxNestingDepth = currentNestingDepth;
                    }
                }
            } else if (ch == '}') {
                if (!blockStack.empty()) {
                    if (blockStack.top()) {
                        currentNestingDepth--;
                    }
                    blockStack.pop();
                }
            }
        }

        // ---- Set flag for NEXT line ----
        // Only carry forward if this line had a control keyword but no brace yet
        nextBlockIsLoop = thisLineIsControl && (line.find('{') == std::string::npos);
    }
}

// ============================================================
//  Big-O conversion helpers
// ============================================================
std::string CodeComplexityAnalyzer::depthToBigO(int depth) const {
    if (depth == 0) return "O(1)   — No loops detected";
    if (depth == 1) return "O(N)   — Single loop level";
    if (depth == 2) return "O(N^2) — Two nested loop levels";
    if (depth == 3) return "O(N^3) — Three nested loop levels";
    return "O(N^" + std::to_string(depth) + ") — Deep nesting detected";
}

std::string CodeComplexityAnalyzer::spaceComplexityEstimate() const {
    int totalStructures = arrayDeclarations + containerDeclarations;
    if (totalStructures == 0) return "O(1)   — No significant data structures found";
    if (totalStructures <= 2)  return "O(N)   — Linear space (arrays/containers found)";
    return "O(N^2) — Multiple large data structures detected";
}

// ============================================================
//  Report builders — return Report objects (uses move semantics
//  when returned from function via RVO/move)
// ============================================================
Report CodeComplexityAnalyzer::buildNestingReport() const {
    Report r("NESTING DEPTH ANALYSIS");

    int totalLoops = loopCounts.at("for") + loopCounts.at("while") + loopCounts.at("do");

    r.addEntry("  Total Lines Scanned  : " + std::to_string(getLineCount()));
    r.addEntry("  Total Loops Found    : " + std::to_string(totalLoops));
    r.addEntry("  -- for loops         : " + std::to_string(loopCounts.at("for")));
    r.addEntry("  -- while loops       : " + std::to_string(loopCounts.at("while")));
    r.addEntry("  -- do-while loops    : " + std::to_string(loopCounts.at("do")));
    r.addEntry("");
    r.addEntry("  Max Nesting Depth    : " + std::to_string(maxNestingDepth));
    r.addEntry("  Estimated Time Complexity : " + depthToBigO(maxNestingDepth));

    return r; // Move semantics applied here (NRVO / move return)
}

Report CodeComplexityAnalyzer::buildJumpReport() const {
    Report r("JUMP STATEMENT ANALYSIS");

    int total = jumpCounts.at("break")    + jumpCounts.at("continue") +
                jumpCounts.at("goto")     + jumpCounts.at("return");

    r.addEntry("  Total Jump Statements : " + std::to_string(total));
    r.addEntry("  -- break              : " + std::to_string(jumpCounts.at("break")));
    r.addEntry("  -- continue           : " + std::to_string(jumpCounts.at("continue")));
    r.addEntry("  -- goto               : " + std::to_string(jumpCounts.at("goto")));
    r.addEntry("  -- return             : " + std::to_string(jumpCounts.at("return")));
    r.addEntry("");
    r.addEntry("  Conditionals Found:");
    r.addEntry("  -- if                 : " + std::to_string(conditionalCounts.at("if")));
    r.addEntry("  -- else if            : " + std::to_string(conditionalCounts.at("else if")));
    r.addEntry("  -- switch             : " + std::to_string(conditionalCounts.at("switch")));

    return r;
}

Report CodeComplexityAnalyzer::buildFullReport() const {
    Report r("FULL COMPLEXITY REPORT (TIME & SPACE)");

    int totalLoops = loopCounts.at("for") + loopCounts.at("while") + loopCounts.at("do");
    int totalJumps = jumpCounts.at("break") + jumpCounts.at("continue") +
                     jumpCounts.at("goto")  + jumpCounts.at("return");
    int totalConds = conditionalCounts.at("if") + conditionalCounts.at("else if") +
                     conditionalCounts.at("switch");

    r.addEntry("  --- Structural Summary ---");
    r.addEntry("  Total Lines          : " + std::to_string(getLineCount()));
    r.addEntry("  Total Loops          : " + std::to_string(totalLoops));
    r.addEntry("  Total Conditionals   : " + std::to_string(totalConds));
    r.addEntry("  Total Jump Stmts     : " + std::to_string(totalJumps));
    r.addEntry("  Max Nesting Depth    : " + std::to_string(maxNestingDepth));
    r.addEntry("");
    r.addEntry("  --- Space Complexity Indicators ---");
    r.addEntry("  Array Declarations   : " + std::to_string(arrayDeclarations));
    r.addEntry("  STL Containers       : " + std::to_string(containerDeclarations));
    r.addEntry("");
    r.addEntry("  ====================================");
    r.addEntry("  TIME COMPLEXITY  : " + depthToBigO(maxNestingDepth));
    r.addEntry("  SPACE COMPLEXITY : " + spaceComplexityEstimate());
    r.addEntry("  ====================================");

    return r;
}

// ============================================================
//  Public report printers — build report then move it for print
// ============================================================
void CodeComplexityAnalyzer::printNestingReport() const {
    Report r = buildNestingReport();      // built on stack
    r.print();                            // print in place
}

void CodeComplexityAnalyzer::printJumpReport() const {
    Report r = buildJumpReport();
    r.print();
}

void CodeComplexityAnalyzer::printFullReport() const {
    Report r = buildFullReport();
    r.print();
}

// Default printReport — prints the full report (polymorphic override)
void CodeComplexityAnalyzer::printReport() const {
    printFullReport();
}
