#include "Comparison.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>

// ==================== CONSTRUCTOR ====================
Comparison::Comparison(const Graph& g) : graph(g), algorithms(g) {}

// ==================== PERFORMANCE MEASUREMENT ====================
PerformanceMetrics Comparison::measureAlgorithm(int startVertex, AlgorithmType type) {
    PerformanceMetrics metrics;
    int V = graph.getVertexCount();
    int E = graph.getEdgeCount();

    if (type == AlgorithmType::DIJKSTRA) {
        metrics.algorithmName = "Dijkstra";
        
        // Check for negative weights
        if (graph.hasNegativeWeights()) {
            metrics.success = false;
            metrics.executionTimeUs = 0;
            return metrics;
        }

        auto start = std::chrono::high_resolution_clock::now();
        PathResult result = algorithms.dijkstra(startVertex, false);
        auto end = std::chrono::high_resolution_clock::now();

        metrics.executionTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        metrics.distancesCalculated = result.distances.size();
        metrics.memoryUsageBytes = (V * sizeof(int) * 2) + (E * sizeof(Edge));
        metrics.complexity = E * std::log(V);
        metrics.success = result.success;

    } else if (type == AlgorithmType::BELLMAN_FORD) {
        metrics.algorithmName = "Bellman-Ford";

        auto start = std::chrono::high_resolution_clock::now();
        PathResult result = algorithms.bellmanFord(startVertex, false);
        auto end = std::chrono::high_resolution_clock::now();

        metrics.executionTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        metrics.distancesCalculated = result.distances.size();
        metrics.memoryUsageBytes = (V * sizeof(int) * 2) + (E * sizeof(Edge));
        metrics.complexity = V * E;
        metrics.success = result.success && !result.hasNegativeCycle;
    }

    return metrics;
}

// ==================== COMPARISON REPORT ====================
ComparisonReport Comparison::comparePerformance(int startVertex, AlgorithmType type) {
    ComparisonReport report;
    report.startVertex = startVertex;
    report.V = graph.getVertexCount();
    report.E = graph.getEdgeCount();

    report.logs.push_back("========================================");
    report.logs.push_back("PERFORMANCE COMPARISON REPORT");
    report.logs.push_back("========================================");
    report.logs.push_back("Start Vertex: " + graph.getVertexLabel(startVertex));
    report.logs.push_back("Vertices (V): " + std::to_string(report.V));
    report.logs.push_back("Edges (E): " + std::to_string(report.E));
    report.logs.push_back("");

    if (type == AlgorithmType::DIJKSTRA || type == AlgorithmType::BOTH) {
        auto metrics = measureAlgorithm(startVertex, AlgorithmType::DIJKSTRA);
        report.metrics.push_back(metrics);

        report.logs.push_back("--- DIJKSTRA'S ALGORITHM ---");
        report.logs.push_back("Execution Time: " + std::to_string(metrics.executionTimeUs) + " μs");
        report.logs.push_back("Memory Usage: " + std::to_string(metrics.memoryUsageBytes) + " bytes");
        report.logs.push_back("Complexity: O(E log V) ≈ O(" + std::to_string((int)metrics.complexity) + ")");
        report.logs.push_back("Status: " + std::string(metrics.success ? "Success" : "Failed"));
        report.logs.push_back("");
    }

    if (type == AlgorithmType::BELLMAN_FORD || type == AlgorithmType::BOTH) {
        auto metrics = measureAlgorithm(startVertex, AlgorithmType::BELLMAN_FORD);
        report.metrics.push_back(metrics);

        report.logs.push_back("--- BELLMAN-FORD ALGORITHM ---");
        report.logs.push_back("Execution Time: " + std::to_string(metrics.executionTimeUs) + " μs");
        report.logs.push_back("Memory Usage: " + std::to_string(metrics.memoryUsageBytes) + " bytes");
        report.logs.push_back("Complexity: O(V × E) ≈ O(" + std::to_string((int)metrics.complexity) + ")");
        report.logs.push_back("Status: " + std::string(metrics.success ? "Success" : "Failed"));
        report.logs.push_back("");
    }

    if (type == AlgorithmType::BOTH && report.metrics.size() == 2) {
        report.logs.push_back("--- COMPARISON ---");
        auto dijkstraTime = report.metrics[0].executionTimeUs;
        auto bellmanTime = report.metrics[1].executionTimeUs;
        
        if (bellmanTime > 0 && dijkstraTime > 0) {
            double ratio = static_cast<double>(bellmanTime) / dijkstraTime;
            report.logs.push_back("Bellman-Ford is " + std::to_string(ratio) + "x slower than Dijkstra");
        } else {
            report.logs.push_back("Insufficient timing resolution for ratio.");
        }
        report.logs.push_back("");
    }

    report.logs.push_back("========================================");

    return report;
}

// ==================== COMPLEXITY ESTIMATION ====================
std::string Comparison::estimateComplexity(const std::string& algorithmName, int V, int E) {
    std::ostringstream oss;
    
    if (algorithmName == "Dijkstra") {
        oss << "O(E log V) = O(" << E << " * log " << V << ") ≈ O(" 
            << (int)(E * std::log(V)) << ")";
    } else if (algorithmName == "Bellman-Ford") {
        oss << "O(V * E) = O(" << V << " * " << E << ") = O(" << (V * E) << ")";
    }
    
    return oss.str();
}

// ==================== REPORT GENERATION ====================
void Comparison::generateReport(const ComparisonReport& report, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return;
    }

    for (const auto& log : report.logs) {
        file << log << "\n";
    }

    file.close();
}
