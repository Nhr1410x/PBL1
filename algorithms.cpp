#include "Algorithms.h"
#include <queue>
#include <limits>
#include <sstream>
#include <cmath>
#include <iomanip>

namespace {
std::vector<std::string> formatDistanceTable(const Graph& graph, const std::vector<int>& distances) {
    std::vector<std::string> lines;
    if (distances.empty()) return lines;

    const int INF = std::numeric_limits<int>::max();
    std::ostringstream header;
    std::ostringstream values;

    header << "Đỉnh :";
    values << "D(i) :";

    for (size_t i = 0; i < distances.size(); ++i) {
        const std::string label = graph.getVertexLabel(static_cast<int>(i));
        header << std::setw(6) << label;
        if (distances[i] == INF) {
            values << std::setw(6) << "INF";
        } else {
            values << std::setw(6) << distances[i];
        }
    }

    lines.push_back(header.str());
    lines.push_back(values.str());
    return lines;
}
} // namespace

Algorithms::Algorithms(const Graph& g) : graph(g) {}

void Algorithms::logStep(std::vector<std::string>& logs, const std::string& message) {
    logs.push_back(message);
}

std::vector<int> Algorithms::reconstructPath(int destination, const std::vector<int>& previousVertex) const {
    std::vector<int> path;
    int current = destination;
    while (current != -1) {
        path.insert(path.begin(), current);
        current = previousVertex[current];
    }
    return path;
}


// Dijkstra 
PathResult Algorithms::dijkstra(int start, bool showSteps) {
    PathResult result;
    result.startVertex = start;
    int V = graph.getVertexCount();
    const auto& adjList = graph.getAdjacencyList();

    const int INF = std::numeric_limits<int>::max();
    result.distances.assign(V, INF);
    result.previousVertex.assign(V, -1);
    result.distances[start] = 0;

    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>,std::greater<std::pair<int, int>>> pq;
    pq.push({0, start});

    if (showSteps) {
        logStep(result.logs, "              ======= THUẬT TOÁN DIJKSTRA =======");
        logStep(result.logs, "Đỉnh bắt đầu: " + graph.getVertexLabel(start));
        logStep(result.logs, "Khởi tạo khoảng cách: tất cả = INF, riêng đỉnh bắt đầu = 0");
    }

    std::vector<bool> visited(V, false);
    int iterations = 0;

    while (!pq.empty()) {
        auto [dist, u] = pq.top();
        pq.pop();

        if (visited[u]) continue;
        visited[u] = true;
        iterations++;

        if (showSteps) {
            logStep(result.logs, "");
            logStep(result.logs, "[Lần lặp " + std::to_string(iterations) + "]");
            logStep(result.logs, "Xử lý đỉnh: " + graph.getVertexLabel(u) +
                                 " (khoảng cách = " + std::to_string(dist) + ")");
        }

        for (const auto& edge : adjList[u]) {
            int v = edge.destination;
            int weight = edge.weight;

            if (result.distances[u] != INF && 
                result.distances[u] + weight < result.distances[v]) {
                
                result.distances[v] = result.distances[u] + weight;
                result.previousVertex[v] = u;
                pq.push({result.distances[v], v});

                if (showSteps) {
                    logStep(result.logs, "  Cập nhật: " + graph.getVertexLabel(u) + " -> " +
                                         graph.getVertexLabel(v) +
                                         " (khoảng cách mới = " + std::to_string(result.distances[v]) + ")");
                }
            }
        }

        if (showSteps) {
            logStep(result.logs, "Khoảng cách sau lần lặp " + std::to_string(iterations) + ":");
            auto table = formatDistanceTable(graph, result.distances);
            for (const auto& line : table) {
                logStep(result.logs, line);
            }
        }
    }

    if (showSteps) {
        logStep(result.logs, "");
        logStep(result.logs, "                  === KHOẢNG CÁCH CUỐI ===");
        for (int i = 0; i < V; i++) {
            if (result.distances[i] == INF) {
                logStep(result.logs,"dist[" + std::to_string(i+1) + "] = INF (không tồn tại đường đi)");
            } 
            else {
                logStep(result.logs,"dist[" + std::to_string(i+1) + "] = " + std::to_string(result.distances[i]));
            }
        }
    }

    result.success = true;
    return result;
}

