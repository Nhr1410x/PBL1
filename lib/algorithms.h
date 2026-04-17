#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <vector>
#include <string>
#include <utility>
#include "Graph.h"

// Một bước trong quá trình thực thi thuật toán (dùng để xuất trace.json)
struct TraceStep {
    std::string type;  // "visit" | "relax" | "negative_cycle"
    int node     = -1; // ID đỉnh đang xét (cho "visit")
    int from     = -1; // Đỉnh nguồn cạnh đang relax (cho "relax")
    int to       = -1; // Đỉnh đích cạnh đang relax (cho "relax")
    int dist     =  0; // Khoảng cách hiện tại
    int new_dist =  0; // Khoảng cách mới sau khi relax
    std::string desc;  // Mô tả cho animation title
};

struct PathResult {
    bool success;
    int startVertex;
    std::vector<int> distances;
    std::vector<int> previousVertex;
    std::vector<int> shortestPath;
    std::vector<std::pair<int, std::string>> logs;
    std::vector<TraceStep> traceSteps; // lịch sử từng bước cho animation
    bool hasNegativeCycle;

    PathResult() : success(false), startVertex(-1), hasNegativeCycle(false) {}
};

class Algorithms {
private:
    const Graph& graph;

    void logStep(std::vector<std::pair<int, std::string>>& logs, int color, const std::string& message);
    std::vector<int> reconstructPath(int destination, const std::vector<int>& previousVertex) const;

public:
    explicit Algorithms(const Graph& g);

    PathResult dijkstra(int start, bool showSteps = false);

    PathResult bellmanFord(int start, bool showSteps = false);

    std::vector<int> getShortestPath(const PathResult& result, int destination) const;

    int getDistance(const PathResult& result, int destination) const;
};

#endif
