#include "../lib/GraphCanvas.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {
const double C_NODE_DEFAULT[3] = {176 / 255.0, 190 / 255.0, 197 / 255.0}; // Chua xet
const double C_NODE_VISIT[3]   = {255 / 255.0, 152 / 255.0, 0 / 255.0};   // Dang xet (Visit)
const double C_NODE_DONE[3]    = {79 / 255.0, 195 / 255.0, 247 / 255.0};  // Da xu ly xong
const double C_NODE_START[3]   = {255 / 255.0, 214 / 255.0, 10 / 255.0};  // Dinh bat dau
const double C_NODE_END[3]     = {239 / 255.0, 83 / 255.0, 80 / 255.0};   // Dinh dich
const double C_NODE_PATH[3]    = {102 / 255.0, 187 / 255.0, 106 / 255.0};
const double C_NODE_HOVER[3]   = {1.0, 1.0, 1.0};

const double C_EDGE_DEFAULT[3] = {176 / 255.0, 176 / 255.0, 176 / 255.0};
const double C_EDGE_RELAX[3]   = {255 / 255.0, 51 / 255.0, 51 / 255.0};
const double C_EDGE_PATH[3]    = {102 / 255.0, 187 / 255.0, 106 / 255.0};

long long edgeKey(int u, int v) {
    return (static_cast<long long>(u) << 32) ^ static_cast<unsigned int>(v);
}
} // namespace

GraphCanvas::GraphCanvas()
    : m_isDirected(true),
      m_hasData(false),
      m_currentStep(0),
      m_speedMs(900),
      m_isPlaying(false),
      m_sceneCenter({0.0, 0.0}),
      m_sceneWidth(240.0),
      m_sceneHeight(240.0),
      m_zoom(1.0),
      m_pan({0.0, 0.0}),
      m_hoveredNode(-1),
      m_hoverScreen({0.0, 0.0}),
      m_draggingNode(-1),
      m_clickCandidateNode(-1),
      m_isPanning(false),
      m_pointerMovedSincePress(false),
      m_pressScreen({0.0, 0.0}),
      m_lastPointerScreen({0.0, 0.0}),
      m_visitingNode(-1),
      m_relaxingEdge({-1, -1}) {
    set_size_request(600, 600);
    add_events(
        Gdk::BUTTON_PRESS_MASK |
        Gdk::BUTTON_RELEASE_MASK |
        Gdk::POINTER_MOTION_MASK |
        Gdk::SCROLL_MASK |
        Gdk::LEAVE_NOTIFY_MASK
    );
}

GraphCanvas::~GraphCanvas() {
    if (m_timeoutConnection.connected()) {
        m_timeoutConnection.disconnect();
    }
}

void GraphCanvas::loadData(const Graph& g, const PathResult& res, bool isDirected) {
    m_graph = g;
    m_result = res;
    m_isDirected = isDirected;
    m_hasData = true;

    m_pathEdges.clear();
    for (std::size_t i = 1; i < res.shortestPath.size(); ++i) {
        const int u = res.shortestPath[i - 1];
        const int v = res.shortestPath[i];
        m_pathEdges.insert(edgeKey(u, v));
        if (!m_isDirected) {
            m_pathEdges.insert(edgeKey(v, u));
        }
    }

    calculateLayout();
    cacheSceneBounds();
    resetView();
    resetAnimation();
}

void GraphCanvas::setAnimationSpeed(int ms) {
    m_speedMs = ms;
    if (m_isPlaying) {
        pauseAnimation();
        playAnimation();
    }
}

GraphCanvas::type_signal_vertex_selected& GraphCanvas::signal_start_vertex_selected() {
    return m_signalStartVertexSelected;
}

GraphCanvas::type_signal_vertex_selected& GraphCanvas::signal_end_vertex_selected() {
    return m_signalEndVertexSelected;
}

void GraphCanvas::playAnimation() {
    if (!m_hasData || m_isPlaying) {
        return;
    }
    m_isPlaying = true;
    m_timeoutConnection = Glib::signal_timeout().connect(
        sigc::mem_fun(*this, &GraphCanvas::on_timeout),
        m_speedMs
    );
}

void GraphCanvas::pauseAnimation() {
    if (!m_isPlaying) {
        return;
    }
    m_isPlaying = false;
    if (m_timeoutConnection.connected()) {
        m_timeoutConnection.disconnect();
    }
}

void GraphCanvas::resetAnimation() {
    pauseAnimation();
    m_currentStep = 0;
    rebuildDistancesForCurrentStep();
    queue_draw();
}

void GraphCanvas::resetView() {
    m_zoom = 1.0;
    m_pan = {0.0, 0.0};
    queue_draw();
}

bool GraphCanvas::on_timeout() {
    const int totalSteps = static_cast<int>(m_result.traceSteps.size());
    if (m_currentStep >= totalSteps) {
        m_isPlaying = false;
        return false;
    }

    ++m_currentStep;
    rebuildDistancesForCurrentStep();
    queue_draw();

    if (m_currentStep >= totalSteps) {
        m_isPlaying = false;
        return false;
    }
    return true;
}

void GraphCanvas::rebuildDistancesForCurrentStep() {
    m_distances.clear();
    m_visited.clear();
    m_visitingNode = -1;
    m_relaxingEdge = {-1, -1};

    const int V = m_graph.getVertexCount();
    for (int i = 0; i < V; ++i) {
        m_distances[i] = 1000000000;
    }

    if (m_result.startVertex >= 0 && m_result.startVertex < V) {
        m_distances[m_result.startVertex] = 0;
    }

    int activeVisit = -1;
    const int appliedSteps = std::min(m_currentStep, static_cast<int>(m_result.traceSteps.size()));
    for (int i = 0; i < appliedSteps; ++i) {
        const auto& step = m_result.traceSteps[i];
        if (step.type == "relax") {
            m_distances[step.to] = step.new_dist;
            if (i == appliedSteps - 1) {
                m_relaxingEdge = {step.from, step.to};
            }
        } else if (step.type == "visit") {
            if (activeVisit != -1 && activeVisit != step.node) {
                m_visited.insert(activeVisit);
            }
            activeVisit = step.node;
        }
    }

    if (appliedSteps >= static_cast<int>(m_result.traceSteps.size())) {
        if (activeVisit != -1) {
            m_visited.insert(activeVisit);
        }
        m_visitingNode = -1;
    } else {
        m_visitingNode = activeVisit;
    }
}

