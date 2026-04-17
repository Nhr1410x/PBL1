#pragma once
#include <string>

constexpr int WINDOW_WIDTH  = 1200;
constexpr int WINDOW_HEIGHT = 800;

enum class AppState {
    MENU,
    INPUT,
    RUNNING,
    COMPARE,
    VISUALIZE
};

enum class AlgorithmType {
    DIJKSTRA,
    BELLMAN_FORD,
    ADVANCED_BMSSP,
    BOTH
};

const std::string DATA_FOLDER = "../data";
const std::string TEMP_EXPORT_FILE = "../data/temp.txt";
const std::string TRACE_EXPORT_FILE = "../data/trace.json";
const std::string DEFAULT_GRAPH_FILE = "../data/graph.txt";
const std::string DEFAULT_REPORT_FILE = "../data/report.txt";
const std::string DEFAULT_FONT_PATH = "assets/arial.ttf";
