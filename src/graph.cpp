#include "../lib/Graph.h"
#include <filesystem>
#include <fstream>

Graph::Graph() : V(0), E(0) {
    std::filesystem::create_directories(DATA_FOLDER);
}

Graph::Graph(int vertices) : V(0), E(0) {
    std::filesystem::create_directories(DATA_FOLDER);
    for (int i = 0; i < vertices; i++) {
        addVertex("V" + std::to_string(i));
    }
}

Graph::~Graph() {
    clear();
}
int Graph::getVertexCount() const {
    return V;
}

int Graph::getEdgeCount() const {
    return E;
}

const std::vector<std::vector<Edge>>& Graph::getAdjacencyList() const {
    return adjList;
}

std::string Graph::getVertexLabel(int vertex) const {
    if (vertex >= 0 && vertex < V) {
        return vertexLabels[vertex];
    }
    return "Invalid";
}

void Graph::clear() {
    adjList.clear();
    vertexLabels.clear();
    V = 0;
    E = 0;
}

void Graph::addVertex(const std::string& label) {
    adjList.push_back(std::vector<Edge>());
    vertexLabels.push_back(label);
    V++;
}

void Graph::addEdge(int source, int destination, int weight) {
    if (source < 0 || source >= V || destination < 0 || destination >= V) {
        return;
    }

    for (auto& edge : adjList[source]) {
        if (edge.destination == destination) {
            edge.weight = weight;
            return;
        }
    }

    adjList[source].push_back(Edge(destination, weight));
    E++;
}

bool Graph::hasEdge(int source, int destination) const {
    if (source < 0 || source >= V) {
        return false;
    }
    for (const auto& edge : adjList[source]) {
        if (edge.destination == destination) {
            return true;
        }
    }
    return false;
}

void Graph::makeUndirected() {
    for (int i = 0; i < V; i++) {
        for (const auto& edge : adjList[i]) {
            if (!hasEdge(edge.destination, i)) {
                addEdge(edge.destination, i, edge.weight);
            }
        }
    }
}

bool Graph::fileExists(const std::string& filename) const {
    return std::filesystem::exists(filename);
}

bool Graph::readFromFile(const std::string& filename, bool& needCreate) {
    needCreate = false;

    if (!fileExists(filename)) {
        needCreate = true;
        return false;
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    clear();

    int numVertices = 0;
    if (!(file >> numVertices)) {
        return false;
    }

    for (int i = 0; i < numVertices; i++) {
        std::string label;
        if (!(file >> label)) {
            label = std::to_string(i + 1);
        }
        addVertex(label);
    }

    int numEdges = 0;
    if (!(file >> numEdges)) {
        return false;
    }

    for (int i = 0; i < numEdges; i++) {
        int source = 0, dest = 0, weight = 0;
        if (file >> source >> dest >> weight) {
            addEdge(source - 1, dest - 1, weight);
        }
    }

    return true;
}

bool Graph::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    file << V << "\n";
    for (const auto& label : vertexLabels) {
        file << label << " ";
    }
    file << "\n";

    file << E << "\n";
    for (int i = 0; i < V; i++) {
        for (const auto& edge : adjList[i]) {
            file << (i + 1) << " " << (edge.destination + 1) << " " << edge.weight << "\n";
        }
    }

    return true;
}

bool Graph::exportForPython(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    file << V << "\n";
    for (const auto& label : vertexLabels) {
        file << label << " ";
    }
    file << "\n";

    for (int i = 0; i < V; i++) {
        for (const auto& edge : adjList[i]) {
            file << (i + 1) << " " << (edge.destination + 1) << " " << edge.weight << "\n";
        }
    }

    return true;
}

bool Graph::exportWithPath(const std::string& filename, const std::vector<int>& path) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    file << V << "\n";
    for (const auto& label : vertexLabels) {
        file << label << " ";
    }
    file << "\n";

    for (int i = 0; i < V; i++) {
        for (const auto& edge : adjList[i]) {
            file << (i + 1) << " " << (edge.destination + 1) << " " << edge.weight << "\n";
        }
    }

    file << "\nPATH:\n";
    for (int v : path) {
        file << (v + 1) << " ";
    }
    file << "\n";

    return true;
}

bool Graph::exportForPython(const std::vector<int>& path) const {
    return exportWithPath(TEMP_EXPORT_FILE, path);
}

bool Graph::isValid() const {
    return V > 0;
}

bool Graph::hasNegativeWeights() const {
    for (int i = 0; i < V; i++) {
        for (const auto& edge : adjList[i]) {
            if (edge.weight < 0) {
                return true;
            }
        }
    }
    return false;
}
