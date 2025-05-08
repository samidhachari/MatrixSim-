#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>
#include <random>
#include <fstream>  // Include for file writing

using namespace std;

const int M = 1024; // Rows in A
const int K = 1024; // Columns in A / Rows in B
const int N = 1024; // Columns in B

const int mac_units = 64;        // Number of MAC units
const int mem_latency_L1 = 4;    // Cycles to load one row/column from L1 cache
const int mem_latency_L2 = 10;   // Cycles for L2 cache
const int mem_latency_L3 = 30;   // Cycles for L3 cache
const int mem_latency_DRAM = 100; // Cycles for DRAM
const int mac_latency = 1;       // Cycles per MAC operation
const int num_threads = 8;       // Number of threads for parallelism

// Memory access simulation for cache levels
int get_memory_latency(int tile_id, int* cache_L1, int* cache_L2, int* cache_L3, int cache_size) {
    if (cache_L1[tile_id] > 0) return mem_latency_L1;
    if (cache_L2[tile_id] > 0) return mem_latency_L2;
    if (cache_L3[tile_id] > 0) return mem_latency_L3;
    return mem_latency_DRAM;
}

void matrix_multiply_thread(int start_row, int end_row, vector<vector<float>>& A, vector<vector<float>>& B, vector<vector<float>>& C, atomic<int>& total_cycles, atomic<int>& load_cycles, atomic<int>& compute_cycles, int* cache_L1, int* cache_L2, int* cache_L3) {
    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < N; j++) {
            // Simulate memory fetch for A row and B column
            total_cycles += get_memory_latency(i, cache_L1, cache_L2, cache_L3, M) * 2;
            load_cycles += get_memory_latency(i, cache_L1, cache_L2, cache_L3, M) * 2;

            // Simulate dot product using MAC units
            int operations = K;
            int cycles = (operations + mac_units - 1) / mac_units * mac_latency;
            total_cycles += cycles;
            compute_cycles += cycles;

            // Actual computation (dot product)
            for (int k = 0; k < K; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

int main() {
    // Dummy matrices A and B
    vector<vector<float>> A(M, vector<float>(K, 1.0));
    vector<vector<float>> B(K, vector<float>(N, 1.0));
    vector<vector<float>> C(M, vector<float>(N, 0.0));

    atomic<int> total_cycles(0);
    atomic<int> compute_cycles(0);
    atomic<int> load_cycles(0);

    // Initialize cache for simulation (cache line hits)
    int* cache_L1 = new int[M];  // Simulate cache for A rows
    int* cache_L2 = new int[M];  // Simulate cache for A rows
    int* cache_L3 = new int[M];  // Simulate cache for A rows
    for (int i = 0; i < M; i++) {
        cache_L1[i] = rand() % 2; // Randomly simulate cache hits/misses
        cache_L2[i] = rand() % 2;
        cache_L3[i] = rand() % 2;
    }

    // Create threads to simulate parallel processing
    vector<thread> threads;
    int rows_per_thread = M / num_threads;

    for (int t = 0; t < num_threads; t++) {
        int start_row = t * rows_per_thread;
        int end_row = (t == num_threads - 1) ? M : (t + 1) * rows_per_thread;
        threads.push_back(thread(matrix_multiply_thread, start_row, end_row, ref(A), ref(B), ref(C), ref(total_cycles), ref(load_cycles), ref(compute_cycles), cache_L1, cache_L2, cache_L3));
    }

    // Wait for threads to finish
    for (auto& t : threads) {
        t.join();
    }

    // Report
    cout << "Matrix Multiplication Performance Model\n";
    cout << "Matrix size: " << M << "x" << K << " x " << K << "x" << N << "\n";
    cout << "MAC Units: " << mac_units << "\n";
    cout << "Total Cycles: " << total_cycles << "\n";
    cout << "Compute Cycles: " << compute_cycles << "\n";
    cout << "Memory Load Cycles: " << load_cycles << "\n";
    cout << "Utilization: " << (100.0 * compute_cycles / total_cycles) << "%\n";

    // Write results to CSV file
    ofstream file("matrix_multiplication_performance.csv");
    file << "Total Cycles,Compute Cycles,Load Cycles,Utilization (%)\n";
    file << total_cycles << "," << compute_cycles << "," << load_cycles << "," << (100.0 * compute_cycles / total_cycles) << "\n";
    file.close();

    // Cleanup
    delete[] cache_L1;
    delete[] cache_L2;
    delete[] cache_L3;

    return 0;
}
