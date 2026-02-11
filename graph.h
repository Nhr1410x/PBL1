#pragma once
#include <string>
#include <vector>
#include "Global.h"

// ==================== EDGE STRUCT ====================
struct Edge {
    int destination;
    int weight;
    Edge(int dest, int w) : destination(dest), weight(w) {}
};

// ==================== GRAPH CLASS ====================
class Graph {
private:
    int V;
    int E;
    std::vector<std::vector<Edge>> adjList;
    std::vector<std::string> vertexLabels;

public:
    Graph();
    explicit Graph(int vertices);
    ~Graph();

    int getVertexCount() const;
    int getEdgeCount() const;
    const std::vector<std::vector<Edge>>& getAdjacencyList() const;
    const std::vector<std::string>& getVertexLabels() const;
    std::string getVertexLabel(int vertex) const;

    void clear();
    void addVertex(const std::string& label);
    void addEdge(int source, int destination, int weight);
    void removeEdge(int source, int destination);
    bool hasEdge(int source, int destination) const;
    void makeUndirected();

    // Input
    void inputFromKeyboard();

    // File I/O
    bool fileExists(const std::string& filename) const;
    bool readFromFile(const std::string& filename);
    bool readFromFile(const std::string& filename, bool& needCreate);
    bool saveToFile(const std::string& filename) const;
    bool exportForPython(const std::string& filename) const;
    bool exportForPython(const std::vector<int>& path) const;
    bool exportWithPath(const std::string& filename, const std::vector<int>& path) const;

    // Validation
    bool isValid() const;
    bool hasNegativeWeights() const;
};
