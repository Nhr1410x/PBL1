#ifndef GRAPHCANVAS_H
#define GRAPHCANVAS_H

#include <gtkmm/drawingarea.h>
#include <cairomm/context.h>
#include <glibmm/main.h>
#include <sigc++/sigc++.h>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include "../lib/graph.h"
#include "../lib/algorithms.h"

struct Point {
    double x = 0.0;
    double y = 0.0;
};

class GraphCanvas : public Gtk::DrawingArea {
public:
    using type_signal_vertex_selected = sigc::signal<void, int>;

    GraphCanvas();
    virtual ~GraphCanvas();

    void loadData(const Graph& g, const PathResult& res, bool isDirected);
    void playAnimation();
    void pauseAnimation();
    void resetAnimation();
    void resetView();
    void setAnimationSpeed(int ms);
    type_signal_vertex_selected& signal_start_vertex_selected();
    type_signal_vertex_selected& signal_end_vertex_selected();

protected:
    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
    bool on_button_press_event(GdkEventButton* event) override;
    bool on_button_release_event(GdkEventButton* event) override;
    bool on_motion_notify_event(GdkEventMotion* event) override;
    bool on_scroll_event(GdkEventScroll* event) override;
    bool on_leave_notify_event(GdkEventCrossing* event) override;

private:
    bool on_timeout();
    void calculateLayout();
    void buildNeighborGraph(std::vector<std::vector<int>>& neighbors) const;
    void assignPrimaryLayers(
        const std::vector<std::vector<int>>& neighbors,
        std::vector<int>& layers,
        int& maxLayer
    ) const;
    void orderNodesWithinLayers(
        const std::vector<std::vector<int>>& neighbors,
        const std::vector<int>& layers,
        std::vector<std::vector<int>>& layerNodes
    ) const;
    void initializeLayerPositions(
        const std::vector<std::vector<int>>& layerNodes,
        std::vector<Point>& positions
    ) const;
    void refinePositionsHybrid(
        const std::vector<std::vector<int>>& neighbors,
        const std::vector<int>& layers,
        std::vector<Point>& positions
    ) const;
    void resolveNodeCollisions(std::vector<Point>& positions, double minDistance);
    void normalizeAndStorePositions(
        std::vector<Point>& positions,
        const std::vector<int>& layers
    );
    double estimateMinimumNodeSpacing() const;
    double computeEdgeCurvature(
        Point from,
        Point to,
        int parallelIndex,
        int parallelCount,
        bool reverseDirection,
        bool bidirectional
    ) const;
    Point computeCurvedLabelPosition(
        Point from,
        Point to,
        double curveAmount,
        int parallelIndex,
        int parallelCount,
        bool selfLoop = false
    ) const;
    void cacheSceneBounds();
    void rebuildDistancesForCurrentStep();
    void updateHover(double x, double y);
    void drawArrow(
        const Cairo::RefPtr<Cairo::Context>& cr,
        Point from,
        Point to,
        double nodeRadius,
        bool curved,
        double curveAmount
    );
    void drawWeightLabel(
        const Cairo::RefPtr<Cairo::Context>& cr,
        Point position,
        const std::string& text
    );
    void drawTooltip(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height);
    void drawLegend(const Cairo::RefPtr<Cairo::Context>& cr);
    void drawDistanceTable(const Cairo::RefPtr<Cairo::Context>& cr, int width);
    void drawProgressBar(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height);
    Point worldToScreen(const Point& world, int width, int height) const;
    Point screenToWorld(double x, double y, int width, int height) const;
    double computeFitScale(int width, int height) const;
    double distance(Point p1, Point p2) const;
    int hitTestNode(double screenX, double screenY) const;
    std::string formatDistance(int value) const;
    double currentNodeRadius() const;

    Graph m_graph;
    PathResult m_result;
    bool m_isDirected;
    bool m_hasData;

    std::vector<Point> m_pos;
    
    // Animation state
    int m_currentStep;
    int m_speedMs;
    sigc::connection m_timeoutConnection;
    bool m_isPlaying;

    Point m_sceneCenter;
    double m_sceneWidth;
    double m_sceneHeight;
    double m_zoom;
    Point m_pan;
    int m_hoveredNode;
    Point m_hoverScreen;
    int m_draggingNode;
    int m_clickCandidateNode;
    bool m_isPanning;
    bool m_pointerMovedSincePress;
    Point m_pressScreen;
    Point m_lastPointerScreen;

    std::unordered_map<int, int> m_distances;
    std::unordered_set<int> m_visited;
    int m_visitingNode;
    std::pair<int, int> m_relaxingEdge;
    std::unordered_set<long long> m_pathEdges;
    type_signal_vertex_selected m_signalStartVertexSelected;
    type_signal_vertex_selected m_signalEndVertexSelected;
};

#endif // GRAPHCANVAS_H
