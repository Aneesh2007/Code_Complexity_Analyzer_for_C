// log_sample.cpp
// Sample file used to verify logarithmic complexity detection.
// Expected analysis results are noted next to each function.

#include <iostream>
#include <vector>

/* ----------------------------------------------------------------
   binarySearch — Expected: O(log N)
   The while loop halves the search space each iteration via
   mid = (lo + hi) / 2, which triggers log-loop detection.
   ---------------------------------------------------------------- */
int binarySearch(const std::vector<int>& arr, int target) {
    int lo = 0;
    int hi = static_cast<int>(arr.size()) - 1;

    while (lo <= hi) {
        int mid = (lo + hi) / 2;   // halving pattern — detected as O(log N)
        if (arr[mid] == target) return mid;
        if (arr[mid] < target)  lo = mid + 1;
        else                    hi = mid - 1;
    }
    return -1;
}

/* ----------------------------------------------------------------
   logCountDown — Expected: O(log N)
   for loop increments i by doubling (*=2), giving log N iterations.
   ---------------------------------------------------------------- */
void logCountDown(int n) {
    for (int i = 1; i < n; i *= 2) {   // i doubles each step — O(log N)
        std::cout << i << " ";
    }
    std::cout << "\n";
}

/* ----------------------------------------------------------------
   shiftSearch — Expected: O(log N)
   Uses right-shift-by-1 (>>=1) to halve a value each iteration.
   ---------------------------------------------------------------- */
int highestBit(unsigned int n) {
    int pos = 0;
    while (n > 1) {
        n >>= 1;   // right-shift by 1 — detected as O(log N)
        ++pos;
    }
    return pos;
}

/* ----------------------------------------------------------------
   searchAllElements — Expected: O(N log N)
   An outer O(N) for loop calls binarySearch (O(log N)) on each
   element — the mid = (lo+hi)/2 inside the inner while loop is
   nested within the outer for loop → O(N log N).
   ---------------------------------------------------------------- */
void searchAllElements(const std::vector<int>& sorted, const std::vector<int>& queries) {
    for (int q : queries) {                         // outer O(N) loop
        int lo = 0;
        int hi = static_cast<int>(sorted.size()) - 1;
        while (lo <= hi) {
            int mid = (lo + hi) / 2;               // log pattern inside outer loop
            if (sorted[mid] == q) { std::cout << q << " found\n"; break; }
            if (sorted[mid] < q)  lo = mid + 1;
            else                  hi = mid - 1;
        }
    }
}

/* ----------------------------------------------------------------
   bubbleSort — Expected: O(N^2)
   Classic doubly-nested loops with no halving — polynomial only.
   ---------------------------------------------------------------- */
void bubbleSort(std::vector<int>& arr) {
    int n = static_cast<int>(arr.size());
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1])
                std::swap(arr[j], arr[j + 1]);
        }
    }
}

/* ----------------------------------------------------------------
   main
   ---------------------------------------------------------------- */
int main() {
    std::vector<int> sorted = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19};
    std::vector<int> queries = {3, 7, 12, 19};

    std::cout << "Binary search for 7: index " << binarySearch(sorted, 7) << "\n";

    std::cout << "Log count-down from 32: ";
    logCountDown(32);

    std::cout << "Highest bit of 100: " << highestBit(100) << "\n";

    std::cout << "Search all elements:\n";
    searchAllElements(sorted, queries);

    std::cout << "Bubble sort demo:\n";
    std::vector<int> unsorted = {5, 3, 8, 1, 9};
    bubbleSort(unsorted);
    for (int v : unsorted) std::cout << v << " ";
    std::cout << "\n";

    return 0;
}
