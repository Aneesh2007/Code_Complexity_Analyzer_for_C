#include "CodeComplexityAnalyzer.h"
#include <iostream>
#include <algorithm>
#include <cctype>

// ============================================================

// ============================================================

// ---- Constructor ----
CodeComplexityAnalyzer::CodeComplexityAnalyzer(const std::vector<std::string>& lines)
    : Analyzer(lines),
      maxNestingDepth(0),
      maxRegularLoopDepth(0),
      logLoopCount(0),
      logLoopMaxRegularDepthFound(0),
      arrayDeclarations(0),
      containerDeclarations(0) {

    // Initialize all keyword counters to 0
    loopCounts["for"]   = 0;
    loopCounts["while"] = 0;
    loopCounts["do"]    = 0;

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

// ---- Checks if line has a loop keyword (not the closing 'while' of a do-while) ----
bool CodeComplexityAnalyzer::isLoopLine(const std::string& line) const {
    if (containsKeyword(line, "for")) return true;
    if (containsKeyword(line, "do"))  return true;
    if (containsKeyword(line, "while")) {
        // A 'while' that follows a '}' on the same line is the tail of a do-while,
        // not a new loop — don't count it as a new loop or a new block opener.
        size_t wPos = line.find("while");
        size_t bPos = line.rfind('}');
        bool isDoWhileTail = (bPos != std::string::npos && bPos < wPos);
        return !isDoWhileTail;
    }
    return false;
}

// ---- Counts occurrences of a character in a line ----
int CodeComplexityAnalyzer::countChar(const std::string& line, char ch) const {
    return static_cast<int>(std::count(line.begin(), line.end(), ch));
}

// ---- Detects STL container declarations ----
bool CodeComplexityAnalyzer::hasContainerDeclaration(const std::string& line) const {
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
//  hasHalvingPattern — detects O(log N) indicators
//  Looks for: divide-by-2 (/=2, >>=1), multiply-by-2 (*=2, <<=1),
//  and binary-search midpoint calculations (mid = ... / 2).
// ============================================================
bool CodeComplexityAnalyzer::hasHalvingPattern(const std::string& line) const {
    // Compound divide-by-2 or right-shift-by-1 assignments
    if (line.find("/= 2")  != std::string::npos) return true;
    if (line.find("/=2")   != std::string::npos) return true;
    if (line.find(">>= 1") != std::string::npos) return true;
    if (line.find(">>=1")  != std::string::npos) return true;

    // Compound multiply-by-2 or left-shift-by-1 assignments (doubling = log iterations)
    if (line.find("*= 2")  != std::string::npos) return true;
    if (line.find("*=2")   != std::string::npos) return true;
    if (line.find("<<= 1") != std::string::npos) return true;
    if (line.find("<<=1")  != std::string::npos) return true;

    // Binary-search midpoint: a variable named 'mid' assigned via integer halving
    if (line.find("mid") != std::string::npos) {
        if (line.find("/ 2")  != std::string::npos || line.find("/2")  != std::string::npos ||
            line.find(">> 1") != std::string::npos || line.find(">>1") != std::string::npos) {
            return true;
        }
    }

    return false;
}

// ============================================================
//  analyze() — Main analysis pass, goes line by line
//  Polymorphic override of Analyzer::analyze()
//
//  Block types tracked per stack entry:
//    0 = non-control scope (function body, if-block, namespace, etc.)
//    1 = regular loop block  (contributes to polynomial complexity)
//    2 = log-time loop block (contributes to O(log N) factor)
// ============================================================
void CodeComplexityAnalyzer::analyze() {
    // Use a vector as a stack so we can retroactively update entries
    // when a halving pattern is discovered inside a loop body.
    std::vector<int> blockStack;

    bool nextBlockIsControl = false; // previous line was a loop without '{'
    bool nextBlockIsLogLoop = false; // previous loop line also had a halving pattern

    int currentAllLoopDepth     = 0; // depth counting ALL loops (for nesting report)
    int currentRegularLoopDepth = 0; // depth counting only regular (non-log) loops

    maxNestingDepth             = 0;
    maxRegularLoopDepth         = 0;
    logLoopCount                = 0;
    logLoopMaxRegularDepthFound = 0;

    for (const auto& line : sourceLines) {

        // ---- Count loops ----
        if (containsKeyword(line, "for")) loopCounts["for"]++;

        // Do-while tail: a 'while' preceded by '}' on the same line closes
        // a do-while and must NOT be counted as a new while loop.
        bool isDoWhileTail = false;
        if (containsKeyword(line, "while")) {
            size_t wPos = line.find("while");
            size_t bPos = line.rfind('}');
            isDoWhileTail = (bPos != std::string::npos && bPos < wPos);
        }
        if (containsKeyword(line, "while") && !isDoWhileTail) loopCounts["while"]++;
        if (containsKeyword(line, "do"))                      loopCounts["do"]++;

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

        // ---- Detect loop type on this line ----
        bool thisLineIsLoop    = isLoopLine(line);
        bool thisLineIsLogLoop = thisLineIsLoop && hasHalvingPattern(line);

        // ---- Retroactive log-loop promotion ----
        // If we find a halving pattern on a non-loop line (e.g., inside a while body),
        // promote the innermost open regular-loop block to a log-loop block.
        if (!thisLineIsLoop && hasHalvingPattern(line)) {
            for (int i = static_cast<int>(blockStack.size()) - 1; i >= 0; i--) {
                if (blockStack[i] == 1) {           // found innermost regular loop
                    blockStack[i] = 2;              // promote to log loop
                    currentRegularLoopDepth--;
                    logLoopCount++;
                    // Record the regular-loop depth OUTSIDE this now-log loop
                    logLoopMaxRegularDepthFound =
                        std::max(logLoopMaxRegularDepthFound, currentRegularLoopDepth);
                    break;
                }
            }
        }

        // ---- Determine what type of block the next '{' will open ----
        bool opensControl = nextBlockIsControl || thisLineIsLoop;
        bool opensAsLog   = nextBlockIsLogLoop  || (thisLineIsLoop && thisLineIsLogLoop);

        // ---- Process braces character by character ----
        for (char ch : line) {
            if (ch == '{') {
                int blockType;
                if (!opensControl) {
                    blockType = 0; // non-control scope
                } else if (opensAsLog) {
                    blockType = 2; // log-time loop
                    logLoopCount++;
                    logLoopMaxRegularDepthFound =
                        std::max(logLoopMaxRegularDepthFound, currentRegularLoopDepth);
                    currentAllLoopDepth++;
                    if (currentAllLoopDepth > maxNestingDepth)
                        maxNestingDepth = currentAllLoopDepth;
                } else {
                    blockType = 1; // regular loop
                    currentRegularLoopDepth++;
                    if (currentRegularLoopDepth > maxRegularLoopDepth)
                        maxRegularLoopDepth = currentRegularLoopDepth;
                    currentAllLoopDepth++;
                    if (currentAllLoopDepth > maxNestingDepth)
                        maxNestingDepth = currentAllLoopDepth;
                }
                blockStack.push_back(blockType);

            } else if (ch == '}') {
                if (!blockStack.empty()) {
                    int blockType = blockStack.back();
                    blockStack.pop_back();
                    if (blockType == 1) {
                        currentRegularLoopDepth--;
                        currentAllLoopDepth--;
                    } else if (blockType == 2) {
                        currentAllLoopDepth--;
                    }
                }
            }
        }

        // ---- Set flags for the NEXT line ----
        // Only carry forward when the loop keyword had no '{' on this line
        bool noOpenBrace   = (line.find('{') == std::string::npos);
        nextBlockIsControl = thisLineIsLoop && noOpenBrace;
        nextBlockIsLogLoop = (thisLineIsLoop && thisLineIsLogLoop) && noOpenBrace;
    }
}

// ============================================================
//  estimateTimeComplexity — log-aware Big-O estimate
//  Uses maxRegularLoopDepth, logLoopCount, and
//  logLoopMaxRegularDepthFound collected by analyze().
// ============================================================
std::string CodeComplexityAnalyzer::estimateTimeComplexity() const {
    // Case 1: No loops at all
    if (maxNestingDepth == 0 && logLoopCount == 0)
        return "O(1)        — No loops detected";

    // Case 2: Only logarithmic loops, no regular polynomial loops
    if (maxRegularLoopDepth == 0 && logLoopCount > 0)
        return "O(log N)    — Logarithmic loop detected (halving / binary search)";

    // Case 3: Regular loops only, no log loops
    if (logLoopCount == 0) {
        if (maxRegularLoopDepth == 1) return "O(N)        — Single loop level";
        if (maxRegularLoopDepth == 2) return "O(N^2)      — Two nested loop levels";
        if (maxRegularLoopDepth == 3) return "O(N^3)      — Three nested loop levels";
        return "O(N^" + std::to_string(maxRegularLoopDepth) +
               ")     — Deep nesting detected";
    }

    // Case 4: Both regular and logarithmic loops found.
    // logLoopMaxRegularDepthFound >= maxRegularLoopDepth means the log loop was
    // found when the regular-loop nesting was already at its deepest, so the log
    // factor multiplies the dominant polynomial term: O(N^k * log N).
    // If it is strictly less, the log loop lives in a shallower scope and the
    // polynomial term dominates.
    if (logLoopMaxRegularDepthFound >= maxRegularLoopDepth) {
        if (maxRegularLoopDepth == 1)
            return "O(N log N)  — Loop with logarithmic inner operation";
        if (maxRegularLoopDepth == 2)
            return "O(N^2 log N) — Nested loops with logarithmic inner operation";
        return "O(N^" + std::to_string(maxRegularLoopDepth) +
               " log N) — Deep nesting with logarithmic inner operation";
    }

    // Log loop exists but is shallower than the deepest regular nesting:
    // the polynomial term dominates (O(N^k) + O(log N) = O(N^k)).
    if (maxRegularLoopDepth == 1)
        return "O(N)        — Single loop level  (separate O(log N) section found)";
    if (maxRegularLoopDepth == 2)
        return "O(N^2)      — Nested loops  (separate O(log N) section found)";
    return "O(N^" + std::to_string(maxRegularLoopDepth) +
           ")     — Deep nesting  (separate O(log N) section found)";
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

    r.addEntry("  Total Lines Scanned       : " + std::to_string(getLineCount()));
    r.addEntry("  Total Loops Found         : " + std::to_string(totalLoops));
    r.addEntry("  -- for loops              : " + std::to_string(loopCounts.at("for")));
    r.addEntry("  -- while loops            : " + std::to_string(loopCounts.at("while")));
    r.addEntry("  -- do-while loops         : " + std::to_string(loopCounts.at("do")));
    r.addEntry("  -- logarithmic loops      : " + std::to_string(logLoopCount));
    r.addEntry("");
    r.addEntry("  Max Nesting Depth (all)   : " + std::to_string(maxNestingDepth));
    r.addEntry("  Max Regular Loop Depth    : " + std::to_string(maxRegularLoopDepth));
    r.addEntry("  Estimated Time Complexity : " + estimateTimeComplexity());

    return r;
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
    r.addEntry("  Total Lines              : " + std::to_string(getLineCount()));
    r.addEntry("  Total Loops              : " + std::to_string(totalLoops));
    r.addEntry("  -- Logarithmic Loops     : " + std::to_string(logLoopCount));
    r.addEntry("  Total Conditionals       : " + std::to_string(totalConds));
    r.addEntry("  Total Jump Stmts         : " + std::to_string(totalJumps));
    r.addEntry("  Max Nesting Depth (all)  : " + std::to_string(maxNestingDepth));
    r.addEntry("  Max Regular Loop Depth   : " + std::to_string(maxRegularLoopDepth));
    r.addEntry("");
    r.addEntry("  --- Space Complexity Indicators ---");
    r.addEntry("  Array Declarations       : " + std::to_string(arrayDeclarations));
    r.addEntry("  STL Containers           : " + std::to_string(containerDeclarations));
    r.addEntry("");
    r.addEntry("  ====================================");
    r.addEntry("  TIME COMPLEXITY  : " + estimateTimeComplexity());
    r.addEntry("  SPACE COMPLEXITY : " + spaceComplexityEstimate());
    r.addEntry("  ====================================");
    r.addEntry("  Note: complexity is estimated via heuristic pattern matching.");
    r.addEntry("  Logarithmic detection looks for /=2, >>=1, *=2, <<=1,");
    r.addEntry("  and binary-search midpoint (mid = ... / 2) patterns.");

    return r;
}

// ============================================================
//  Public report printers — build report then print
// ============================================================
void CodeComplexityAnalyzer::printNestingReport() const {
    Report r = buildNestingReport();
    r.print();
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
