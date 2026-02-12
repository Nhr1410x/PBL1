#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <vector>
#include <string>
#include <utility>
#include "Graph.h"


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

class Algorithms {
private:
    const Graph& graph;

    void logStep(std::vector<std::string>& logs, const std::string& message);
    std::vector<int> reconstructPath(int destination, const std::vector<int>& previousVertex) const;

public:
    explicit Algorithms(const Graph& g);

    PathResult dijkstra(int start, bool showSteps = false);

    PathResult bellmanFord(int start, bool showSteps = false);

    std::vector<int> getShortestPath(const PathResult& result, int destination) const;

    int getDistance(const PathResult& result, int destination) const;
};

#endif
