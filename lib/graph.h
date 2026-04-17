#pragma once
#include <string>
#include <tuple>
#include <vector>
#include "Global.h"

// Forward declaration để tránh circular dependency
struct TraceStep;

struct Edge {
    int destination;
    int weight;
    Edge(int dest, int w) : destination(dest), weight(w) {}
};

class Graph {
private:
    using ParsedEdge = std::tuple<int, int, int>;

    int V;
    int E;
    std::vector<std::vector<Edge>> adjList;
    std::vector<std::string> vertexLabels;

    bool hasEdgeWithWeight(int source, int destination, int weight) const;

    void replaceGraph(
        int vertices,
        const std::vector<std::string>& labels,
        const std::vector<ParsedEdge>& edges,
        bool isDirected
    );

public:
    Graph();
    explicit Graph(int vertices);
    ~Graph();

    int getVertexCount() const;
    int getEdgeCount() const;
    const std::vector<std::vector<Edge>>& getAdjacencyList() const;
    std::string getVertexLabel(int vertex) const;

    void clear();
    void addVertex(const std::string& label);
    void addEdge(int source, int destination, int weight);
    bool hasEdge(int source, int destination) const;
    void makeUndirected();

    bool fileExists(const std::string& filename) const;
    bool readFromFile(const std::string& filename, bool& needCreate);
    bool loadFromEdgeListText(
        int numVertices,
        const std::string& edgeText,
        bool isDirected,
        std::string& errorMessage
    );
    bool saveToFile(const std::string& filename) const;
    bool exportForPython(const std::string& filename) const;
    bool exportForPython(const std::vector<int>& path) const;
    bool exportWithPath(const std::string& filename, const std::vector<int>& path) const;

    // Xuất file trace.json cho animation từng bước
    bool exportTraceJson(
        const std::string& filename,
        const std::string& algorithmName,
        int startVertex,
        int endVertex,
        const std::vector<TraceStep>& steps,
        const std::vector<int>& shortestPath
    ) const;

    bool isValid() const;
    bool hasNegativeWeights() const;
};
