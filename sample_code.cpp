// sample_code.cpp
// This is a sample C++ file used to test the Code Complexity Analyzer.
// It contains nested loops, conditionals, jump statements, and data structures.

#include <iostream>
#include <vector>
#include <map>
#include <string>

// ---- Bubble Sort function ----
// Time Complexity: O(N^2) due to nested loops
void bubbleSort(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {          // outer loop
        bool swapped = false;
        for (int j = 0; j < n - i - 1; j++) {  // inner loop (nesting depth = 2)
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
                swapped = true;
            }
        }
        if (!swapped) break;  // Jump statement: break
    }
}

// ---- Matrix multiplication ----
// Time Complexity: O(N^3) due to triple nested loops
void matrixMultiply(int a[][3], int b[][3], int result[][3], int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {          // depth 2
            result[i][j] = 0;
            for (int k = 0; k < n; k++) {      // depth 3 — deepest nesting
                result[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

// ---- Student grade classifier ----
// Demonstrates conditionals and switch
std::string classifyGrade(int score) {
    if (score >= 90) {
        return "A";
    } else if (score >= 80) {
        return "B";
    } else if (score >= 70) {
        return "C";
    } else if (score >= 60) {
        return "D";
    } else {
        return "F";
    }
}

// ---- Day name from number ----
// Demonstrates switch statement
std::string getDayName(int day) {
    switch (day) {
        case 1: return "Monday";
        case 2: return "Tuesday";
        case 3: return "Wednesday";
        case 4: return "Thursday";
        case 5: return "Friday";
        default: return "Weekend";
    }
}

// ---- Search function ----
// Demonstrates continue and return jump statements
int linearSearch(std::vector<int>& data, int target) {
    for (int i = 0; i < (int)data.size(); i++) {
        if (data[i] == 0) continue;   // skip zeros
        if (data[i] == target) return i;
    }
    return -1;
}

// ---- Count word frequency using std::map ----
// Space: O(N) due to STL map
void countWords(std::vector<std::string>& words) {
    std::map<std::string, int> freq;
    for (const auto& w : words) {
        freq[w]++;
    }
    for (const auto& pair : freq) {
        std::cout << pair.first << ": " << pair.second << "\n";
    }
}

// ---- Do-while loop example ----
int readPositiveNumber() {
    int num = 0;
    do {
        std::cout << "Enter a positive number: ";
        std::cin >> num;
    } while (num <= 0);
    return num;
}

// ---- Main function ----
int main() {
    // Array declaration
    int arr[10] = {5, 3, 8, 1, 9, 2, 7, 4, 6, 0};

    bubbleSort(arr, 10);

    // Vector (STL container)
    std::vector<int> nums = {10, 20, 0, 40, 50};
    int pos = linearSearch(nums, 40);

    if (pos != -1) {
        std::cout << "Found at index: " << pos << "\n";
    }

    // Grade classification
    std::vector<int> scores = {95, 82, 74, 61, 55};
    for (int score : scores) {
        std::cout << score << " -> " << classifyGrade(score) << "\n";
    }

    // Day name
    for (int i = 1; i <= 7; i++) {
        std::cout << getDayName(i) << "\n";
    }

    return 0;
}
