#include <iostream>
#include <string>
#include <vector>
#include <memory>   // std::unique_ptr (smart pointer — no manual delete)
#include <limits>

#include "CodeParser.h"
#include "Analyzer.h"
#include "CodeComplexityAnalyzer.h"

// ============================================================
//  main.cpp — Entry point and menu controller
//  C/C++ Code Complexity Analyzer
//  Demonstrates: Polymorphism (Analyzer* base pointer),
//                Smart Pointers, Exception Handling, Menu UI
// ============================================================

// ---- Helper: clear input buffer ----
void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// ---- Helper: print the application banner ----
void printBanner() {
    std::cout << "\n";
    std::cout << "*          C/C++ CODE COMPLEXITY ANALYZER                  *\n";
    std::cout << "*      Analyze Time & Space Complexity of Source Code      *\n";
    std::cout << "************************************************************\n";
}

// ---- Helper: print the main file-loading menu ----
void printMainMenu() {
    std::cout << "\n------------------------------------------------------------\n";
    std::cout << "  MAIN MENU\n";
    std::cout << "------------------------------------------------------------\n";
    std::cout << "  1. Load file from current directory (enter filename)\n";
    std::cout << "  2. Load file from full path\n";
    std::cout << "  3. Exit\n";
    std::cout << "------------------------------------------------------------\n";
    std::cout << "  Enter choice: ";
}

// ---- Helper: print the analysis submenu ----
void printAnalysisMenu(const std::string& loadedFile) {
    std::cout << "\n------------------------------------------------------------\n";
    std::cout << "  ANALYSIS MENU  [File: " << loadedFile << "]\n";
    std::cout << "------------------------------------------------------------\n";
    std::cout << "  1. Analyze Nesting Depth\n";
    std::cout << "  2. Analyze Jump Statements\n";
    std::cout << "  3. Full Complexity Report (Time & Space)\n";
    std::cout << "  4. Load a different file\n";
    std::cout << "  5. Exit\n";
    std::cout << "------------------------------------------------------------\n";
    std::cout << "  Enter choice: ";
}

// ============================================================
//  runAnalysisMenu — uses BASE CLASS POINTER (polymorphism)
//  The Analyzer* doesn't know it's a CodeComplexityAnalyzer
//  until runtime — this demonstrates runtime polymorphism.
// ============================================================
void runAnalysisMenu(Analyzer* analyzer,
                     CodeComplexityAnalyzer* concreteAnalyzer,
                     const std::string& filename) {
    int choice = 0;

    while (true) {
        printAnalysisMenu(filename);
        std::cin >> choice;
        clearInput();

        switch (choice) {
            case 1:
                // Polymorphic call path: calls overridden analyze() internally
                concreteAnalyzer->printNestingReport();
                break;
            case 2:
                concreteAnalyzer->printJumpReport();
                break;
            case 3:
                // Calls printReport() through BASE POINTER — polymorphism!
                analyzer->printReport();
                break;
            case 4:
                return; // Go back to main menu to load a new file
            case 5:
                std::cout << "\n  Exiting. Thank you!\n\n";
                exit(0);
            default:
                std::cout << "  [!] Invalid choice. Please enter 1-5.\n";
        }
    }
}

// ============================================================
//  main — Application entry point
// ============================================================
int main() {
    printBanner();

    int mainChoice = 0;

    while (true) {
        printMainMenu();
        std::cin >> mainChoice;
        clearInput();

        if (mainChoice == 3) {
            std::cout << "\n  Exiting. Thank you!\n\n";
            break;
        }

        if (mainChoice != 1 && mainChoice != 2) {
            std::cout << "  [!] Invalid choice. Please enter 1, 2, or 3.\n";
            continue;
        }

        // ---- Get the file path from user ----
        std::string filePath;
        if (mainChoice == 1) {
            std::cout << "  Enter filename (e.g., sample_code.cpp): ";
        } else {
            std::cout << "  Enter full path (e.g., /home/user/project/main.cpp): ";
        }
        std::getline(std::cin, filePath);

        if (filePath.empty()) {
            std::cout << "  [!] No file name entered. Try again.\n";
            continue;
        }

        // ---- Load file using CodeParser (exception-safe) ----
        try {
            std::cout << "\n  Loading file...\n";
            CodeParser parser(filePath);
            parser.loadFile(); // Throws CodeParser::FileException on error

            const std::vector<std::string>& lines = parser.getLines();

            // ---- Build the analyzer ----
            // Use unique_ptr for automatic memory management
            auto concreteAnalyzer = std::make_unique<CodeComplexityAnalyzer>(lines);

            // Polymorphic base pointer — demonstrates runtime polymorphism
            Analyzer* analyzerBase = concreteAnalyzer.get();

            // Run the analysis pass (calls overridden analyze())
            std::cout << "  Analyzing code structure...\n";
            analyzerBase->analyze();
            std::cout << "  Analysis complete.\n";

            // Enter the analysis submenu
            runAnalysisMenu(analyzerBase, concreteAnalyzer.get(), filePath);

        } catch (const CodeParser::FileException& e) {
            std::cout << "\n  [ERROR] " << e.what() << "\n";
        } catch (const std::exception& e) {
            std::cout << "\n  [UNEXPECTED ERROR] " << e.what() << "\n";
        }
    }

    return 0;
}
