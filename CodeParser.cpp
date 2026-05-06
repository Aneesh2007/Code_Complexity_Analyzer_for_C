#include "CodeParser.h"
#include <fstream>
#include <iostream>

// ============================================================
//  CodeParser.cpp — File I/O implementation
// ============================================================

CodeParser::CodeParser(const std::string& path) : filePath(path) {}

// ============================================================
//  stripComment — removes // single-line comments and
//  /* block comments */ (including multi-line blocks).
//  inBlockComment is passed by reference to carry state across
//  successive calls (one call per source line).
// ============================================================
std::string CodeParser::stripComment(const std::string& line, bool& inBlockComment) const {
    std::string result;
    bool inString = false;

    for (size_t i = 0; i < line.size(); ++i) {
        if (inBlockComment) {
            // Look for the block-comment closing sequence '*/'
            if (i + 1 < line.size() && line[i] == '*' && line[i + 1] == '/') {
                inBlockComment = false;
                ++i; // consume the '/'
            }
            // All characters inside a block comment are dropped
        } else {
            // Toggle string tracking so we don't treat /* or // inside
            // a string literal as a comment opener
            if (line[i] == '"') inString = !inString;

            if (!inString) {
                // Single-line comment: cut the rest of the line
                if (i + 1 < line.size() && line[i] == '/' && line[i + 1] == '/') {
                    break;
                }
                // Block-comment open: set flag and skip the '/*'
                if (i + 1 < line.size() && line[i] == '/' && line[i + 1] == '*') {
                    inBlockComment = true;
                    ++i; // consume the '*'
                    continue;
                }
            }
            result += line[i];
        }
    }
    return result;
}

void CodeParser::loadFile() {
    std::ifstream fileStream(filePath);

    // Check if file opened successfully — throw custom exception if not
    if (!fileStream.is_open()) {
        throw FileException("Cannot open file: '" + filePath +
                            "'. Please check the path and try again.");
    }

    lines.clear();
    bool inBlockComment = false; // tracks multi-line /* ... */ blocks
    std::string rawLine;
    while (std::getline(fileStream, rawLine)) {
        std::string cleaned = stripComment(rawLine, inBlockComment);
        lines.push_back(cleaned);
    }
    fileStream.close();

    if (lines.empty()) {
        throw FileException("File '" + filePath + "' is empty or unreadable.");
    }

    std::cout << "  [OK] Loaded " << lines.size() << " lines from '"
              << filePath << "'.\n";
}
