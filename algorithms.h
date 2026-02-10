#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <vector>
#include <string>
#include <utility>
#include "Graph.h"

// ==================== RESULT STRUCTURE ====================
struct PathResult {
    bool success;
    int startVertex;
    std::vector<int> distances;
    std::vector<int> previousVertex;
    std::vector<int> shortestPath;
    std::vector<std::string> logs;
    bool hasNegativeCycle;

    PathResult() : success(false), startVertex(-1), hasNegativeCycle(false) {}
};

// ==================== ALGORITHMS CLASS ====================
class Algorithms {
private:
    const Graph& graph;

    // Helper methods
    void logStep(std::vector<std::string>& logs, const std::string& message);
    std::vector<int> reconstructPath(int destination, const std::vector<int>& previousVertex) const;

public:
    // Constructor
    explicit Algorithms(const Graph& g);

    // Dijkstra's Algorithm (O(E log V) with priority queue)
    PathResult dijkstra(int start, bool showSteps = false);

    // Bellman-Ford Algorithm (O(V * E))
    PathResult bellmanFord(int start, bool showSteps = false);

    // Get shortest path from result
    std::vector<int> getShortestPath(const PathResult& result, int destination) const;

    // Get distance from result
    int getDistance(const PathResult& result, int destination) const;

    // Visualize the path on graph
    void visualizePath(const PathResult& result, int destination, const std::string& filename);
};

#endif
