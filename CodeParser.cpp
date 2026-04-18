#include "CodeParser.h"
#include <fstream>
#include <sstream>
#include <iostream>

// ============================================================
//  CodeParser.cpp — File I/O implementation
// ============================================================

CodeParser::CodeParser(const std::string& path) : filePath(path) {}

// Strips inline // comments so they don't confuse keyword detection
std::string CodeParser::stripComment(const std::string& line) const {
    // Find "//" but not inside a string literal (basic heuristic)
    bool inString = false;
    for (size_t i = 0; i < line.size(); ++i) {
        if (line[i] == '"') inString = !inString;
        if (!inString && i + 1 < line.size() &&
            line[i] == '/' && line[i + 1] == '/') {
            return line.substr(0, i); // Cut everything after //
        }
    }
    return line;
}

void CodeParser::loadFile() {
    std::ifstream fileStream(filePath);

    // Check if file opened successfully — throw custom exception if not
    if (!fileStream.is_open()) {
        throw FileException("Cannot open file: '" + filePath +
                            "'. Please check the path and try again.");
    }

    lines.clear();
    std::string rawLine;
    while (std::getline(fileStream, rawLine)) {
        std::string cleaned = stripComment(rawLine);
        lines.push_back(cleaned);
    }
    fileStream.close();
    if (lines.empty()) {
        throw FileException("File '" + filePath + "' is empty or unreadable.");
    }

    std::cout << "  [OK] Loaded " << lines.size() << " lines from '"
              << filePath << "'.\n";\
    std::cout << " This code is for O powered k time for this version "<<std::endl;
}
