#include <iostream>
#include <iomanip>
#include <cstdint>
#include <omp.h>

using namespace std;

const uint64_t N = 100000000ULL;
const uint64_t STUDENT_TICKET_NUMBER = 431419;
const int BLOCK_SIZE = 10 * STUDENT_TICKET_NUMBER; // 4314190

double CalculatePiOpenMP(int threadCount, double& timeSec) {
    double sum = 0.0;

    omp_set_num_threads(threadCount);

    double startTime = omp_get_wtime();

#pragma omp parallel for schedule(dynamic, BLOCK_SIZE) reduction(+:sum)
    for (int64_t i = 0; i < static_cast<int64_t>(N); ++i) {
        double x = (static_cast<double>(i) + 0.5) / static_cast<double>(N);
        sum += 4.0 / (1.0 + x * x);
    }

    double endTime = omp_get_wtime();

    timeSec = endTime - startTime;

    return sum / static_cast<double>(N);
}

int main() {
    cout << "========== Lab 3.2 OpenMP ==========\n";
    cout << "Formula: pi = (sum(4 / (1 + x_i^2))) / N\n";
    cout << "x_i = (i + 0.5) / N\n";
    cout << "N = " << N << endl;
    cout << "Student ticket number = " << STUDENT_TICKET_NUMBER << endl;
    cout << "Block size = 10 * student ticket number = " << BLOCK_SIZE << endl;
    cout << "Schedule type: dynamic\n";
    cout << "Threads to test: 1, 2, 4, 8, 12, 16\n";
    cout << "====================================\n\n";

    int threadCounts[] = { 1, 2, 4, 8, 12, 16 };

    cout << left << setw(12) << "Threads"
        << setw(18) << "Time, sec"
        << setw(20) << "Pi value"
        << endl;

    cout << "-------------------------------------------------\n";

    for (int tc : threadCounts) {
        double timeSec = 0.0;
        double pi = CalculatePiOpenMP(tc, timeSec);

        cout << left << setw(12) << tc
            << setw(18) << fixed << setprecision(6) << timeSec
            << setw(20) << setprecision(12) << pi
            << endl;
    }

    cout << "\nDone.\n";
    return 0;
}