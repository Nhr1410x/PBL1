#include "../lib/Comparison.h"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>

Comparison::Comparison(const Graph& g) : graph(g), algorithms(g), advancedAlgorithms(g) {}

std::string Comparison::escapeJson(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (unsigned char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:   out += static_cast<char>(c); break;
        }
    }
    return out;
}

PerformanceMetrics Comparison::measureAlgorithm(int startVertex, AlgorithmType type) {
    PerformanceMetrics metrics;
    int V = graph.getVertexCount();
    int E = graph.getEdgeCount();

    if (type == AlgorithmType::DIJKSTRA) {
        metrics.algorithmName = "Dijkstra";
        metrics.complexityLabel = "O(E log V)";
        
        // kiểm tra trong số âm
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
        metrics.complexityLabel = "O(V × E)";

        auto start = std::chrono::high_resolution_clock::now();
        PathResult result = algorithms.bellmanFord(startVertex, false);
        auto end = std::chrono::high_resolution_clock::now();

        metrics.executionTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        metrics.distancesCalculated = result.distances.size();
        metrics.memoryUsageBytes = (V * sizeof(int) * 2) + (E * sizeof(Edge));
        metrics.complexity = V * E;
        metrics.success = result.success && !result.hasNegativeCycle;
    } else if (type == AlgorithmType::ADVANCED_BMSSP) {
        metrics.algorithmName = "BMSSP";
        metrics.complexityLabel = "BMSSP 2025";

        // Thuật toán mới chỉ áp dụng cho trọng số không âm (theo implementation hiện tại)
        if (graph.hasNegativeWeights()) {
            metrics.success = false;
            metrics.executionTimeUs = 0;
            return metrics;
        }

        auto start = std::chrono::high_resolution_clock::now();
        PathResult result = advancedAlgorithms.breakingSortingBarrier(startVertex, false);
        auto end = std::chrono::high_resolution_clock::now();

        metrics.executionTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        metrics.distancesCalculated = static_cast<int>(result.distances.size());

        // Ước lượng thô: distances + previous + một phần overhead. (Không đo RSS thực tế.)
        metrics.memoryUsageBytes =
            (V * sizeof(int) * 2) +                   // distances + previous
            (E * sizeof(Edge)) +                      // adjacency
            (V * static_cast<int>(sizeof(long long))); // ước lượng distances_ nội bộ

        metrics.complexity = E * std::log(std::max(2, V));
        metrics.success = result.success;
    }

    return metrics;
}


ComparisonReport Comparison::comparePerformance(int startVertex, AlgorithmType type) {
    ComparisonReport report;
    report.startVertex = startVertex;
    report.V = graph.getVertexCount();
    report.E = graph.getEdgeCount();

    // Requirement: always measure all 3 algorithms.
    (void)type;
    report.metrics.clear();
    report.metrics.push_back(measureAlgorithm(startVertex, AlgorithmType::DIJKSTRA));
    report.metrics.push_back(measureAlgorithm(startVertex, AlgorithmType::BELLMAN_FORD));
    report.metrics.push_back(measureAlgorithm(startVertex, AlgorithmType::ADVANCED_BMSSP));

    exportStatsToJson(report.metrics);

    return report;
}

bool Comparison::exportStatsToJson(const std::vector<PerformanceMetrics>& metrics) const {
    std::error_code ec;
    std::filesystem::create_directories(DATA_FOLDER, ec);

    const std::string outPath = "../data/compare_stats.json";
    std::ofstream file(outPath);
    if (!file.is_open()) {
        return false;
    }

    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"stats\": [\n";
    for (std::size_t i = 0; i < metrics.size(); ++i) {
        const auto& m = metrics[i];
        ss << "    {";
        ss << "\"name\": \"" << escapeJson(m.algorithmName) << "\"";
        ss << ", \"time_us\": " << m.executionTimeUs;
        ss << ", \"memory_bytes\": " << m.memoryUsageBytes;
        ss << "}";
        if (i + 1 < metrics.size()) ss << ",";
        ss << "\n";
    }
    ss << "  ]\n";
    ss << "}\n";

    file << ss.str();
    return file.good();
}
