#include "../lib/Graph.h"
#include "../lib/Algorithms.h"
#include <filesystem>
#include <fstream>
#include <sstream>

namespace {
bool parseIntToken(const std::string& token, int& value) {
    size_t parsed = 0;
    try {
        value = std::stoi(token, &parsed);
    } catch (...) {
        return false;
    }
    return parsed == token.size();
}
}

Graph::Graph() : V(0), E(0) {
    std::error_code ec;
    std::filesystem::create_directories(DATA_FOLDER, ec);
}

Graph::Graph(int vertices) : V(0), E(0) {
    std::error_code ec;
    std::filesystem::create_directories(DATA_FOLDER, ec);
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

void Graph::replaceGraph(
    int vertices,
    const std::vector<std::string>& labels,
    const std::vector<ParsedEdge>& edges,
    bool isDirected
) {
    clear();

    for (int i = 0; i < vertices; ++i) {
        if (i < static_cast<int>(labels.size()) && !labels[i].empty()) {
            addVertex(labels[i]);
        } else {
            addVertex(std::to_string(i + 1));
        }
    }

    for (const auto& [source, destination, weight] : edges) {
        addEdge(source, destination, weight);
    }

    if (!isDirected) {
        makeUndirected();
    }
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

    // Allow parallel edges (same source & destination) with different weights.
    // Do NOT overwrite existing edges.
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

bool Graph::hasEdgeWithWeight(int source, int destination, int weight) const {
    if (source < 0 || source >= V) {
        return false;
    }
    for (const auto& edge : adjList[source]) {
        if (edge.destination == destination && edge.weight == weight) {
            return true;
        }
    }
    return false;
}

void Graph::makeUndirected() {
    for (int i = 0; i < V; i++) {
        for (const auto& edge : adjList[i]) {
            // Preserve parallel edges by mirroring each distinct weight once.
            if (!hasEdgeWithWeight(edge.destination, i, edge.weight)) {
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

    std::vector<std::string> tokens;
    std::string token;
    while (file >> token) {
        tokens.push_back(token);
    }

    if (tokens.empty()) {
        return false;
    }

    int numVertices = 0;
    if (!parseIntToken(tokens.front(), numVertices) || numVertices < 1) {
        return false;
    }

    auto tryParse = [&](bool hasLabels) {
        size_t index = 1;
        std::vector<std::string> labels;

        if (hasLabels) {
            if (tokens.size() < index + static_cast<size_t>(numVertices) + 1) {
                return false;
            }
            labels.assign(tokens.begin() + index, tokens.begin() + index + numVertices);
            index += static_cast<size_t>(numVertices);
        } else {
            labels.reserve(numVertices);
            for (int i = 0; i < numVertices; ++i) {
                labels.push_back(std::to_string(i + 1));
            }
        }

        if (tokens.size() <= index) {
            return false;
        }

        int numEdges = 0;
        if (!parseIntToken(tokens[index], numEdges) || numEdges < 0) {
            return false;
        }
        ++index;

        const size_t expectedTokenCount = index + static_cast<size_t>(numEdges) * 3;
        if (tokens.size() != expectedTokenCount) {
            return false;
        }

        std::vector<ParsedEdge> edges;
        edges.reserve(numEdges);
        for (int i = 0; i < numEdges; ++i) {
            int source = 0;
            int dest = 0;
            int weight = 0;

            if (!parseIntToken(tokens[index], source) ||
                !parseIntToken(tokens[index + 1], dest) ||
                !parseIntToken(tokens[index + 2], weight)) {
                return false;
            }

            if (source < 1 || source > numVertices || dest < 1 || dest > numVertices) {
                return false;
            }

            edges.emplace_back(source - 1, dest - 1, weight);
            index += 3;
        }

        replaceGraph(numVertices, labels, edges, true);
        return true;
    };

    return tryParse(true) || tryParse(false);
}

bool Graph::loadFromEdgeListText(
    int numVertices,
    const std::string& edgeText,
    bool isDirected,
    std::string& errorMessage
) {
    errorMessage.clear();

    if (numVertices < 1 || numVertices > 1000) {
        errorMessage = "Số đỉnh phải là số nguyên từ 1 đến 1000.";
        return false;
    }

    std::istringstream iss(edgeText);
    std::string line;
    int lineNo = 0;
    std::vector<ParsedEdge> edges;

    while (std::getline(iss, line)) {
        ++lineNo;
        if (line.find_first_not_of(" \t\r\n") == std::string::npos) {
            continue;
        }

        std::istringstream lineStream(line);
        int source = 0;
        int destination = 0;
        int weight = 0;
        std::string trailingToken;

        if (!(lineStream >> source >> destination >> weight) || (lineStream >> trailingToken)) {
            errorMessage = "Dòng " + std::to_string(lineNo) +
                           ": định dạng sai (cần đúng 3 giá trị: u v w).";
            return false;
        }

        if (source < 1 || source > numVertices || destination < 1 || destination > numVertices) {
            errorMessage = "Dòng " + std::to_string(lineNo) +
                           ": đỉnh " + std::to_string(source) + " hoặc " +
                           std::to_string(destination) +
                           " nằm ngoài phạm vi [1.." + std::to_string(numVertices) + "].";
            return false;
        }

        edges.emplace_back(source - 1, destination - 1, weight);
    }

    std::vector<std::string> labels;
    labels.reserve(numVertices);
    for (int i = 0; i < numVertices; ++i) {
        labels.push_back(std::to_string(i + 1));
    }

    replaceGraph(numVertices, labels, edges, isDirected);
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

// ============================================================
//  exportTraceJson — build JSON bằng std::ostringstream
// ============================================================
static std::string escapeJson(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 4);
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

bool Graph::exportTraceJson(
    const std::string& filename,
    const std::string& algorithmName,
    int startVertex,
    int endVertex,
    const std::vector<TraceStep>& steps,
    const std::vector<int>& shortestPath
) const {
    std::ofstream file(filename);
    if (!file.is_open()) return false;

    std::ostringstream ss;

    ss << "{\n";
    ss << "  \"algorithm\": \"" << escapeJson(algorithmName) << "\",\n";
    ss << "  \"start\": " << startVertex << ",\n";
    ss << "  \"end\": "   << endVertex   << ",\n";

    // nodes
    ss << "  \"nodes\": [\n";
    for (int i = 0; i < V; ++i) {
        ss << "    {\"id\": " << i << ", \"label\": \"" << escapeJson(vertexLabels[i]) << "\"}";
        if (i < V - 1) ss << ",";
        ss << "\n";
    }
    ss << "  ],\n";

    // edges
    ss << "  \"edges\": [\n";
    bool firstEdge = true;
    for (int u = 0; u < V; ++u) {
        for (const auto& edge : adjList[u]) {
            if (!firstEdge) ss << ",\n";
            ss << "    {\"source\": " << u
               << ", \"target\": " << edge.destination
               << ", \"weight\": " << edge.weight << "}";
            firstEdge = false;
        }
    }
    ss << "\n  ],\n";

    // steps
    ss << "  \"steps\": [\n";
    for (size_t i = 0; i < steps.size(); ++i) {
        const auto& st = steps[i];
        ss << "    {";
        ss << "\"type\": \"" << escapeJson(st.type) << "\"";
        if (st.node >= 0)  ss << ", \"node\": " << st.node;
        if (st.from >= 0)  ss << ", \"from\": " << st.from;
        if (st.to   >= 0)  ss << ", \"to\": "   << st.to;
        ss << ", \"dist\": "     << st.dist;
        ss << ", \"new_dist\": " << st.new_dist;
        ss << ", \"desc\": \""   << escapeJson(st.desc) << "\"";
        ss << "}";
        if (i + 1 < steps.size()) ss << ",";
        ss << "\n";
    }
    ss << "  ],\n";

    // shortest_path
    ss << "  \"shortest_path\": [";
    for (size_t i = 0; i < shortestPath.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << shortestPath[i];
    }
    ss << "]\n}\n";

    file << ss.str();
    return file.good();
}
