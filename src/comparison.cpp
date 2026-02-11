#include "../lib/Comparison.h"
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

    report.logs.push_back("        ========================================");
    report.logs.push_back("                BÁO CÁO SO SÁNH HIỆU NĂNG");
    report.logs.push_back("        ========================================");
    report.logs.push_back("Đỉnh bắt đầu: " + std::to_string(startVertex + 1) + "   Số đỉnh (V): " + std::to_string(report.V) + "  Số cạnh (E): " + std::to_string(report.E));

    if (type == AlgorithmType::DIJKSTRA || type == AlgorithmType::BOTH) {
        auto metrics = measureAlgorithm(startVertex, AlgorithmType::DIJKSTRA);
        report.metrics.push_back(metrics);
    }

    if (type == AlgorithmType::BELLMAN_FORD || type == AlgorithmType::BOTH) {
        auto metrics = measureAlgorithm(startVertex, AlgorithmType::BELLMAN_FORD);
        report.metrics.push_back(metrics);
    }

    if (type == AlgorithmType::BOTH && report.metrics.size() == 2) {
        const auto& d = report.metrics[0];
        const auto& b = report.metrics[1];

        const int labelW = 16;
        const int colW = 20;

        auto utf8Len = [](const std::string& s) {
            int count = 0;
            for (unsigned char c : s) {
                if ((c & 0xC0) != 0x80) {
                    count++;
                }
            }
            return count;
        };

        auto fit = [&](const std::string& s, int width) {
            if (width <= 0) return std::string();
            int len = utf8Len(s);
            if (len == width) return s;
            if (len < width) return s + std::string(width - len, ' ');

            std::string out;
            out.reserve(s.size());
            int count = 0;
            for (size_t i = 0; i < s.size() && count < width; i++) {
                unsigned char c = static_cast<unsigned char>(s[i]);
                if ((c & 0xC0) != 0x80) {
                    if (count >= width) break;
                    count++;
                }
                out.push_back(s[i]);
            }
            int outLen = utf8Len(out);
            if (outLen < width) {
                out += std::string(width - outLen, ' ');
            }
            return out;
        };

        auto row = [&](const std::string& label, const std::string& dv, const std::string& bv) {
            return std::string("|") + fit(label, labelW) + "|"
                + fit(dv, colW) + "|" + fit(bv, colW) + "|";
        };

        std::string border = "+" + std::string(labelW, '-') +
                             "+" + std::string(colW, '-') +
                             "+" + std::string(colW, '-') + "+";

        auto fmtStatus = [](bool ok) {
            return ok ? std::string("Thành công") : std::string("Thất bại");
        };

        auto fmtComplexity = [](double value, const std::string& form) {
            return form + " ≈ O(" + std::to_string(static_cast<int>(value)) + ")";
        };

        report.logs.push_back(border);
        report.logs.push_back(row("", "DIJKSTRA", "BELLMAN-FORD"));
        report.logs.push_back(border);
        report.logs.push_back(row("Thời gian chạy", std::to_string(d.executionTimeUs) + " us",
                                  std::to_string(b.executionTimeUs) + " us"));
        report.logs.push_back(row("Bộ nhớ dùng", std::to_string(d.memoryUsageBytes) + " bytes",
                                  std::to_string(b.memoryUsageBytes) + " bytes"));
        report.logs.push_back(row("Độ phức tạp",
                                  fmtComplexity(d.complexity, "O(E log V)"),
                                  fmtComplexity(b.complexity, "O(V × E)")));
        report.logs.push_back(row("Trạng thái", fmtStatus(d.success), fmtStatus(b.success)));
        report.logs.push_back(border);
    } 
    // else if (type == AlgorithmType::DIJKSTRA && report.metrics.size() == 1) {
    //     const auto& metrics = report.metrics[0];
    //     report.logs.push_back("--- THUẬT TOÁN DIJKSTRA ---");
    //     report.logs.push_back("Thời gian chạy: " + std::to_string(metrics.executionTimeUs) + " us");
    //     report.logs.push_back("Bộ nhớ sử dụng: " + std::to_string(metrics.memoryUsageBytes) + " bytes");
    //     report.logs.push_back("Độ phức tạp: O(E log V) ≈ O(" + std::to_string((int)metrics.complexity) + ")");
    //     report.logs.push_back("Trạng thái: " + std::string(metrics.success ? "Thành công" : "Thất bại"));
    //     report.logs.push_back("");
    // } else if (type == AlgorithmType::BELLMAN_FORD && report.metrics.size() == 1) {
    //     const auto& metrics = report.metrics[0];
    //     report.logs.push_back("--- THUẬT TOÁN BELLMAN-FORD ---");
    //     report.logs.push_back("Thời gian chạy: " + std::to_string(metrics.executionTimeUs) + " us");
    //     report.logs.push_back("Bộ nhớ sử dụng: " + std::to_string(metrics.memoryUsageBytes) + " bytes");
    //     report.logs.push_back("Độ phức tạp: O(V × E) ≈ O(" + std::to_string((int)metrics.complexity) + ")");
    //     report.logs.push_back("Trạng thái: " + std::string(metrics.success ? "Thành công" : "Thất bại"));
    //     report.logs.push_back("");
    // }

    if (type == AlgorithmType::BOTH && report.metrics.size() == 2) {
        report.logs.push_back("                        --- SO SÁNH ---");
        auto dijkstraTime = report.metrics[0].executionTimeUs;
        auto bellmanTime = report.metrics[1].executionTimeUs;

        if (bellmanTime > 0 && dijkstraTime > 0) {
            double ratio = static_cast<double>(bellmanTime) / dijkstraTime;
            report.logs.push_back("Bellman-Ford chậm hơn Dijkstra " + std::to_string(ratio) + " lần");
        } else {
            report.logs.push_back("Không đủ độ chênh lệch thời gian để so sánh.");
        }
        report.logs.push_back("");
    }


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
