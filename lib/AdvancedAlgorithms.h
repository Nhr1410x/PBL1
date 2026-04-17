#ifndef ADVANCEDALGORITHMS_H
#define ADVANCEDALGORITHMS_H

#include <cstddef>
#include <list>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "algorithms.h"
#include "graph.h"

class AdaptiveBlockList {
public:
    using Distance = long long;

    struct KeyValue {
        int key = -1;
        Distance value = 0;
    };

    struct PullResult {
        std::vector<int> keys;
        Distance separator = 0;
    };

    AdaptiveBlockList();
    AdaptiveBlockList(std::size_t expectedInsertions, std::size_t blockSize, Distance upperBound);

    void reset(std::size_t expectedInsertions, std::size_t blockSize, Distance upperBound);
    void Insert(int key, Distance value);
    void BatchPrepend(const std::vector<KeyValue>& values);
    PullResult Pull(std::size_t count);

    bool empty() const;
    std::size_t size() const;
    std::size_t blockSize() const;

private:
    struct Block;

    struct EntryRef {
        std::shared_ptr<Block> block;
        std::size_t index = 0;
    };

    struct Block {
        std::size_t id = 0;
        bool prepended = false;
        Distance upperBound = 0;
        std::vector<KeyValue> items;
        std::vector<unsigned char> activeFlags;
        std::size_t liveCount = 0;
    };

    using BlockPtr = std::shared_ptr<Block>;
    using BlockList = std::list<BlockPtr>;

    std::size_t expectedInsertions_;
    std::size_t blockSize_;
    Distance upperBound_;
    std::size_t totalSize_;
    std::size_t nextBlockId_;

    BlockList prependedBlocks_;
    BlockList insertedBlocks_;
    std::unordered_map<std::size_t, BlockList::iterator> prependedPositions_;
    std::unordered_map<std::size_t, BlockList::iterator> insertedPositions_;
    std::set<std::pair<Distance, std::size_t>> insertedUpperBounds_;
    std::unordered_map<int, EntryRef> keyIndex_;

    void ensureInsertTail();
    void insertIntoInsertedBlocks(int key, Distance value);
    void splitInsertedBlock(const BlockPtr& block);
    void deactivateKey(int key);
    void deactivateEntry(const BlockPtr& block, std::size_t index);
    void eraseBlockIfEmpty(const BlockPtr& block);
    void cleanupFrontEmptyBlocks();
    Distance blockMinValue(const BlockPtr& block) const;
    std::vector<KeyValue> collectActiveEntries(const BlockList& blocks, std::size_t limit) const;
    Distance currentMinimumValue() const;
    void registerPrependedBlock(const BlockPtr& block, bool toFront);
    void registerInsertedBlock(const BlockPtr& block, BlockList::iterator position);
};

class AdvancedAlgorithms {
public:
    using Distance = long long;

    explicit AdvancedAlgorithms(const Graph& g);

    PathResult breakingSortingBarrier(int start, bool showSteps = false);

    std::vector<int> getShortestPath(const PathResult& result, int destination) const;
    int getDistance(const PathResult& result, int destination) const;

private:
    struct PivotSearchResult {
        std::vector<int> pivots;
        std::vector<int> witnessed;
    };

    struct BMSSPResult {
        Distance boundary = 0;
        std::vector<int> vertices;
    };

    const Graph& graph;

    int source_;
    int k_;
    int t_;
    int maxLevel_;
    bool showSteps_;

    std::vector<Distance> distances_;
    std::vector<int> predecessor_;
    std::vector<int> hopCount_;
    std::vector<unsigned char> complete_;
    std::vector<std::pair<int, std::string>> logs_;
    std::vector<TraceStep> traceSteps_;

    void configureParameters();
    void resetState(int start);
    bool preferCandidate(int u, int v, Distance candidateDistance) const;
    bool applyRelax(int u, int v, int weight);
    void markComplete(const std::vector<int>& vertices);
    void addLog(int color, const std::string& message);
    std::vector<int> uniqueVertices(const std::vector<int>& vertices) const;
    Distance minDistanceOf(const std::vector<int>& vertices, Distance fallback) const;
    PivotSearchResult findPivots(Distance bound, const std::vector<int>& sources);
    BMSSPResult baseCase(Distance bound, const std::vector<int>& sources);
    BMSSPResult bmssp(int level, Distance bound, const std::vector<int>& sources);
    std::vector<int> reconstructPath(int destination) const;
    PathResult buildResult(int start) const;

    static Distance infinity();
    static std::size_t saturatingPow2(int exponent);
    static std::size_t saturatingMultiply(std::size_t a, std::size_t b);
    static int clampToInt(Distance value);
};

#endif // ADVANCEDALGORITHMS_H
