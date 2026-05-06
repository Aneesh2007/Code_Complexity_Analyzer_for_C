#ifndef CODEPARSER_H
#define CODEPARSER_H

#include <string>
#include <vector>
#include <stdexcept>

// ============================================================
//  CodeParser.h — Handles file loading and preprocessing
//  Demonstrates: File Handling (std::ifstream), Exception Handling
// ============================================================

class CodeParser {
private:
    std::string filePath;
    std::vector<std::string> lines;

    // Strips // single-line comments and handles /* block comments */ (multi-line aware).
    // inBlockComment is updated in place to carry state across lines.
    std::string stripComment(const std::string& line, bool& inBlockComment) const;

public:
    // Custom exception for file errors
    class FileException : public std::runtime_error {
    public:
        explicit FileException(const std::string& msg) : std::runtime_error(msg) {}
    };

    explicit CodeParser(const std::string& path);

    // Loads the file — throws FileException if file cannot be opened
    void loadFile();

    // Returns all loaded lines
    const std::vector<std::string>& getLines() const { return lines; }

    // Returns the file path
    const std::string& getFilePath() const { return filePath; }

    // Returns total number of lines loaded
    int getLineCount() const { return static_cast<int>(lines.size()); }
};

#endif // CODEPARSER_H
