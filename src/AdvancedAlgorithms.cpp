#include "../lib/AdvancedAlgorithms.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <queue>
#include <sstream>

namespace {

using Distance = AdaptiveBlockList::Distance;

bool lessKeyValue(const AdaptiveBlockList::KeyValue& lhs, const AdaptiveBlockList::KeyValue& rhs) {
    if (lhs.value != rhs.value) {
        return lhs.value < rhs.value;
    }
    return lhs.key < rhs.key;
}

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
//  AdaptiveBlockList
// ─────────────────────────────────────────────────────────────────────────────
AdaptiveBlockList::AdaptiveBlockList()
    : expectedInsertions_(0), blockSize_(1), upperBound_(0), totalSize_(0), nextBlockId_(1) {
    reset(0, 1, 0);
}

AdaptiveBlockList::AdaptiveBlockList(std::size_t expectedInsertions, std::size_t blockSize, Distance upperBound)
    : expectedInsertions_(0), blockSize_(1), upperBound_(0), totalSize_(0), nextBlockId_(1) {
    reset(expectedInsertions, blockSize, upperBound);
}

void AdaptiveBlockList::reset(std::size_t expectedInsertions, std::size_t blockSize, Distance upperBound) {
    expectedInsertions_ = expectedInsertions;
    blockSize_ = std::max<std::size_t>(1, blockSize);
    upperBound_ = upperBound;
    totalSize_ = 0;
    nextBlockId_ = 1;

    prependedBlocks_.clear();
    insertedBlocks_.clear();
    prependedPositions_.clear();
    insertedPositions_.clear();
    insertedUpperBounds_.clear();
    keyIndex_.clear();

    ensureInsertTail();
}

bool AdaptiveBlockList::empty() const {
    return totalSize_ == 0;
}

std::size_t AdaptiveBlockList::size() const {
    return totalSize_;
}

std::size_t AdaptiveBlockList::blockSize() const {
    return blockSize_;
}

void AdaptiveBlockList::ensureInsertTail() {
    if (!insertedBlocks_.empty()) {
        return;
    }

    auto block = std::make_shared<Block>();
    block->id = nextBlockId_++;
    block->prepended = false;
    block->upperBound = upperBound_;
    insertedBlocks_.push_back(block);
    auto position = insertedBlocks_.end();
    --position;
    insertedPositions_[block->id] = position;
    insertedUpperBounds_.insert({block->upperBound, block->id});
}

void AdaptiveBlockList::deactivateEntry(const BlockPtr& block, std::size_t index) {
    if (!block || index >= block->activeFlags.size() || block->activeFlags[index] == 0) {
        return;
    }

    block->activeFlags[index] = 0;
    if (block->liveCount > 0) {
        --block->liveCount;
    }
    if (totalSize_ > 0) {
        --totalSize_;
    }

    const int key = block->items[index].key;
    auto keyIt = keyIndex_.find(key);
    if (keyIt != keyIndex_.end()
        && keyIt->second.block == block
        && keyIt->second.index == index) {
        keyIndex_.erase(keyIt);
    }

    eraseBlockIfEmpty(block);
}

void AdaptiveBlockList::deactivateKey(int key) {
    auto it = keyIndex_.find(key);
    if (it == keyIndex_.end()) {
        return;
    }
    deactivateEntry(it->second.block, it->second.index);
}

void AdaptiveBlockList::eraseBlockIfEmpty(const BlockPtr& block) {
    if (!block || block->liveCount != 0) {
        return;
    }

    if (block->prepended) {
        auto posIt = prependedPositions_.find(block->id);
        if (posIt != prependedPositions_.end()) {
            prependedBlocks_.erase(posIt->second);
            prependedPositions_.erase(posIt);
        }
        return;
    }

    auto posIt = insertedPositions_.find(block->id);
    if (posIt != insertedPositions_.end()) {
        insertedUpperBounds_.erase({block->upperBound, block->id});
        insertedBlocks_.erase(posIt->second);
        insertedPositions_.erase(posIt);
    }

    ensureInsertTail();
}

Distance AdaptiveBlockList::blockMinValue(const BlockPtr& block) const {
    if (!block || block->liveCount == 0) {
        return upperBound_;
    }

    Distance best = upperBound_;
    for (std::size_t i = 0; i < block->items.size(); ++i) {
        if (block->activeFlags[i] == 0) {
            continue;
        }
        best = std::min(best, block->items[i].value);
    }
    return best;
}

void AdaptiveBlockList::cleanupFrontEmptyBlocks() {
    while (!prependedBlocks_.empty() && prependedBlocks_.front()->liveCount == 0) {
        prependedPositions_.erase(prependedBlocks_.front()->id);
        prependedBlocks_.pop_front();
    }

    while (!insertedBlocks_.empty() && insertedBlocks_.front()->liveCount == 0) {
        insertedUpperBounds_.erase({insertedBlocks_.front()->upperBound, insertedBlocks_.front()->id});
        insertedPositions_.erase(insertedBlocks_.front()->id);
        insertedBlocks_.pop_front();
    }

    ensureInsertTail();
}

void AdaptiveBlockList::registerPrependedBlock(const BlockPtr& block, bool toFront) {
    if (!block) {
        return;
    }

    if (toFront) {
        prependedBlocks_.push_front(block);
        prependedPositions_[block->id] = prependedBlocks_.begin();
    } else {
        prependedBlocks_.push_back(block);
        auto pos = prependedBlocks_.end();
        --pos;
        prependedPositions_[block->id] = pos;
    }
}

void AdaptiveBlockList::registerInsertedBlock(const BlockPtr& block, BlockList::iterator position) {
    if (!block) {
        return;
    }

    insertedPositions_[block->id] = position;
    insertedUpperBounds_.insert({block->upperBound, block->id});
}

void AdaptiveBlockList::splitInsertedBlock(const BlockPtr& block) {
    if (!block || block->liveCount <= blockSize_) {
        return;
    }

    std::vector<KeyValue> liveItems;
    liveItems.reserve(block->liveCount);
    for (std::size_t i = 0; i < block->items.size(); ++i) {
        if (block->activeFlags[i] != 0) {
            liveItems.push_back(block->items[i]);
        }
    }

    if (liveItems.size() <= blockSize_) {
        block->items = std::move(liveItems);
        block->activeFlags.assign(block->items.size(), 1);
        block->liveCount = block->items.size();
        for (std::size_t i = 0; i < block->items.size(); ++i) {
            keyIndex_[block->items[i].key] = {block, i};
        }
        return;
    }

    std::sort(liveItems.begin(), liveItems.end(), lessKeyValue);
    const std::size_t middle = liveItems.size() / 2;

    auto left = std::make_shared<Block>();
    left->id = nextBlockId_++;
    left->prepended = false;
    left->items.assign(liveItems.begin(), liveItems.begin() + static_cast<std::ptrdiff_t>(middle));
    left->activeFlags.assign(left->items.size(), 1);
    left->liveCount = left->items.size();
    left->upperBound = left->items.back().value;

    auto right = std::make_shared<Block>();
    right->id = nextBlockId_++;
    right->prepended = false;
    right->items.assign(liveItems.begin() + static_cast<std::ptrdiff_t>(middle), liveItems.end());
    right->activeFlags.assign(right->items.size(), 1);
    right->liveCount = right->items.size();
    right->upperBound = block->upperBound;

    auto posIt = insertedPositions_.find(block->id);
    if (posIt == insertedPositions_.end()) {
        return;
    }

    auto oldPos = posIt->second;
    insertedUpperBounds_.erase({block->upperBound, block->id});
    auto rightPos = insertedBlocks_.erase(oldPos);
    insertedPositions_.erase(posIt);

    auto leftPos = insertedBlocks_.insert(rightPos, left);
    auto insertedRightPos = insertedBlocks_.insert(rightPos, right);
    registerInsertedBlock(left, leftPos);
    registerInsertedBlock(right, insertedRightPos);

    for (std::size_t i = 0; i < left->items.size(); ++i) {
        keyIndex_[left->items[i].key] = {left, i};
    }
    for (std::size_t i = 0; i < right->items.size(); ++i) {
        keyIndex_[right->items[i].key] = {right, i};
    }
}

void AdaptiveBlockList::insertIntoInsertedBlocks(int key, Distance value) {
    ensureInsertTail();

    auto upperIt = insertedUpperBounds_.lower_bound({value, 0});
    if (upperIt == insertedUpperBounds_.end()) {
        upperIt = std::prev(insertedUpperBounds_.end());
    }

    auto blockPosIt = insertedPositions_.find(upperIt->second);
    if (blockPosIt == insertedPositions_.end()) {
        return;
    }

    auto block = *blockPosIt->second;
    block->items.push_back({key, value});
    block->activeFlags.push_back(1);
    block->liveCount += 1;
    totalSize_ += 1;
    keyIndex_[key] = {block, block->items.size() - 1};

    splitInsertedBlock(block);
}

void AdaptiveBlockList::Insert(int key, Distance value) {
    auto existing = keyIndex_.find(key);
    if (existing != keyIndex_.end()) {
        const auto& oldItem = existing->second.block->items[existing->second.index];
        if (oldItem.value < value) {
            return;
        }
        deactivateKey(key);
    }

    insertIntoInsertedBlocks(key, value);
}

void AdaptiveBlockList::BatchPrepend(const std::vector<KeyValue>& values) {
    if (values.empty()) {
        return;
    }

    std::unordered_map<int, Distance> bestByKey;
    bestByKey.reserve(values.size() * 2);
    for (const auto& pair : values) {
        auto it = bestByKey.find(pair.key);
        if (it == bestByKey.end() || pair.value < it->second) {
            bestByKey[pair.key] = pair.value;
        }
    }

    std::vector<KeyValue> normalized;
    normalized.reserve(bestByKey.size());
    for (const auto& [key, value] : bestByKey) {
        auto existing = keyIndex_.find(key);
        if (existing != keyIndex_.end()) {
            const auto& oldItem = existing->second.block->items[existing->second.index];
            if (oldItem.value <= value) {
                continue;
            }
            deactivateKey(key);
        }
        normalized.push_back({key, value});
    }

    if (normalized.empty()) {
        return;
    }

    std::sort(normalized.begin(), normalized.end(), lessKeyValue);

    std::vector<BlockPtr> newBlocks;
    std::size_t index = 0;
    while (index < normalized.size()) {
        const std::size_t chunkSize = std::min(blockSize_, normalized.size() - index);
        auto block = std::make_shared<Block>();
        block->id = nextBlockId_++;
        block->prepended = true;
        block->items.assign(
            normalized.begin() + static_cast<std::ptrdiff_t>(index),
            normalized.begin() + static_cast<std::ptrdiff_t>(index + chunkSize)
        );
        block->activeFlags.assign(block->items.size(), 1);
        block->liveCount = block->items.size();
        block->upperBound = block->items.back().value;
        newBlocks.push_back(block);
        index += chunkSize;
    }

    for (auto it = newBlocks.rbegin(); it != newBlocks.rend(); ++it) {
        registerPrependedBlock(*it, true);
    }

    for (const auto& block : newBlocks) {
        for (std::size_t i = 0; i < block->items.size(); ++i) {
            keyIndex_[block->items[i].key] = {block, i};
            totalSize_ += 1;
        }
    }
}

std::vector<AdaptiveBlockList::KeyValue> AdaptiveBlockList::collectActiveEntries(
    const BlockList& blocks,
    std::size_t limit
) const {
    std::vector<KeyValue> collected;
    if (limit == 0) {
        return collected;
    }

    std::size_t liveSeen = 0;
    for (const auto& block : blocks) {
        if (!block || block->liveCount == 0) {
            continue;
        }

        for (std::size_t i = 0; i < block->items.size(); ++i) {
            if (block->activeFlags[i] == 0) {
                continue;
            }
            collected.push_back(block->items[i]);
            liveSeen += 1;
        }

        if (liveSeen >= limit) {
            break;
        }
    }

    return collected;
}

Distance AdaptiveBlockList::currentMinimumValue() const {
    Distance best = upperBound_;

    for (const auto& block : prependedBlocks_) {
        if (!block || block->liveCount == 0) {
            continue;
        }
        best = std::min(best, blockMinValue(block));
        break;
    }

    for (const auto& block : insertedBlocks_) {
        if (!block || block->liveCount == 0) {
            continue;
        }
        best = std::min(best, blockMinValue(block));
        break;
    }

    return best;
}

AdaptiveBlockList::PullResult AdaptiveBlockList::Pull(std::size_t count) {
    cleanupFrontEmptyBlocks();

    PullResult result;
    if (totalSize_ == 0 || count == 0) {
        result.separator = upperBound_;
        return result;
    }

    auto candidates = collectActiveEntries(prependedBlocks_, count);
    auto insertedCandidates = collectActiveEntries(insertedBlocks_, count);
    candidates.insert(candidates.end(), insertedCandidates.begin(), insertedCandidates.end());

    std::sort(candidates.begin(), candidates.end(), lessKeyValue);
    if (candidates.size() > count) {
        candidates.resize(count);
    }

    result.keys.reserve(candidates.size());
    for (const auto& pair : candidates) {
        result.keys.push_back(pair.key);
        deactivateKey(pair.key);
    }

    cleanupFrontEmptyBlocks();
    result.separator = (totalSize_ == 0) ? upperBound_ : currentMinimumValue();
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
//  AdvancedAlgorithms
// ─────────────────────────────────────────────────────────────────────────────
AdvancedAlgorithms::AdvancedAlgorithms(const Graph& g)
    : graph(g), source_(-1), k_(1), t_(1), maxLevel_(0), showSteps_(false) {}

AdvancedAlgorithms::Distance AdvancedAlgorithms::infinity() {
    return std::numeric_limits<Distance>::max() / 4;
}

std::size_t AdvancedAlgorithms::saturatingPow2(int exponent) {
    if (exponent <= 0) {
        return 1;
    }
    if (exponent >= static_cast<int>(sizeof(std::size_t) * 8 - 2)) {
        return std::numeric_limits<std::size_t>::max() / 4;
    }
    return static_cast<std::size_t>(1) << exponent;
}

std::size_t AdvancedAlgorithms::saturatingMultiply(std::size_t a, std::size_t b) {
    if (a == 0 || b == 0) {
        return 0;
    }
    if (a > std::numeric_limits<std::size_t>::max() / b) {
        return std::numeric_limits<std::size_t>::max() / 4;
    }
    return a * b;
}

int AdvancedAlgorithms::clampToInt(Distance value) {
    if (value >= static_cast<Distance>(std::numeric_limits<int>::max())) {
        return std::numeric_limits<int>::max();
    }
    if (value <= static_cast<Distance>(std::numeric_limits<int>::min())) {
        return std::numeric_limits<int>::min();
    }
    return static_cast<int>(value);
}

void AdvancedAlgorithms::configureParameters() {
    const int n = std::max(2, graph.getVertexCount());
    const double logN = std::max(1.0, std::log2(static_cast<double>(n)));
    k_ = std::max(1, static_cast<int>(std::floor(std::pow(logN, 1.0 / 3.0))));
    t_ = std::max(1, static_cast<int>(std::floor(std::pow(logN, 2.0 / 3.0))));
    maxLevel_ = std::max(0, static_cast<int>(std::ceil(logN / static_cast<double>(t_))));
}

void AdvancedAlgorithms::resetState(int start) {
    source_ = start;
    showSteps_ = false;

    const int n = graph.getVertexCount();
    distances_.assign(n, infinity());
    predecessor_.assign(n, -1);
    hopCount_.assign(n, std::numeric_limits<int>::max());
    complete_.assign(n, 0);
    logs_.clear();
    traceSteps_.clear();

    if (start >= 0 && start < n) {
        distances_[start] = 0;
        hopCount_[start] = 0;
        complete_[start] = 1;
    }
}

bool AdvancedAlgorithms::preferCandidate(int u, int v, Distance candidateDistance) const {
    if (candidateDistance < distances_[v]) {
        return true;
    }
    if (candidateDistance > distances_[v]) {
        return false;
    }

    const int candidateHops = (hopCount_[u] == std::numeric_limits<int>::max())
        ? std::numeric_limits<int>::max()
        : hopCount_[u] + 1;

    if (candidateHops < hopCount_[v]) {
        return true;
    }
    if (candidateHops > hopCount_[v]) {
        return false;
    }

    if (predecessor_[v] == -1) {
        return true;
    }
    return u < predecessor_[v];
}

bool AdvancedAlgorithms::applyRelax(int u, int v, int weight) {
    if (u < 0 || u >= static_cast<int>(distances_.size()) || distances_[u] >= infinity() / 2) {
        return false;
    }

    const Distance candidate = distances_[u] + static_cast<Distance>(weight);
    if (!preferCandidate(u, v, candidate)) {
        return false;
    }

    distances_[v] = candidate;
    predecessor_[v] = u;
    hopCount_[v] = (hopCount_[u] == std::numeric_limits<int>::max())
        ? std::numeric_limits<int>::max()
        : hopCount_[u] + 1;

    TraceStep step;
    step.type = "relax";
    step.from = u;
    step.to = v;
    step.dist = clampToInt(distances_[u]);
    step.new_dist = clampToInt(candidate);
    step.desc = "Advanced relax " + graph.getVertexLabel(u) + " -> " + graph.getVertexLabel(v);
    traceSteps_.push_back(std::move(step));

    return true;
}

void AdvancedAlgorithms::markComplete(const std::vector<int>& vertices) {
    for (int vertex : vertices) {
        if (vertex >= 0 && vertex < static_cast<int>(complete_.size())) {
            complete_[vertex] = 1;
        }
    }
}

void AdvancedAlgorithms::addLog(int color, const std::string& message) {
    if (!showSteps_) {
        return;
    }
    logs_.push_back({color, message});
}

std::vector<int> AdvancedAlgorithms::uniqueVertices(const std::vector<int>& vertices) const {
    std::vector<int> unique;
    std::vector<unsigned char> seen(graph.getVertexCount(), 0);
    for (int vertex : vertices) {
        if (vertex < 0 || vertex >= graph.getVertexCount()) {
            continue;
        }
        if (seen[vertex] != 0) {
            continue;
        }
        seen[vertex] = 1;
        unique.push_back(vertex);
    }
    return unique;
}

AdvancedAlgorithms::Distance AdvancedAlgorithms::minDistanceOf(
    const std::vector<int>& vertices,
    Distance fallback
) const {
    Distance best = fallback;
    for (int vertex : vertices) {
        if (vertex < 0 || vertex >= static_cast<int>(distances_.size())) {
            continue;
        }
        best = std::min(best, distances_[vertex]);
    }
    return best;
}

AdvancedAlgorithms::PivotSearchResult AdvancedAlgorithms::findPivots(
    Distance bound,
    const std::vector<int>& sources
) {
    PivotSearchResult result;
    const auto uniqueSources = uniqueVertices(sources);
    if (uniqueSources.empty()) {
        return result;
    }

    const int n = graph.getVertexCount();
    const auto& adj = graph.getAdjacencyList();

    std::vector<unsigned char> inW(n, 0);
    std::vector<unsigned char> inNext(n, 0);
    std::vector<int> W = uniqueSources;
    for (int vertex : uniqueSources) {
        inW[vertex] = 1;
    }

    std::vector<int> frontier = uniqueSources;
    for (int stepIndex = 1; stepIndex <= k_; ++stepIndex) {
        std::fill(inNext.begin(), inNext.end(), 0);
        std::vector<int> next;

        for (int u : frontier) {
            if (u < 0 || u >= n || distances_[u] >= infinity() / 2) {
                continue;
            }

            TraceStep visit;
            visit.type = "visit";
            visit.node = u;
            visit.dist = clampToInt(distances_[u]);
            visit.desc = "FindPivots step on " + graph.getVertexLabel(u);
            traceSteps_.push_back(std::move(visit));

            for (const auto& edge : adj[u]) {
                const Distance candidate = distances_[u] + static_cast<Distance>(edge.weight);
                if (candidate > distances_[edge.destination]) {
                    continue;
                }

                applyRelax(u, edge.destination, edge.weight);
                if (candidate < bound && inNext[edge.destination] == 0) {
                    inNext[edge.destination] = 1;
                    next.push_back(edge.destination);
                    if (inW[edge.destination] == 0) {
                        inW[edge.destination] = 1;
                        W.push_back(edge.destination);
                    }
                }
            }
        }

        frontier.swap(next);
        if (W.size() > saturatingMultiply(static_cast<std::size_t>(k_), uniqueSources.size())) {
            result.pivots = uniqueSources;
            result.witnessed = W;
            addLog(11, "FindPivots dừng sớm vì |W| vượt ngưỡng k|S|.");
            return result;
        }
    }

    std::vector<std::vector<int>> children(n);
    std::vector<int> roots;
    for (int vertex : W) {
        const int parent = predecessor_[vertex];
        if (parent >= 0 && parent < n && inW[parent] != 0 && parent != vertex) {
            children[parent].push_back(vertex);
        } else {
            roots.push_back(vertex);
        }
    }

    std::vector<int> subtreeSize(n, 0);
    std::function<int(int)> dfs = [&](int u) -> int {
        int total = 1;
        for (int child : children[u]) {
            total += dfs(child);
        }
        subtreeSize[u] = total;
        return total;
    };

    for (int root : roots) {
        dfs(root);
    }

    std::vector<unsigned char> isSource(n, 0);
    for (int vertex : uniqueSources) {
        isSource[vertex] = 1;
    }

    for (int root : roots) {
        if (isSource[root] != 0 && subtreeSize[root] >= k_) {
            result.pivots.push_back(root);
        }
    }

    result.witnessed = W;
    addLog(10, "FindPivots: |S| = " + std::to_string(uniqueSources.size())
        + ", |W| = " + std::to_string(result.witnessed.size())
        + ", |P| = " + std::to_string(result.pivots.size()));
    return result;
}

AdvancedAlgorithms::BMSSPResult AdvancedAlgorithms::baseCase(
    Distance bound,
    const std::vector<int>& sources
) {
    BMSSPResult result;
    result.boundary = bound;

    const auto baseSources = uniqueVertices(sources);
    if (baseSources.empty()) {
        return result;
    }

    const int x = baseSources.front();
    const auto& adj = graph.getAdjacencyList();
    std::vector<Distance> localBest(graph.getVertexCount(), infinity());
    std::vector<unsigned char> extracted(graph.getVertexCount(), 0);
    std::vector<unsigned char> inU0(graph.getVertexCount(), 0);

    using HeapNode = std::pair<Distance, int>;
    std::priority_queue<HeapNode, std::vector<HeapNode>, std::greater<HeapNode>> heap;
    localBest[x] = distances_[x];
    heap.push({localBest[x], x});

    std::vector<int> U0;
    while (!heap.empty() && U0.size() < static_cast<std::size_t>(k_ + 1)) {
        const auto [bestDistance, u] = heap.top();
        heap.pop();

        if (bestDistance != localBest[u] || extracted[u] != 0) {
            continue;
        }
        extracted[u] = 1;

        if (inU0[u] == 0) {
            inU0[u] = 1;
            U0.push_back(u);
        }

        TraceStep visit;
        visit.type = "visit";
        visit.node = u;
        visit.dist = clampToInt(bestDistance);
        visit.desc = "BMSSP base-case visit " + graph.getVertexLabel(u);
        traceSteps_.push_back(std::move(visit));

        for (const auto& edge : adj[u]) {
            if (distances_[u] >= infinity() / 2) {
                continue;
            }

            const Distance candidate = distances_[u] + static_cast<Distance>(edge.weight);
            if (candidate > distances_[edge.destination] || candidate >= bound) {
                continue;
            }

            applyRelax(u, edge.destination, edge.weight);
            if (candidate < localBest[edge.destination]) {
                localBest[edge.destination] = candidate;
                heap.push({candidate, edge.destination});
            }
        }
    }

    if (U0.size() <= static_cast<std::size_t>(k_)) {
        result.boundary = bound;
        result.vertices = U0;
    } else {
        Distance newBoundary = -1;
        for (int vertex : U0) {
            newBoundary = std::max(newBoundary, distances_[vertex]);
        }
        result.boundary = newBoundary;
        for (int vertex : U0) {
            if (distances_[vertex] < newBoundary) {
                result.vertices.push_back(vertex);
            }
        }
    }

    markComplete(result.vertices);
    addLog(13, "BaseCase trả về " + std::to_string(result.vertices.size())
        + " đỉnh với biên B' = " + std::to_string(clampToInt(result.boundary)) + ".");
    return result;
}

AdvancedAlgorithms::BMSSPResult AdvancedAlgorithms::bmssp(
    int level,
    Distance bound,
    const std::vector<int>& sources
) {
    const auto activeSources = uniqueVertices(sources);
    if (activeSources.empty()) {
        return {bound, {}};
    }

    if (level <= 0) {
        return baseCase(bound, activeSources);
    }

    const auto pivotResult = findPivots(bound, activeSources);
    const std::size_t maxFanout = std::max<std::size_t>(1, saturatingPow2((level - 1) * t_));
    const std::size_t expectedInsertions = std::max<std::size_t>(
        maxFanout,
        saturatingMultiply(static_cast<std::size_t>(k_), saturatingPow2(level * t_))
    );

    AdaptiveBlockList frontier(expectedInsertions, maxFanout, bound);
    for (int pivot : pivotResult.pivots) {
        frontier.Insert(pivot, distances_[pivot]);
    }

    Distance currentBoundary = minDistanceOf(pivotResult.pivots, bound);
    std::vector<int> U;
    std::vector<unsigned char> inU(graph.getVertexCount(), 0);
    auto appendUnique = [&](const std::vector<int>& vertices) {
        for (int vertex : vertices) {
            if (vertex < 0 || vertex >= graph.getVertexCount()) {
                continue;
            }
            if (inU[vertex] != 0) {
                continue;
            }
            inU[vertex] = 1;
            U.push_back(vertex);
        }
    };

    const std::size_t limit = saturatingMultiply(
        static_cast<std::size_t>(std::max(1, k_)),
        saturatingPow2(level * t_)
    );

    int phase = 0;
    while (U.size() < limit && !frontier.empty()) {
        ++phase;
        const auto pulled = frontier.Pull(maxFanout);
        const Distance phaseBound = pulled.separator;
        const auto subResult = bmssp(level - 1, phaseBound, pulled.keys);
        currentBoundary = subResult.boundary;
        appendUnique(subResult.vertices);

        std::vector<AdaptiveBlockList::KeyValue> prependList;
        const auto& adj = graph.getAdjacencyList();
        for (int u : subResult.vertices) {
            if (u < 0 || u >= graph.getVertexCount() || distances_[u] >= infinity() / 2) {
                continue;
            }

            for (const auto& edge : adj[u]) {
                const Distance candidate = distances_[u] + static_cast<Distance>(edge.weight);
                const Distance previous = distances_[edge.destination];
                if (candidate > previous) {
                    continue;
                }

                applyRelax(u, edge.destination, edge.weight);
                if (candidate >= phaseBound && candidate < bound) {
                    frontier.Insert(edge.destination, candidate);
                } else if (candidate >= subResult.boundary && candidate < phaseBound) {
                    prependList.push_back({edge.destination, candidate});
                }
            }
        }

        for (int vertex : pulled.keys) {
            if (vertex >= 0
                && vertex < graph.getVertexCount()
                && distances_[vertex] >= subResult.boundary
                && distances_[vertex] < phaseBound) {
                prependList.push_back({vertex, distances_[vertex]});
            }
        }

        frontier.BatchPrepend(prependList);
        addLog(10, "BMSSP(l=" + std::to_string(level) + ", phase=" + std::to_string(phase)
            + "): |Si|=" + std::to_string(pulled.keys.size())
            + ", |Ui|=" + std::to_string(subResult.vertices.size())
            + ", |U|=" + std::to_string(U.size()) + ".");
    }

    const Distance finalBoundary = std::min(currentBoundary, bound);
    std::vector<int> witnessedComplete;
    for (int vertex : pivotResult.witnessed) {
        if (vertex >= 0 && vertex < graph.getVertexCount() && distances_[vertex] < finalBoundary) {
            witnessedComplete.push_back(vertex);
        }
    }

    appendUnique(witnessedComplete);
    markComplete(U);

    addLog(11, "BMSSP level " + std::to_string(level)
        + " kết thúc với |U| = " + std::to_string(U.size())
        + ", B' = " + std::to_string(clampToInt(finalBoundary)) + ".");
    return {finalBoundary, U};
}

std::vector<int> AdvancedAlgorithms::reconstructPath(int destination) const {
    if (destination < 0 || destination >= static_cast<int>(predecessor_.size())) {
        return {};
    }
    if (distances_[destination] >= infinity() / 2) {
        return {};
    }

    std::vector<int> path;
    std::vector<unsigned char> seen(predecessor_.size(), 0);
    int current = destination;
    while (current != -1 && seen[current] == 0) {
        seen[current] = 1;
        path.push_back(current);
        current = predecessor_[current];
    }

    if (!path.empty() && path.back() != source_) {
        return {};
    }

    std::reverse(path.begin(), path.end());
    return path;
}

PathResult AdvancedAlgorithms::buildResult(int start) const {
    PathResult result;
    result.success = true;
    result.startVertex = start;
    result.hasNegativeCycle = false;
    result.distances.resize(distances_.size(), std::numeric_limits<int>::max());
    result.previousVertex = predecessor_;
    result.logs = logs_;
    result.traceSteps = traceSteps_;

    for (std::size_t i = 0; i < distances_.size(); ++i) {
        if (distances_[i] < infinity() / 2) {
            result.distances[i] = clampToInt(distances_[i]);
        }
    }

    return result;
}

PathResult AdvancedAlgorithms::breakingSortingBarrier(int start, bool showSteps) {
    PathResult failure;
    resetState(start);
    showSteps_ = showSteps;
    configureParameters();

    if (!graph.isValid() || start < 0 || start >= graph.getVertexCount()) {
        addLog(12, "Đỉnh bắt đầu không hợp lệ cho Breaking Sorting Barrier.");
        failure.logs = logs_;
        failure.traceSteps = traceSteps_;
        return failure;
    }

    if (graph.hasNegativeWeights()) {
        addLog(12, "Thuật toán mới chỉ áp dụng cho trọng số không âm, đúng theo bài báo July 30, 2025.");
        failure.logs = logs_;
        failure.traceSteps = traceSteps_;
        failure.startVertex = start;
        return failure;
    }

    addLog(14, "======= BREAKING THE SORTING BARRIER (Practical Translation) =======");
    addLog(11, "Tham số: k = floor(log^(1/3) n) = " + std::to_string(k_)
        + ", t = floor(log^(2/3) n) = " + std::to_string(t_)
        + ", maxLevel = " + std::to_string(maxLevel_) + ".");
    addLog(7, "Lưu ý: bản cài đặt này giữ tư tưởng BMSSP + FindPivots + AdaptiveBlockList của bài báo,");
    addLog(7, "nhưng chạy trực tiếp trên đồ thị hiện có thay vì thêm bước degree-reduction lý thuyết.");

    bmssp(maxLevel_, infinity(), {start});

    auto pathResult = buildResult(start);
    addLog(10, "Hoàn tất wrapper breakingSortingBarrier().");
    pathResult.logs = logs_;
    pathResult.traceSteps = traceSteps_;
    return pathResult;
}

std::vector<int> AdvancedAlgorithms::getShortestPath(const PathResult& result, int destination) const {
    if (destination < 0 || destination >= static_cast<int>(result.previousVertex.size())) {
        return {};
    }
    if (destination >= static_cast<int>(result.distances.size())
        || result.distances[destination] == std::numeric_limits<int>::max()) {
        return {};
    }

    std::vector<int> path;
    std::vector<unsigned char> seen(result.previousVertex.size(), 0);
    int current = destination;
    while (current != -1 && seen[current] == 0) {
        seen[current] = 1;
        path.push_back(current);
        current = result.previousVertex[current];
    }

    std::reverse(path.begin(), path.end());
    return path;
}

int AdvancedAlgorithms::getDistance(const PathResult& result, int destination) const {
    if (destination < 0 || destination >= static_cast<int>(result.distances.size())) {
        return -1;
    }
    return result.distances[destination];
}