double GraphCanvas::distance(Point p1, Point p2) const {
    return std::sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

double GraphCanvas::currentNodeRadius() const {
    return std::clamp(24.0 * std::sqrt(std::max(0.45, m_zoom)), 19.5, 33.0);
}

void GraphCanvas::buildNeighborGraph(std::vector<std::vector<int>>& neighbors) const {
    const int V = m_graph.getVertexCount();
    neighbors.assign(V, {});
    const auto& adj = m_graph.getAdjacencyList();

    for (int u = 0; u < V; ++u) {
        for (const auto& edge : adj[u]) {
            const int v = edge.destination;
            if (v < 0 || v >= V || v == u) {
                continue;
            }
            neighbors[u].push_back(v);
            neighbors[v].push_back(u);
        }
    }

    for (auto& row : neighbors) {
        std::sort(row.begin(), row.end());
        row.erase(std::unique(row.begin(), row.end()), row.end());
    }
}

void GraphCanvas::assignPrimaryLayers(
    const std::vector<std::vector<int>>& neighbors,
    std::vector<int>& layers,
    int& maxLayer
) const {
    const int V = static_cast<int>(neighbors.size());
    layers.assign(V, -1);
    maxLayer = 0;
    if (V == 0) {
        return;
    }

    auto degreeOf = [&](int node) -> int {
        return static_cast<int>(neighbors[node].size());
    };

    int start = (m_result.startVertex >= 0 && m_result.startVertex < V)
        ? m_result.startVertex
        : -1;
    if (start == -1) {
        start = 0;
        for (int i = 1; i < V; ++i) {
            if (degreeOf(i) > degreeOf(start)) {
                start = i;
            }
        }
    }

    layers[start] = 0;
    std::queue<int> q;
    q.push(start);

    if (m_result.previousVertex.size() == static_cast<std::size_t>(V)) {
        std::vector<std::vector<int>> treeChildren(V);
        for (int v = 0; v < V; ++v) {
            const int parent = m_result.previousVertex[v];
            if (parent >= 0 && parent < V && parent != v) {
                treeChildren[parent].push_back(v);
            }
        }

        std::vector<char> seenTree(V, 0);
        seenTree[start] = 1;
        std::queue<int> treeQ;
        treeQ.push(start);
        while (!treeQ.empty()) {
            const int u = treeQ.front();
            treeQ.pop();
            maxLayer = std::max(maxLayer, layers[u]);
            for (int child : treeChildren[u]) {
                if (layers[child] == -1 || layers[child] > layers[u] + 1) {
                    layers[child] = layers[u] + 1;
                }
                if (!seenTree[child]) {
                    seenTree[child] = 1;
                    treeQ.push(child);
                }
            }
        }
    }

    for (int i = 0; i < V; ++i) {
        if (layers[i] >= 0 && i != start) {
            q.push(i);
        }
    }

    while (!q.empty()) {
        const int u = q.front();
        q.pop();
        maxLayer = std::max(maxLayer, layers[u]);
        for (int v : neighbors[u]) {
            if (layers[v] == -1) {
                layers[v] = layers[u] + 1;
                q.push(v);
            }
        }
    }

    std::vector<char> visited(V, 0);
    for (int i = 0; i < V; ++i) {
        if (layers[i] >= 0) {
            visited[i] = 1;
        }
    }

    int nextComponentBase = maxLayer + 2;
    while (true) {
        int seed = -1;
        int bestDegree = -1;
        for (int i = 0; i < V; ++i) {
            if (visited[i]) {
                continue;
            }
            if (degreeOf(i) > bestDegree) {
                bestDegree = degreeOf(i);
                seed = i;
            }
        }
        if (seed == -1) {
            break;
        }

        std::queue<int> cq;
        cq.push(seed);
        visited[seed] = 1;
        layers[seed] = nextComponentBase;
        int componentMax = layers[seed];

        while (!cq.empty()) {
            const int u = cq.front();
            cq.pop();
            componentMax = std::max(componentMax, layers[u]);
            for (int v : neighbors[u]) {
                if (visited[v]) {
                    continue;
                }
                visited[v] = 1;
                layers[v] = layers[u] + 1;
                cq.push(v);
            }
        }

        maxLayer = std::max(maxLayer, componentMax);
        nextComponentBase = componentMax + 2;
    }

    const int minLayer = *std::min_element(layers.begin(), layers.end());
    if (minLayer > 0) {
        for (int& l : layers) {
            l -= minLayer;
        }
    }
    maxLayer = *std::max_element(layers.begin(), layers.end());
}

void GraphCanvas::orderNodesWithinLayers(
    const std::vector<std::vector<int>>& neighbors,
    const std::vector<int>& layers,
    std::vector<std::vector<int>>& layerNodes
) const {
    if (layers.empty()) {
        layerNodes.clear();
        return;
    }

    const int maxLayer = *std::max_element(layers.begin(), layers.end());
    layerNodes.assign(maxLayer + 1, {});
    for (int i = 0; i < static_cast<int>(layers.size()); ++i) {
        layerNodes[layers[i]].push_back(i);
    }

    auto degreeOf = [&](int node) -> int {
        return static_cast<int>(neighbors[node].size());
    };

    for (auto& layer : layerNodes) {
        std::sort(layer.begin(), layer.end(), [&](int a, int b) {
            if (degreeOf(a) != degreeOf(b)) {
                return degreeOf(a) > degreeOf(b);
            }
            return a < b;
        });
    }

    std::vector<double> rank(layers.size(), 0.0);
    auto refreshRanks = [&]() {
        for (const auto& layer : layerNodes) {
            for (std::size_t idx = 0; idx < layer.size(); ++idx) {
                rank[layer[idx]] = static_cast<double>(idx);
            }
        }
    };

    auto medianNeighborRank = [&](int node, int targetLayer) -> double {
        std::vector<double> rs;
        rs.reserve(neighbors[node].size());
        for (int nb : neighbors[node]) {
            if (layers[nb] == targetLayer) {
                rs.push_back(rank[nb]);
            }
        }
        if (rs.empty()) {
            return rank[node];
        }
        std::sort(rs.begin(), rs.end());
        const std::size_t mid = rs.size() / 2;
        if (rs.size() % 2 == 0) {
            return (rs[mid - 1] + rs[mid]) * 0.5;
        }
        return rs[mid];
    };

    constexpr int kSweepPasses = 6;
    for (int pass = 0; pass < kSweepPasses; ++pass) {
        refreshRanks();
        for (int l = 1; l <= maxLayer; ++l) {
            auto& nodes = layerNodes[l];
            std::stable_sort(nodes.begin(), nodes.end(), [&](int a, int b) {
                const double ma = medianNeighborRank(a, l - 1);
                const double mb = medianNeighborRank(b, l - 1);
                if (std::abs(ma - mb) > 1e-6) {
                    return ma < mb;
                }
                if (degreeOf(a) != degreeOf(b)) {
                    return degreeOf(a) > degreeOf(b);
                }
                return a < b;
            });
        }

        refreshRanks();
        for (int l = maxLayer - 1; l >= 0; --l) {
            auto& nodes = layerNodes[l];
            std::stable_sort(nodes.begin(), nodes.end(), [&](int a, int b) {
                const double ma = medianNeighborRank(a, l + 1);
                const double mb = medianNeighborRank(b, l + 1);
                if (std::abs(ma - mb) > 1e-6) {
                    return ma < mb;
                }
                if (degreeOf(a) != degreeOf(b)) {
                    return degreeOf(a) > degreeOf(b);
                }
                return a < b;
            });
        }
    }
}

void GraphCanvas::initializeLayerPositions(
    const std::vector<std::vector<int>>& layerNodes,
    std::vector<Point>& positions
) const {
    const int V = m_graph.getVertexCount();
    positions.assign(V, {0.0, 0.0});
    if (V == 0) {
        return;
    }

    const double minSpacing = estimateMinimumNodeSpacing();
    const double layerSpacing = minSpacing * 1.90;
    const double rowSpacing = minSpacing * 1.12;
    const double subColumnSpacing = minSpacing * 0.92;

    for (int l = 0; l < static_cast<int>(layerNodes.size()); ++l) {
        const auto& nodes = layerNodes[l];
        const int n = static_cast<int>(nodes.size());
        if (n == 0) {
            continue;
        }

        const int maxRowsPerColumn = std::max(
            4,
            static_cast<int>(std::ceil(std::sqrt(static_cast<double>(n)) * 1.18))
        );
        const int subColumns = std::max(
            1,
            static_cast<int>(std::ceil(static_cast<double>(n) / maxRowsPerColumn))
        );

        std::vector<int> colSizes(subColumns, 0);
        for (int idx = 0; idx < n; ++idx) {
            ++colSizes[idx % subColumns];
        }

        std::vector<int> rowCursor(subColumns, 0);
        for (int idx = 0; idx < n; ++idx) {
            const int node = nodes[idx];
            const int col = idx % subColumns;
            const int row = rowCursor[col]++;
            const double xOffset = (col - (subColumns - 1) / 2.0) * subColumnSpacing;
            const double colHeight = (colSizes[col] - 1) * rowSpacing;
            const double y = (row * rowSpacing) - (colHeight * 0.5);
            positions[node] = {l * layerSpacing + xOffset, y};
        }
    }
}

void GraphCanvas::refinePositionsHybrid(
    const std::vector<std::vector<int>>& neighbors,
    const std::vector<int>& layers,
    std::vector<Point>& positions
) const {
    const int V = static_cast<int>(positions.size());
    if (V <= 1) {
        return;
    }

    const double minSpacing = estimateMinimumNodeSpacing();
    const double repelBase = minSpacing * minSpacing * 2.30;
    const double springBase = 0.055;
    const double layerAnchor = 0.18;
    const double gravityY = 0.010;

    const double layerStep = minSpacing * 1.90;
    std::vector<double> anchorX(V, 0.0);
    for (int i = 0; i < V; ++i) {
        anchorX[i] = layers[i] * layerStep;
    }

    std::vector<std::pair<int, int>> edges;
    edges.reserve(V * 2);
    for (int u = 0; u < V; ++u) {
        for (int v : neighbors[u]) {
            if (u < v) {
                edges.emplace_back(u, v);
            }
        }
    }

    const int iterations = std::clamp(80 + V * 2, 110, 240);
    double temperature = minSpacing * 0.24;

    for (int it = 0; it < iterations; ++it) {
        std::vector<Point> force(V, {0.0, 0.0});

        for (int i = 0; i < V; ++i) {
            for (int j = i + 1; j < V; ++j) {
                double dx = positions[j].x - positions[i].x;
                double dy = positions[j].y - positions[i].y;
                double dist2 = dx * dx + dy * dy + 1e-6;
                const double dist = std::sqrt(dist2);
                if (dist > minSpacing * 7.5) {
                    continue;
                }
                const double repel = repelBase / dist2;
                const double ux = dx / dist;
                const double uy = dy / dist;
                const double fx = repel * ux;
                const double fy = repel * uy;
                force[i].x -= fx;
                force[i].y -= fy;
                force[j].x += fx;
                force[j].y += fy;
            }
        }

        for (const auto& e : edges) {
            const int u = e.first;
            const int v = e.second;
            const double dx = positions[v].x - positions[u].x;
            const double dy = positions[v].y - positions[u].y;
            const double dist = std::sqrt(dx * dx + dy * dy) + 1e-6;
            const double target = minSpacing * (1.00 + 0.22 * std::abs(layers[u] - layers[v]));
            const double stretch = (dist - target) * springBase;
            const double fx = stretch * (dx / dist);
            const double fy = stretch * (dy / dist);
            force[u].x += fx;
            force[u].y += fy;
            force[v].x -= fx;
            force[v].y -= fy;
        }

        for (int i = 0; i < V; ++i) {
            force[i].x += (anchorX[i] - positions[i].x) * layerAnchor;
            force[i].y += (-positions[i].y) * gravityY;
        }

        for (int i = 0; i < V; ++i) {
            double fx = force[i].x;
            double fy = force[i].y;
            const double norm = std::sqrt(fx * fx + fy * fy);
            if (norm > temperature) {
                const double scale = temperature / norm;
                fx *= scale;
                fy *= scale;
            }
            positions[i].x += fx;
            positions[i].y += fy;
        }

        temperature *= 0.986;
    }

    for (int pass = 0; pass < 10; ++pass) {
        for (int i = 0; i < V; ++i) {
            if (neighbors[i].empty()) {
                continue;
            }
            double avgY = 0.0;
            for (int nb : neighbors[i]) {
                avgY += positions[nb].y;
            }
            avgY /= static_cast<double>(neighbors[i].size());
            positions[i].y = positions[i].y * 0.88 + avgY * 0.12;
            positions[i].x = positions[i].x * 0.90 + anchorX[i] * 0.10;
        }
    }
}

void GraphCanvas::resolveNodeCollisions(std::vector<Point>& positions, double minDistance) {
    const int V = static_cast<int>(positions.size());
    if (V <= 1) {
        return;
    }

    const int maxPass = std::clamp(V / 2, 14, 30);
    const double target = std::max(18.0, minDistance);

    for (int pass = 0; pass < maxPass; ++pass) {
        bool moved = false;
        for (int i = 0; i < V; ++i) {
            for (int j = i + 1; j < V; ++j) {
                double dx = positions[j].x - positions[i].x;
                double dy = positions[j].y - positions[i].y;
                double dist = std::sqrt(dx * dx + dy * dy);
                if (dist >= target) {
                    continue;
                }

                moved = true;
                if (dist < 1e-6) {
                    const double angle = std::fmod((i * 97.0 + j * 31.0), 360.0) * (M_PI / 180.0);
                    dx = std::cos(angle);
                    dy = std::sin(angle);
                    dist = 1.0;
                }

                const double overlap = target - dist;
                const double ux = dx / dist;
                const double uy = dy / dist;
                const double push = overlap * 0.5;
                positions[i].x -= ux * push;
                positions[i].y -= uy * push;
                positions[j].x += ux * push;
                positions[j].y += uy * push;
            }
        }

        if (!moved) {
            break;
        }
    }
}

double GraphCanvas::estimateMinimumNodeSpacing() const {
    const double nodeRadius = currentNodeRadius();
    const double labelPadding = std::clamp(36.0 * std::sqrt(std::max(0.55, m_zoom)), 28.0, 56.0);
    return nodeRadius * 2.6 + labelPadding;
}

void GraphCanvas::normalizeAndStorePositions(
    std::vector<Point>& positions,
    const std::vector<int>& layers
) {
    if (positions.empty()) {
        m_pos.clear();
        return;
    }

    const int V = static_cast<int>(positions.size());
    int maxLayer = 0;
    for (int l : layers) {
        maxLayer = std::max(maxLayer, l);
    }

    std::vector<double> meanX(maxLayer + 1, 0.0);
    std::vector<int> count(maxLayer + 1, 0);
    for (int i = 0; i < V; ++i) {
        meanX[layers[i]] += positions[i].x;
        count[layers[i]] += 1;
    }
    for (int l = 0; l <= maxLayer; ++l) {
        if (count[l] > 0) {
            meanX[l] /= static_cast<double>(count[l]);
        }
    }

    std::vector<int> activeLayers;
    activeLayers.reserve(maxLayer + 1);
    for (int l = 0; l <= maxLayer; ++l) {
        if (count[l] > 0) {
            activeLayers.push_back(l);
        }
    }

    const double minLayerGap = estimateMinimumNodeSpacing() * 1.55;
    for (std::size_t idx = 1; idx < activeLayers.size(); ++idx) {
        const int prev = activeLayers[idx - 1];
        const int cur = activeLayers[idx];
        const double gap = meanX[cur] - meanX[prev];
        if (gap >= minLayerGap) {
            continue;
        }

        const double delta = minLayerGap - gap;
        for (int i = 0; i < V; ++i) {
            if (layers[i] >= cur) {
                positions[i].x += delta;
            }
        }
        for (std::size_t k = idx; k < activeLayers.size(); ++k) {
            meanX[activeLayers[k]] += delta;
        }
    }

    Point center{0.0, 0.0};
    for (const auto& p : positions) {
        center.x += p.x;
        center.y += p.y;
    }
    center.x /= static_cast<double>(V);
    center.y /= static_cast<double>(V);

    for (auto& p : positions) {
        p.x -= center.x;
        p.y -= center.y;
    }

    m_pos = positions;
}

void GraphCanvas::calculateLayout() {
    const int V = m_graph.getVertexCount();
    if (V == 0) {
        m_pos.clear();
        return;
    }

    std::vector<std::vector<int>> neighbors;
    buildNeighborGraph(neighbors);

    std::vector<int> layers(V, 0);
    int maxLayer = 0;
    assignPrimaryLayers(neighbors, layers, maxLayer);
    (void)maxLayer;

    std::vector<std::vector<int>> layerNodes;
    orderNodesWithinLayers(neighbors, layers, layerNodes);

    std::vector<Point> positions;
    initializeLayerPositions(layerNodes, positions);
    refinePositionsHybrid(neighbors, layers, positions);
    resolveNodeCollisions(positions, estimateMinimumNodeSpacing());
    normalizeAndStorePositions(positions, layers);
    resolveNodeCollisions(m_pos, estimateMinimumNodeSpacing() * 0.96);
}

void GraphCanvas::cacheSceneBounds() {
    if (m_pos.empty()) {
        m_sceneCenter = {0.0, 0.0};
        m_sceneWidth = 240.0;
        m_sceneHeight = 240.0;
        return;
    }

    double minX = m_pos.front().x;
    double maxX = m_pos.front().x;
    double minY = m_pos.front().y;
    double maxY = m_pos.front().y;

    for (const auto& p : m_pos) {
        minX = std::min(minX, p.x);
        maxX = std::max(maxX, p.x);
        minY = std::min(minY, p.y);
        maxY = std::max(maxY, p.y);
    }

    m_sceneCenter = {(minX + maxX) / 2.0, (minY + maxY) / 2.0};
    m_sceneWidth = std::max(240.0, maxX - minX + 160.0);
    m_sceneHeight = std::max(240.0, maxY - minY + 160.0);
}

double GraphCanvas::computeFitScale(int width, int height) const {
    const double usableW = std::max(200.0, width - 220.0);
    const double usableH = std::max(200.0, height - 180.0);
    const double scaleX = usableW / std::max(1.0, m_sceneWidth);
    const double scaleY = usableH / std::max(1.0, m_sceneHeight);
    return std::clamp(std::min(scaleX, scaleY), 0.20, 2.20);
}

Point GraphCanvas::worldToScreen(const Point& world, int width, int height) const {
    const double scale = computeFitScale(width, height) * m_zoom;
    return {
        (world.x - m_sceneCenter.x) * scale + (width / 2.0) + m_pan.x,
        (world.y - m_sceneCenter.y) * scale + (height / 2.0) + m_pan.y
    };
}

Point GraphCanvas::screenToWorld(double x, double y, int width, int height) const {
    const double scale = computeFitScale(width, height) * m_zoom;
    if (scale <= 0.0) {
        return m_sceneCenter;
    }
    return {
        ((x - (width / 2.0) - m_pan.x) / scale) + m_sceneCenter.x,
        ((y - (height / 2.0) - m_pan.y) / scale) + m_sceneCenter.y
    };
}

int GraphCanvas::hitTestNode(double screenX, double screenY) const {
    if (!m_hasData || m_pos.empty()) {
        return -1;
    }

    const auto allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();
    const double hitRadius = currentNodeRadius() + 8.0;

    int bestNode = -1;
    double bestDistance = 1e18;
    for (int i = 0; i < static_cast<int>(m_pos.size()); ++i) {
        const Point screen = worldToScreen(m_pos[i], width, height);
        const double d = distance(screen, {screenX, screenY});
        if (d <= hitRadius && d < bestDistance) {
            bestDistance = d;
            bestNode = i;
        }
    }
    return bestNode;
}

void GraphCanvas::updateHover(double x, double y) {
    const int hovered = hitTestNode(x, y);
    const bool changed = (hovered != m_hoveredNode) ||
                         std::abs(x - m_hoverScreen.x) > 1.0 ||
                         std::abs(y - m_hoverScreen.y) > 1.0;
    m_hoveredNode = hovered;
    m_hoverScreen = {x, y};
    if (changed) {
        queue_draw();
    }
}

bool GraphCanvas::on_button_press_event(GdkEventButton* event) {
    if (!m_hasData) {
        return false;
    }

    m_pressScreen = {event->x, event->y};
    m_lastPointerScreen = m_pressScreen;
    m_pointerMovedSincePress = false;

    if (event->button == 1) {
        const int hitNode = hitTestNode(event->x, event->y);
        if (hitNode >= 0) {
            m_clickCandidateNode = hitNode;
            m_draggingNode = -1;
        } else {
            m_clickCandidateNode = -1;
            m_isPanning = true;
        }
        updateHover(event->x, event->y);
        return true;
    }

    if (event->button == 2) {
        m_isPanning = true;
        return true;
    }

    if (event->button == 3) {
        const int hitNode = hitTestNode(event->x, event->y);
        if (hitNode >= 0) {
            m_signalEndVertexSelected.emit(hitNode);
            updateHover(event->x, event->y);
            return true;
        }
    }

    return false;
}

bool GraphCanvas::on_button_release_event(GdkEventButton* event) {
    if (!m_hasData) {
        return false;
    }

    const int candidateNode = m_clickCandidateNode;
    const bool hadInteraction =
        (m_draggingNode >= 0) || m_isPanning || (candidateNode >= 0);

    if (event->button == 1) {
        if (!m_pointerMovedSincePress && candidateNode >= 0) {
            m_signalStartVertexSelected.emit(candidateNode);
        }
        m_draggingNode = -1;
        m_clickCandidateNode = -1;
        m_isPanning = false;
        m_pointerMovedSincePress = false;
        updateHover(event->x, event->y);
        return hadInteraction;
    }

    if (event->button == 2) {
        m_isPanning = false;
        m_pointerMovedSincePress = false;
        updateHover(event->x, event->y);
        return true;
    }

    return false;
}

bool GraphCanvas::on_motion_notify_event(GdkEventMotion* event) {
    if (!m_hasData) {
        return false;
    }

    Point current{event->x, event->y};
    const bool crossedThreshold = distance(current, m_pressScreen) > 4.0;

    if ((m_clickCandidateNode >= 0 || m_draggingNode >= 0) && crossedThreshold) {
        m_pointerMovedSincePress = true;
        if (m_draggingNode < 0) {
            m_draggingNode = m_clickCandidateNode;
        }
    }

    if (m_draggingNode >= 0) {
        const auto allocation = get_allocation();
        m_pos[m_draggingNode] = screenToWorld(
            current.x,
            current.y,
            allocation.get_width(),
            allocation.get_height()
        );
        m_lastPointerScreen = current;
        updateHover(current.x, current.y);
        return true;
    }

    if (m_isPanning) {
        m_pointerMovedSincePress = true;
        m_pan.x += current.x - m_lastPointerScreen.x;
        m_pan.y += current.y - m_lastPointerScreen.y;
        m_lastPointerScreen = current;
        updateHover(current.x, current.y);
        return true;
    }

    m_lastPointerScreen = current;
    updateHover(current.x, current.y);
    return true;
}

bool GraphCanvas::on_scroll_event(GdkEventScroll* event) {
    if (!m_hasData) {
        return false;
    }

    double zoomFactor = 1.0;
    if (event->direction == GDK_SCROLL_UP) {
        zoomFactor = 1.12;
    } else if (event->direction == GDK_SCROLL_DOWN) {
        zoomFactor = 1.0 / 1.12;
    } else if (event->direction == GDK_SCROLL_SMOOTH) {
        zoomFactor = (event->delta_y < 0.0) ? 1.08 : (1.0 / 1.08);
    } else {
        return false;
    }

    const auto allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();
    const Point anchorWorld = screenToWorld(event->x, event->y, width, height);

    m_zoom = std::clamp(m_zoom * zoomFactor, 0.55, 4.25);
    const double scale = computeFitScale(width, height) * m_zoom;

    m_pan.x = event->x - (width / 2.0) - ((anchorWorld.x - m_sceneCenter.x) * scale);
    m_pan.y = event->y - (height / 2.0) - ((anchorWorld.y - m_sceneCenter.y) * scale);

    updateHover(event->x, event->y);
    return true;
}

bool GraphCanvas::on_leave_notify_event(GdkEventCrossing* /*event*/) {
    m_hoveredNode = -1;
    queue_draw();
    return true;
}

double GraphCanvas::computeEdgeCurvature(
    Point from,
    Point to,
    int parallelIndex,
    int parallelCount,
    bool reverseDirection,
    bool bidirectional
) const {
    const double len = std::max(1.0, distance(from, to));
    if (len < 1.5) {
        const double baseLoop = currentNodeRadius() * 1.25 + 14.0;
        const double rank = static_cast<double>(parallelIndex) - (parallelCount - 1) * 0.5;
        const double sign = (rank >= 0.0) ? 1.0 : -1.0;
        return sign * (baseLoop + std::abs(rank) * 16.0);
    }

    const double dynamicBase = std::clamp(len * 0.085, 16.0, 96.0);
    double rank = 0.0;

    if (parallelCount <= 1) {
        if (!bidirectional) {
            return 0.0;
        }
        rank = reverseDirection ? -0.65 : 0.65;
    } else {
        rank = static_cast<double>(parallelIndex) - (parallelCount - 1) * 0.5;
        if (parallelCount % 2 == 1 && std::abs(rank) < 1e-9 && bidirectional) {
            rank = reverseDirection ? -0.45 : 0.45;
        } else if (reverseDirection) {
            rank = -rank;
        }

        if (std::abs(rank) < 1e-9 && !bidirectional) {
            return 0.0;
        }
    }

    double magnitude = dynamicBase * (0.65 + std::abs(rank) * 0.90);
    if (bidirectional) {
        magnitude *= 1.12;
    }

    const double sign = (rank >= 0.0) ? 1.0 : -1.0;
    const double maxCurve = len * 0.42;
    return std::clamp(sign * magnitude, -maxCurve, maxCurve);
}

Point GraphCanvas::computeCurvedLabelPosition(
    Point from,
    Point to,
    double curveAmount,
    int parallelIndex,
    int parallelCount,
    bool selfLoop
) const {
    if (selfLoop) {
        const double side = (curveAmount >= 0.0) ? 1.0 : -1.0;
        const double loopRadius = currentNodeRadius() * 0.95 + std::max(12.0, std::abs(curveAmount) * 0.35);
        Point center{
            from.x + side * (currentNodeRadius() + loopRadius * 0.65),
            from.y - (currentNodeRadius() + loopRadius * 1.40 + std::abs(curveAmount) * 0.08)
        };
        return {
            center.x + side * (loopRadius * 0.85),
            center.y - loopRadius * 0.55
        };
    }

    const double dx = to.x - from.x;
    const double dy = to.y - from.y;
    const double len = std::max(1.0, std::sqrt(dx * dx + dy * dy));
    const double tx = dx / len;
    const double ty = dy / len;
    const double nx = -ty;
    const double ny = tx;

    const double rank = (parallelCount > 1)
        ? (static_cast<double>(parallelIndex) - (parallelCount - 1) * 0.5)
        : 0.0;

    if (std::abs(curveAmount) <= 1.0) {
        const double offset = 13.0 + std::abs(rank) * 7.0;
        return {
            (from.x + to.x) * 0.5 + nx * offset,
            (from.y + to.y) * 0.5 + ny * offset
        };
    }

    Point control{
        (from.x + to.x) * 0.5 + nx * curveAmount,
        (from.y + to.y) * 0.5 + ny * curveAmount
    };

    const double t = 0.5;
    Point label{
        (1 - t) * (1 - t) * from.x + 2 * (1 - t) * t * control.x + t * t * to.x,
        (1 - t) * (1 - t) * from.y + 2 * (1 - t) * t * control.y + t * t * to.y
    };

    double side = (curveAmount >= 0.0) ? 1.0 : -1.0;
    if (parallelCount > 1 && std::abs(rank) > 1e-6) {
        side = (rank >= 0.0) ? 1.0 : -1.0;
    }

    const double normalOffset = 10.0 + std::min(32.0, std::abs(curveAmount) * 0.24) + std::abs(rank) * 5.0;
    label.x += nx * normalOffset * side + tx * rank * 4.0;
    label.y += ny * normalOffset * side + ty * rank * 4.0;

    const double minNodeGap = currentNodeRadius() + 17.0;
    auto pushAwayNode = [&](const Point& center) {
        double lx = label.x - center.x;
        double ly = label.y - center.y;
        double dist = std::sqrt(lx * lx + ly * ly);
        if (dist >= minNodeGap) {
            return;
        }
        if (dist < 1e-6) {
            lx = nx;
            ly = ny;
            dist = 1.0;
        }
        const double push = minNodeGap - dist;
        label.x += (lx / dist) * push;
        label.y += (ly / dist) * push;
    };

    pushAwayNode(from);
    pushAwayNode(to);
    return label;
}

void GraphCanvas::drawArrow(
    const Cairo::RefPtr<Cairo::Context>& cr,
    Point from,
    Point to,
    double nodeRadius,
    bool curved,
    double curveAmount
) {
    if (distance(from, to) < 1.0) {
        const double side = (curveAmount >= 0.0) ? 1.0 : -1.0;
        const double loopRadius = nodeRadius * 0.95 + std::max(12.0, std::abs(curveAmount) * 0.35);
        const Point center{
            from.x + side * (nodeRadius + loopRadius * 0.65),
            from.y - (nodeRadius + loopRadius * 1.40 + std::abs(curveAmount) * 0.08)
        };

        const double startAngle = (side > 0.0) ? (0.65 * M_PI) : (0.35 * M_PI);
        const double endAngle = startAngle + (1.75 * M_PI);

        cr->begin_new_path();
        cr->arc(center.x, center.y, loopRadius, startAngle, endAngle);
        cr->stroke();

        if (m_isDirected) {
            const double tipX = center.x + loopRadius * std::cos(endAngle);
            const double tipY = center.y + loopRadius * std::sin(endAngle);
            const double tangent = endAngle + M_PI / 2.0;
            const double arrowLength = 12.0;
            const double arrowAngle = M_PI / 7.0;

            cr->begin_new_path();
            cr->move_to(tipX, tipY);
            cr->line_to(
                tipX - arrowLength * std::cos(tangent - arrowAngle),
                tipY - arrowLength * std::sin(tangent - arrowAngle)
            );
            cr->line_to(
                tipX - arrowLength * std::cos(tangent + arrowAngle),
                tipY - arrowLength * std::sin(tangent + arrowAngle)
            );
            cr->close_path();
            cr->fill();
        }
        return;
    }

    const double angle = std::atan2(to.y - from.y, to.x - from.x);
    const double sourceX = from.x + nodeRadius * std::cos(angle);
    const double sourceY = from.y + nodeRadius * std::sin(angle);
    const double targetX = to.x - nodeRadius * std::cos(angle);
    const double targetY = to.y - nodeRadius * std::sin(angle);

    double headAngle = angle;

    if (curved) {
        cr->begin_new_path();
        const double perpX = -std::sin(angle);
        const double perpY = std::cos(angle);
        const double cX = ((sourceX + targetX) / 2.0) + perpX * curveAmount;
        const double cY = ((sourceY + targetY) / 2.0) + perpY * curveAmount;

        const double px1 = sourceX + (2.0 / 3.0) * (cX - sourceX);
        const double py1 = sourceY + (2.0 / 3.0) * (cY - sourceY);
        const double px2 = targetX + (2.0 / 3.0) * (cX - targetX);
        const double py2 = targetY + (2.0 / 3.0) * (cY - targetY);

        cr->move_to(sourceX, sourceY);
        cr->curve_to(px1, py1, px2, py2, targetX, targetY);
        cr->stroke();

        headAngle = std::atan2(targetY - py2, targetX - px2);
    } else {
        cr->begin_new_path();
        cr->move_to(sourceX, sourceY);
        cr->line_to(targetX, targetY);
        cr->stroke();
    }

    if (m_isDirected) {
        const double arrowLength = 12.0;
        const double arrowAngle = M_PI / 7.0;

        cr->begin_new_path();
        cr->move_to(targetX, targetY);
        cr->line_to(
            targetX - arrowLength * std::cos(headAngle - arrowAngle),
            targetY - arrowLength * std::sin(headAngle - arrowAngle)
        );
        cr->line_to(
            targetX - arrowLength * std::cos(headAngle + arrowAngle),
            targetY - arrowLength * std::sin(headAngle + arrowAngle)
        );
        cr->close_path();
        cr->fill();
    }
}

void GraphCanvas::drawWeightLabel(
    const Cairo::RefPtr<Cairo::Context>& cr,
    Point position,
    const std::string& text
) {
    cr->save();
    cr->select_font_face("Segoe UI", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_BOLD);
    cr->set_font_size(12.0);

    Cairo::TextExtents te;
    cr->get_text_extents(text, te);

    const double padX = 7.0;
    const double padY = 4.0;
    const double boxW = te.width + (padX * 2.0);
    const double boxH = te.height + (padY * 2.0);
    const double x = position.x - (boxW / 2.0);
    const double y = position.y - (boxH / 2.0);

    cr->set_source_rgba(0.06, 0.09, 0.14, 0.90);
    cr->rectangle(x, y, boxW, boxH);
    cr->fill_preserve();
    cr->set_source_rgba(1.0, 1.0, 1.0, 0.15);
    cr->set_line_width(1.0);
    cr->stroke();

    cr->set_source_rgb(1.0, 1.0, 1.0);
    cr->move_to(position.x - (te.width / 2.0) - te.x_bearing, position.y - te.y_bearing / 2.0);
    cr->show_text(text);
    cr->restore();
}

std::string GraphCanvas::formatDistance(int value) const {
    if (value >= 1000000000) {
        return "INF";
    }
    return std::to_string(value);
}

void GraphCanvas::drawLegend(const Cairo::RefPtr<Cairo::Context>& cr) {
    const double x = 18.0;
    const double y = 18.0;
    const double w = 235.0;
    const double h = 210.0;

    cr->save();
    cr->set_source_rgba(15 / 255.0, 23 / 255.0, 42 / 255.0, 0.92);
    cr->rectangle(x, y, w, h);
    cr->fill_preserve();
    cr->set_source_rgba(148 / 255.0, 163 / 255.0, 184 / 255.0, 0.55);
    cr->set_line_width(1.0);
    cr->stroke();

    cr->select_font_face("Segoe UI", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_BOLD);
    cr->set_font_size(13.0);
    cr->set_source_rgb(0.96, 0.98, 1.0);
    cr->move_to(x + 14.0, y + 22.0);
    cr->show_text("Chu giai & thao tac");

    auto drawItem = [&](double rowY, const double* color, const std::string& text, bool line) {
        if (line) {
            cr->set_source_rgb(color[0], color[1], color[2]);
            cr->set_line_width(3.0);
            cr->move_to(x + 14.0, rowY);
            cr->line_to(x + 34.0, rowY);
            cr->stroke();
        } else {
            cr->set_source_rgb(color[0], color[1], color[2]);
            cr->arc(x + 24.0, rowY, 7.0, 0.0, 2.0 * M_PI);
            cr->fill();
        }
        cr->set_source_rgb(0.88, 0.92, 0.97);
        cr->select_font_face("Segoe UI", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_NORMAL);
        cr->set_font_size(11.0);
        cr->move_to(x + 42.0, rowY + 4.0);
        cr->show_text(text);
    };

    drawItem(y + 44.0, C_NODE_START, "Start hien tai", false);
    drawItem(y + 64.0, C_NODE_END, "Dich hien tai", false);
    drawItem(y + 84.0, C_NODE_VISIT, "Dang xet (Visit)", false);
    drawItem(y + 104.0, C_NODE_DONE, "Da xu ly xong", false);
    drawItem(y + 124.0, C_NODE_DEFAULT, "Chua xet", false);
    drawItem(y + 144.0, C_EDGE_RELAX, "Dang noi long (Relax)", true);
    drawItem(y + 164.0, C_EDGE_PATH, "Duong di ngan nhat", true);

    cr->set_source_rgb(0.72, 0.79, 0.88);
    cr->set_font_size(10.5);
    cr->move_to(x + 14.0, y + 188.0);
    cr->show_text("Left click: start | Right click: dich");
    cr->move_to(x + 14.0, y + 204.0);
    cr->show_text("Keo dinh / keo nen / scroll de zoom");
    cr->restore();
}

void GraphCanvas::drawDistanceTable(const Cairo::RefPtr<Cairo::Context>& cr, int width) {
    const int V = m_graph.getVertexCount();
    if (V == 0) {
        return;
    }

    std::vector<int> shown;
    if (V <= 10) {
        for (int i = 0; i < V; ++i) {
            shown.push_back(i);
        }
    } else {
        for (int i = 0; i < 5; ++i) {
            shown.push_back(i);
        }
        shown.push_back(-1);
        for (int i = std::max(5, V - 4); i < V; ++i) {
            shown.push_back(i);
        }
    }

    const double rowHeight = 18.0;
    const double panelW = 210.0;
    const double panelH = 44.0 + (shown.size() * rowHeight);
    const double x = width - panelW - 18.0;
    const double y = 18.0;

    cr->save();
    cr->set_source_rgba(15 / 255.0, 23 / 255.0, 42 / 255.0, 0.92);
    cr->rectangle(x, y, panelW, panelH);
    cr->fill_preserve();
    cr->set_source_rgba(148 / 255.0, 163 / 255.0, 184 / 255.0, 0.55);
    cr->set_line_width(1.0);
    cr->stroke();

    cr->select_font_face("Segoe UI", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_BOLD);
    cr->set_font_size(13.0);
    cr->set_source_rgb(0.96, 0.98, 1.0);
    cr->move_to(x + 14.0, y + 22.0);
    cr->show_text("Bang khoang cach d[v]");

    double rowY = y + 42.0;
    cr->select_font_face("Consolas", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_NORMAL);
    cr->set_font_size(11.0);
    for (int id : shown) {
        cr->set_source_rgb(0.84, 0.90, 0.96);
        if (id == -1) {
            cr->move_to(x + 14.0, rowY);
            cr->show_text("...");
        } else {
            std::ostringstream line;
            const auto it = m_distances.find(id);
            line << "[" << m_graph.getVertexLabel(id) << "]  d=";
            line << formatDistance(it == m_distances.end() ? 1000000000 : it->second);
            cr->move_to(x + 14.0, rowY);
            cr->show_text(line.str());
        }
        rowY += rowHeight;
    }
    cr->restore();
}

void GraphCanvas::drawProgressBar(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height) {
    const int totalSteps = static_cast<int>(m_result.traceSteps.size());
    const int shownStep = std::min(m_currentStep, totalSteps);
    const double progress = (totalSteps == 0) ? 1.0 : static_cast<double>(shownStep) / totalSteps;

    const double barW = std::min(420.0, width - 220.0);
    const double barH = 12.0;
    const double x = (width - barW) / 2.0;
    const double y = height - 54.0;

    std::string desc = "San sang";
    if (shownStep > 0 && shownStep <= totalSteps) {
        desc = m_result.traceSteps[shownStep - 1].desc;
    } else if (totalSteps > 0 && shownStep >= totalSteps) {
        desc = "Hoan tat animation";
    }

    cr->save();
    cr->select_font_face("Segoe UI", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_BOLD);
    cr->set_font_size(12.0);
    cr->set_source_rgb(0.94, 0.96, 0.99);
    cr->move_to(x, y - 12.0);
    cr->show_text("Step " + std::to_string(shownStep) + " / " + std::to_string(totalSteps) + "  |  " + desc);

    cr->set_source_rgba(148 / 255.0, 163 / 255.0, 184 / 255.0, 0.35);
    cr->rectangle(x, y, barW, barH);
    cr->fill();

    cr->set_source_rgb(41 / 255.0, 121 / 255.0, 1.0);
    cr->rectangle(x, y, barW * progress, barH);
    cr->fill();

    cr->set_source_rgba(1.0, 1.0, 1.0, 0.12);
    cr->set_line_width(1.0);
    cr->rectangle(x, y, barW, barH);
    cr->stroke();
    cr->restore();
}

void GraphCanvas::drawTooltip(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height) {
    if (m_hoveredNode < 0 || m_hoveredNode >= m_graph.getVertexCount()) {
        return;
    }

    std::vector<std::string> lines;
    lines.push_back("Dinh " + m_graph.getVertexLabel(m_hoveredNode));
    const auto it = m_distances.find(m_hoveredNode);
    lines.push_back("d = " + formatDistance(it == m_distances.end() ? 1000000000 : it->second));
    lines.push_back("Left click: set start | Right click: set dich");

    cr->save();
    cr->select_font_face("Segoe UI", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_NORMAL);
    cr->set_font_size(11.5);

    double maxWidth = 0.0;
    std::vector<Cairo::TextExtents> extents(lines.size());
    for (std::size_t i = 0; i < lines.size(); ++i) {
        cr->get_text_extents(lines[i], extents[i]);
        maxWidth = std::max(maxWidth, extents[i].width);
    }

    const double padding = 10.0;
    const double lineHeight = 16.0;
    const double boxW = maxWidth + padding * 2.0;
    const double boxH = padding * 2.0 + lineHeight * lines.size();

    double x = m_hoverScreen.x + 16.0;
    double y = m_hoverScreen.y + 16.0;
    if (x + boxW > width - 12.0) {
        x = width - boxW - 12.0;
    }
    if (y + boxH > height - 12.0) {
        y = height - boxH - 12.0;
    }

    cr->set_source_rgba(2 / 255.0, 6 / 255.0, 23 / 255.0, 0.95);
    cr->rectangle(x, y, boxW, boxH);
    cr->fill_preserve();
    cr->set_source_rgba(1.0, 1.0, 1.0, 0.18);
    cr->set_line_width(1.0);
    cr->stroke();

    for (std::size_t i = 0; i < lines.size(); ++i) {
        if (i == 0) {
            cr->select_font_face("Segoe UI", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_BOLD);
            cr->set_source_rgb(1.0, 1.0, 1.0);
        } else if (i == 1) {
            cr->select_font_face("Consolas", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_NORMAL);
            cr->set_source_rgb(0.69, 0.86, 1.0);
        } else {
            cr->select_font_face("Segoe UI", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_NORMAL);
            cr->set_source_rgb(0.84, 0.90, 0.96);
        }
        cr->move_to(x + padding, y + padding + 12.0 + (i * lineHeight));
        cr->show_text(lines[i]);
    }
    cr->restore();
}

bool GraphCanvas::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
    const auto allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    cr->set_source_rgb(248 / 255.0, 250 / 255.0, 252 / 255.0);
    cr->paint();

    if (!m_hasData) {
        return false;
    }

    const int V = m_graph.getVertexCount();
    if (V == 0) {
        cr->set_source_rgb(0.29, 0.33, 0.40);
        cr->select_font_face("Segoe UI", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_BOLD);
        cr->set_font_size(18.0);
        cr->move_to(40.0, 60.0);
        cr->show_text("Canvas chua co du lieu do thi.");
        return true;
    }

    const double nodeRadius = currentNodeRadius();
    const bool done = (m_currentStep >= static_cast<int>(m_result.traceSteps.size()));
    const auto& adj = m_graph.getAdjacencyList();

    cr->set_line_cap(Cairo::LINE_CAP_ROUND);
    cr->set_line_join(Cairo::LINE_JOIN_ROUND);

    std::vector<Point> screenPos(V);
    for (int i = 0; i < V; ++i) {
        screenPos[i] = worldToScreen(m_pos[i], width, height);
    }

    struct EdgeRenderInfo {
        int from = -1;
        int to = -1;
        int weight = 0;
        int pairA = -1;
        int pairB = -1;
        int parallelIndex = 0;
        int parallelCount = 1;
        bool bidirectional = false;
        bool selfLoop = false;
    };

    std::vector<EdgeRenderInfo> edgeInfos;
    edgeInfos.reserve(V * 3);
    std::unordered_map<long long, std::vector<int>> pairBuckets;

    for (int i = 0; i < V; ++i) {
        for (const auto& edge : adj[i]) {
            const int j = edge.destination;
            if (!m_isDirected && i > j) {
                continue;
            }

            EdgeRenderInfo info;
            info.from = i;
            info.to = j;
            info.weight = edge.weight;
            info.pairA = std::min(i, j);
            info.pairB = std::max(i, j);
            info.selfLoop = (i == j);

            const long long pairId =
                (static_cast<long long>(info.pairA) << 32) ^
                static_cast<unsigned int>(info.pairB);
            pairBuckets[pairId].push_back(static_cast<int>(edgeInfos.size()));
            edgeInfos.push_back(info);
        }
    }

    for (auto& item : pairBuckets) {
        auto& bucket = item.second;
        if (bucket.empty()) {
            continue;
        }

        EdgeRenderInfo& sample = edgeInfos[bucket.front()];
        if (sample.selfLoop) {
            const int total = static_cast<int>(bucket.size());
            for (int idx = 0; idx < total; ++idx) {
                edgeInfos[bucket[idx]].parallelIndex = idx;
                edgeInfos[bucket[idx]].parallelCount = total;
                edgeInfos[bucket[idx]].bidirectional = false;
            }
            continue;
        }

        if (m_isDirected) {
            std::vector<int> forward;
            std::vector<int> backward;
            forward.reserve(bucket.size());
            backward.reserve(bucket.size());

            for (int edgeIdx : bucket) {
                const auto& e = edgeInfos[edgeIdx];
                if (e.from == e.pairA && e.to == e.pairB) {
                    forward.push_back(edgeIdx);
                } else {
                    backward.push_back(edgeIdx);
                }
            }

            const bool hasBothDirections = !forward.empty() && !backward.empty();
            for (int idx = 0; idx < static_cast<int>(forward.size()); ++idx) {
                edgeInfos[forward[idx]].parallelIndex = idx;
                edgeInfos[forward[idx]].parallelCount = static_cast<int>(forward.size());
                edgeInfos[forward[idx]].bidirectional = hasBothDirections;
            }
            for (int idx = 0; idx < static_cast<int>(backward.size()); ++idx) {
                edgeInfos[backward[idx]].parallelIndex = idx;
                edgeInfos[backward[idx]].parallelCount = static_cast<int>(backward.size());
                edgeInfos[backward[idx]].bidirectional = hasBothDirections;
            }
        } else {
            const int total = static_cast<int>(bucket.size());
            for (int idx = 0; idx < total; ++idx) {
                edgeInfos[bucket[idx]].parallelIndex = idx;
                edgeInfos[bucket[idx]].parallelCount = total;
                edgeInfos[bucket[idx]].bidirectional = false;
            }
        }
    }

    for (const auto& info : edgeInfos) {
        const int i = info.from;
        const int j = info.to;
        const long long uid = edgeKey(i, j);
        const bool isPath = done && (m_pathEdges.find(uid) != m_pathEdges.end());
        const bool isRelaxing = (!done && m_relaxingEdge.first == i && m_relaxingEdge.second == j);

        if (isPath) {
            cr->set_source_rgb(C_EDGE_PATH[0], C_EDGE_PATH[1], C_EDGE_PATH[2]);
            cr->set_line_width(3.8);
        } else if (isRelaxing) {
            cr->set_source_rgb(C_EDGE_RELAX[0], C_EDGE_RELAX[1], C_EDGE_RELAX[2]);
            cr->set_line_width(3.0);
        } else {
            cr->set_source_rgb(C_EDGE_DEFAULT[0], C_EDGE_DEFAULT[1], C_EDGE_DEFAULT[2]);
            cr->set_line_width(1.6);
        }

        const bool reverseDirection = (info.from != info.pairA);
        const double curveAmount = computeEdgeCurvature(
            screenPos[i],
            screenPos[j],
            info.parallelIndex,
            info.parallelCount,
            reverseDirection,
            info.bidirectional
        );

        const bool isCurved = info.selfLoop || std::abs(curveAmount) > 1.0;
        drawArrow(cr, screenPos[i], screenPos[j], nodeRadius, isCurved, curveAmount);

        const Point labelPos = computeCurvedLabelPosition(
            screenPos[i],
            screenPos[j],
            curveAmount,
            info.parallelIndex,
            info.parallelCount,
            info.selfLoop
        );
        drawWeightLabel(cr, labelPos, std::to_string(info.weight));
    }

    for (int i = 0; i < V; ++i) {
        bool isStart = (i == m_result.startVertex);
        bool isEnd = !m_result.shortestPath.empty() && i == m_result.shortestPath.back();
        const bool isPathNode =
            done &&
            std::find(m_result.shortestPath.begin(), m_result.shortestPath.end(), i) != m_result.shortestPath.end();

        const double* color = C_NODE_DEFAULT;
        if (done) {
            if (isPathNode) {
                color = C_NODE_PATH;
            }
            if (isStart) {
                color = C_NODE_START;
            }
            if (isEnd) {
                color = C_NODE_END;
            }
        } else {
            if (i == m_visitingNode) {
                color = C_NODE_VISIT;
            } else if (m_visited.find(i) != m_visited.end()) {
                color = isStart ? C_NODE_START : C_NODE_DONE;
            } else if (isStart) {
                color = C_NODE_START;
            }
        }

        if (i == m_hoveredNode) {
            cr->set_source_rgba(C_NODE_HOVER[0], C_NODE_HOVER[1], C_NODE_HOVER[2], 0.28);
            cr->begin_new_path();
            cr->arc(screenPos[i].x, screenPos[i].y, nodeRadius + 7.0, 0.0, 2.0 * M_PI);
            cr->fill();
        }

        cr->begin_new_path();
        cr->arc(screenPos[i].x, screenPos[i].y, nodeRadius, 0.0, 2.0 * M_PI);
        cr->set_source_rgb(color[0], color[1], color[2]);
        cr->fill_preserve();
        cr->set_source_rgb(0.05, 0.07, 0.09);
        cr->set_line_width(i == m_hoveredNode ? 2.5 : 1.8);
        cr->stroke();

        cr->select_font_face("Segoe UI", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_BOLD);
        cr->set_source_rgb(0.05, 0.05, 0.05);
        cr->set_font_size(13.0);

        const std::string nameText = m_graph.getVertexLabel(i);
        Cairo::TextExtents teName;
        cr->get_text_extents(nameText, teName);
        cr->move_to(
            screenPos[i].x - (teName.width / 2.0) - teName.x_bearing,
            screenPos[i].y - 2.0
        );
        cr->show_text(nameText);

        cr->select_font_face("Consolas", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_NORMAL);
        cr->set_font_size(10.5);
        const auto it = m_distances.find(i);
        const std::string distText = "d=" + formatDistance(it == m_distances.end() ? 1000000000 : it->second);
        Cairo::TextExtents teDist;
        cr->get_text_extents(distText, teDist);
        cr->move_to(
            screenPos[i].x - (teDist.width / 2.0) - teDist.x_bearing,
            screenPos[i].y + 14.0
        );
        cr->show_text(distText);
    }

    drawLegend(cr);
    drawDistanceTable(cr, width);
    drawProgressBar(cr, width, height);
    drawTooltip(cr, width, height);
    return true;
}
