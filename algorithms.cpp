#include "Algorithms.h"
#include <queue>
#include <limits>
#include <sstream>
#include <cmath>

// ==================== CONSTRUCTOR ====================
Algorithms::Algorithms(const Graph& g) : graph(g) {}

// ==================== HELPER METHODS ====================
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

// ==================== DIJKSTRA'S ALGORITHM ====================
PathResult Algorithms::dijkstra(int start, bool showSteps) {
    PathResult result;
    result.startVertex = start;
    int V = graph.getVertexCount();
    const auto& adjList = graph.getAdjacencyList();

    // Initialize distances and previous vertices
    const int INF = std::numeric_limits<int>::max();
    result.distances.assign(V, INF);
    result.previousVertex.assign(V, -1);
    result.distances[start] = 0;

    // Priority queue: pair<distance, vertex>
    std::priority_queue<std::pair<int, int>, 
                       std::vector<std::pair<int, int>>,
                       std::greater<std::pair<int, int>>> pq;
    pq.push({0, start});

    if (showSteps) {
        logStep(result.logs, "=== DIJKSTRA'S ALGORITHM ===");
        logStep(result.logs, "Starting vertex: " + graph.getVertexLabel(start));
        logStep(result.logs, "Initializing distances: all = INF except start = 0");
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
            logStep(result.logs, "\n[Iteration " + std::to_string(iterations) + "]");
            logStep(result.logs, "Processing vertex: " + graph.getVertexLabel(u) + 
                                 " (distance = " + std::to_string(dist) + ")");
        }

        // Relax edges
        for (const auto& edge : adjList[u]) {
            int v = edge.destination;
            int weight = edge.weight;

            if (result.distances[u] != INF && 
                result.distances[u] + weight < result.distances[v]) {
                
                result.distances[v] = result.distances[u] + weight;
                result.previousVertex[v] = u;
                pq.push({result.distances[v], v});

                if (showSteps) {
                    logStep(result.logs, "  Relax: " + graph.getVertexLabel(u) + " -> " + 
                                         graph.getVertexLabel(v) + 
                                         " (new distance = " + std::to_string(result.distances[v]) + ")");
                }
            }
        }
    }

    if (showSteps) {
        logStep(result.logs, "\n=== FINAL DISTANCES ===");
        for (int i = 0; i < V; i++) {
            if (result.distances[i] == INF) {
                logStep(result.logs, graph.getVertexLabel(i) + " = INF (unreachable)");
            } else {
                logStep(result.logs, graph.getVertexLabel(i) + " = " + std::to_string(result.distances[i]));
            }
        }
    }

    result.success = true;
    return result;
}

// ==================== BELLMAN-FORD ALGORITHM ====================
PathResult Algorithms::bellmanFord(int start, bool showSteps) {
    PathResult result;
    result.startVertex = start;
    int V = graph.getVertexCount();
    const auto& adjList = graph.getAdjacencyList();

    // Initialize distances and previous vertices
    const int INF = std::numeric_limits<int>::max();
    result.distances.assign(V, INF);
    result.previousVertex.assign(V, -1);
    result.distances[start] = 0;

    if (showSteps) {
        logStep(result.logs, "=== BELLMAN-FORD ALGORITHM ===");
        logStep(result.logs, "Starting vertex: " + graph.getVertexLabel(start));
        logStep(result.logs, "Initializing distances: all = INF except start = 0");
    }

    // Relax edges V-1 times
    for (int i = 0; i < V - 1; i++) {
        bool updated = false;

        if (showSteps) {
            logStep(result.logs, "\n[Pass " + std::to_string(i + 1) + "/" + std::to_string(V - 1) + "]");
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
                        logStep(result.logs, "  Relax: " + graph.getVertexLabel(u) + " -> " + 
                                             graph.getVertexLabel(v) + 
                                             " (new distance = " + std::to_string(result.distances[v]) + ")");
                    }
                }
            }
        }

        if (!updated && showSteps) {
            logStep(result.logs, "  (No updates in this pass - early termination possible)");
        }
    }

    // Check for negative cycles
    result.hasNegativeCycle = false;
    if (showSteps) {
        logStep(result.logs, "\n=== CHECKING FOR NEGATIVE CYCLES ===");
    }

    for (int u = 0; u < V; u++) {
        for (const auto& edge : adjList[u]) {
            int v = edge.destination;
            int weight = edge.weight;

            if (result.distances[u] != INF && 
                result.distances[u] + weight < result.distances[v]) {
                result.hasNegativeCycle = true;

                if (showSteps) {
                    logStep(result.logs, "NEGATIVE CYCLE DETECTED!");
                    logStep(result.logs, "Edge: " + graph.getVertexLabel(u) + " -> " + 
                                         graph.getVertexLabel(v) + 
                                         " (weight = " + std::to_string(weight) + ")");
                }
            }
        }
    }

    if (!result.hasNegativeCycle && showSteps) {
        logStep(result.logs, "No negative cycles detected.");
    }

    if (showSteps) {
        logStep(result.logs, "\n=== FINAL DISTANCES ===");
        for (int i = 0; i < V; i++) {
            if (result.distances[i] == INF) {
                logStep(result.logs, graph.getVertexLabel(i) + " = INF (unreachable)");
            } else {
                logStep(result.logs, graph.getVertexLabel(i) + " = " + std::to_string(result.distances[i]));
            }
        }
    }

    result.success = !result.hasNegativeCycle;
    return result;
}

// ==================== UTILITY METHODS ====================
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