//bellman
PathResult Algorithms::bellmanFord(int start, bool showSteps) {
    PathResult result;
    result.startVertex = start;
    int V = graph.getVertexCount();
    const auto& adjList = graph.getAdjacencyList();

    const int INF = std::numeric_limits<int>::max();
    result.distances.assign(V, INF);
    result.previousVertex.assign(V, -1);
    result.distances[start] = 0;

    if (showSteps) {
        logStep(result.logs, "           ======= THUẬT TOÁN BELLMAN-FORD =======");
        logStep(result.logs, "Đỉnh bắt đầu: " + graph.getVertexLabel(start-1));
        logStep(result.logs, "Khởi tạo khoảng cách: tất cả = INF, riêng đỉnh bắt đầu = 0");
    }

    for (int i = 0; i < V - 1; i++) {
        bool updated = false;

        if (showSteps) {
            logStep(result.logs, "");
            logStep(result.logs, "[Lượt " + std::to_string(i + 1) + "/" + std::to_string(V - 1) + "]");
        }

        for (int u = 0; u < V; u++) {
            for (const auto& edge : adjList[u]) {
                int v = edge.destination;
                int weight = edge.weight;

                if (result.distances[u] != INF && 
                    result.distances[u] + weight < result.distances[v]) {
                    
                    result.distances[v] = result.distances[u] + weight;
                    result.previousVertex[v] = u;
                    updated = true;

                    if (showSteps) {
                        logStep(result.logs, "  Cập nhật: " + graph.getVertexLabel(u) + " -> " +
                                             graph.getVertexLabel(v) +
                                             " (khoảng cách mới = " + std::to_string(result.distances[v]) + ")");
                    }
                }
            }
        }

        if (!updated && showSteps) {
            logStep(result.logs, "  (Không có cập nhật ở lượt này - có thể dừng sớm)");
        }

        if (showSteps) {
            logStep(result.logs, "Bảng khoảng cách sau lượt " + std::to_string(i + 1) + ":");
            auto table = formatDistanceTable(graph, result.distances);
            for (const auto& line : table) {
                logStep(result.logs, line);
            }
        }
    }

    result.hasNegativeCycle = false;
    if (showSteps) {
        logStep(result.logs, "");
        logStep(result.logs, "=== KIỂM TRA CHU TRÌNH ÂM ===");
    }

    for (int u = 0; u < V; u++) {
        for (const auto& edge : adjList[u]) {
            int v = edge.destination;
            int weight = edge.weight;

            if (result.distances[u] != INF && 
                result.distances[u] + weight < result.distances[v]) {
                result.hasNegativeCycle = true;

                if (showSteps) {
                    logStep(result.logs, "PHÁT HIỆN CHU TRÌNH ÂM!");
                    logStep(result.logs, "Cạnh: " + graph.getVertexLabel(u) + " -> " +
                                         graph.getVertexLabel(v) +
                                         " (trọng số = " + std::to_string(weight) + ")");
                }
            }
        }
    }

    if (!result.hasNegativeCycle && showSteps) {
        logStep(result.logs, "Không phát hiện chu trình âm.");
    }

    if (showSteps) {
        logStep(result.logs, "");
        logStep(result.logs, "=== KHOẢNG CÁCH CUỐI ===");
        for (int i = 0; i < V; i++) {
            if (result.distances[i] == INF) {
                logStep(result.logs, graph.getVertexLabel(i) + " = INF (không tới được)");
            } else {
                logStep(result.logs, graph.getVertexLabel(i) + " = " + std::to_string(result.distances[i]));
            }
        }
    }

    result.success = !result.hasNegativeCycle;
    return result;
}

std::vector<int> Algorithms::getShortestPath(const PathResult& result, int destination) const {
    if (destination < 0 || destination >= result.previousVertex.size()) {
        return {};
    }
    if (result.distances.empty()) {
        return {};
    }
    if (result.distances[destination] == std::numeric_limits<int>::max()) {
        return {};
    }
    return reconstructPath(destination, result.previousVertex);
}

int Algorithms::getDistance(const PathResult& result, int destination) const {
    if (destination < 0 || destination >= result.distances.size()) {
        return -1;
    }
    return result.distances[destination];
}

void Algorithms::visualizePath(const PathResult& result, int destination, const std::string& filename) {
    auto path = getShortestPath(result, destination);
    graph.exportWithPath(filename, path);
}
