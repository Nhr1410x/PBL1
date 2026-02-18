#ifndef COMPARISON_H
#define COMPARISON_H

#include <string>
#include <vector>
#include <chrono>
#include "Algorithms.h"
#include "Graph.h"

struct PerformanceMetrics {
    std::string algorithmName;
    long long executionTimeUs;       // tinh bang micro giay
    long long memoryUsageBytes;      // byte
    int distancesCalculated;
    double complexity;              // do phuc tap
    bool success;

    PerformanceMetrics() : algorithmName(""), executionTimeUs(0), 
                          memoryUsageBytes(0), distancesCalculated(0), 
                          complexity(0.0), success(false) {}
};


struct ComparisonReport {
    int startVertex;
    int V;  // só dinh
    int E;  // canh
    std::vector<PerformanceMetrics> metrics;
    std::vector<std::string> logs;

    ComparisonReport() : startVertex(-1), V(0), E(0) {}
};

class Comparison {
private:
    const Graph& graph;
    Algorithms algorithms;

public:
    explicit Comparison(const Graph& g);

    ComparisonReport comparePerformance(int startVertex, AlgorithmType type = AlgorithmType::BOTH);

    PerformanceMetrics measureAlgorithm(int startVertex, AlgorithmType type);
};

#endif
