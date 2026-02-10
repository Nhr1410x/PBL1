#ifndef COMPARISON_H
#define COMPARISON_H

#include <string>
#include <vector>
#include <chrono>
#include "Algorithms.h"
#include "Graph.h"

// ==================== PERFORMANCE METRICS ====================
struct PerformanceMetrics {
    std::string algorithmName;
    long long executionTimeUs;       // Microseconds
    long long memoryUsageBytes;      // Bytes
    int distancesCalculated;
    double complexity;              // Estimated O(...)
    bool success;

    PerformanceMetrics() : algorithmName(""), executionTimeUs(0), 
                          memoryUsageBytes(0), distancesCalculated(0), 
                          complexity(0.0), success(false) {}
};

// ==================== COMPARISON REPORT ====================
struct ComparisonReport {
    int startVertex;
    int V;  // Number of vertices
    int E;  // Number of edges
    std::vector<PerformanceMetrics> metrics;
    std::vector<std::string> logs;

    ComparisonReport() : startVertex(-1), V(0), E(0) {}
};

// ==================== COMPARISON CLASS ====================
class Comparison {
private:
    const Graph& graph;
    Algorithms algorithms;

public:
    // Constructor
    explicit Comparison(const Graph& g);

    // Performance comparison
    ComparisonReport comparePerformance(int startVertex, AlgorithmType type = AlgorithmType::BOTH);

    // Individual algorithm timing
    PerformanceMetrics measureAlgorithm(int startVertex, AlgorithmType type);

    // Theoretical complexity estimation
    std::string estimateComplexity(const std::string& algorithmName, int V, int E);

    // Generate report
    void generateReport(const ComparisonReport& report, const std::string& filename);
};

#endif
